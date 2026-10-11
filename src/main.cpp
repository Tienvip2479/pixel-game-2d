#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

namespace {
constexpr unsigned int kWindowWidth = 960;
constexpr unsigned int kWindowHeight = 540;
constexpr float kPlayerSpeed = 220.0f;
constexpr float kBulletSpeed = 540.0f;
constexpr float kEnemySpeed = 90.0f;
constexpr float kEnemySpawnDelay = 2.0f;
constexpr float kPickupSpawnDelay = 6.0f;

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    float length() const { return std::sqrt(x * x + y * y); }

    Vec2 normalized() const {
        float len = length();
        if (len == 0.0f) return {0.0f, 0.0f};
        return {x / len, y / len};
    }

    Vec2 operator+(const Vec2& rhs) const { return {x + rhs.x, y + rhs.y}; }
    Vec2 operator-(const Vec2& rhs) const { return {x - rhs.x, y - rhs.y}; }
    Vec2 operator*(float v) const { return {x * v, y * v}; }
    Vec2 operator/(float v) const { return {x / v, y / v}; }
    Vec2& operator+=(const Vec2& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    Vec2& operator-=(const Vec2& rhs) {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
};

sf::Vector2f toVector2f(const Vec2& v) { return {v.x, v.y}; }

float clamp(float value, float minv, float maxv) {
    return std::max(minv, std::min(maxv, value));
}

int randomInt(int minv, int maxv) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(minv, maxv);
    return dist(rng);
}

class Bullet {
public:
    bool active = false;
    Vec2 pos;
    Vec2 dir;
    float radius = 4.0f;
    sf::CircleShape shape;

    Bullet() {
        shape.setRadius(radius);
        shape.setFillColor(sf::Color(255, 208, 94));
        shape.setOutlineColor(sf::Color(110, 79, 0));
        shape.setOutlineThickness(1.0f);
    }

    void fire(const Vec2& start, const Vec2& direction) {
        active = true;
        pos = start;
        dir = direction.normalized();
        shape.setPosition(start.x - radius, start.y - radius);
    }

    void update(float dt) {
        if (!active) return;
        pos += dir * kBulletSpeed * dt;
        shape.setPosition(pos.x - radius, pos.y - radius);
        if (pos.x < -50 || pos.x > kWindowWidth + 50 || pos.y < -50 || pos.y > kWindowHeight + 50) {
            active = false;
        }
    }

    void draw(sf::RenderWindow& window) {
        if (active) {
            window.draw(shape);
        }
    }
};

class Enemy {
public:
    bool active = false;
    Vec2 pos;
    Vec2 vel;
    float radius = 16.0f;
    sf::CircleShape shape;
    int hp = 1;
    int scoreValue = 10;

    Enemy() {
        shape.setRadius(radius);
        shape.setFillColor(sf::Color(229, 80, 80));
        shape.setOutlineColor(sf::Color(90, 20, 20));
        shape.setOutlineThickness(2.0f);
    }

    void spawn(float x, float y) {
        active = true;
        pos = {x, y};
        vel = {0.0f, 0.0f};
        hp = 1;
        shape.setPosition(x - radius, y - radius);
    }

    void update(float dt, const Vec2& playerPos) {
        if (!active) return;
        Vec2 dir = playerPos - pos;
        float len = dir.length();
        if (len > 0.001f) {
            dir = dir / len;
            vel = dir * kEnemySpeed;
        }
        pos += vel * dt;
        shape.setPosition(pos.x - radius, pos.y - radius);
    }

    void draw(sf::RenderWindow& window) {
        if (active) {
            window.draw(shape);
        }
    }

    sf::FloatRect getBounds() const {
        return sf::FloatRect(pos.x - radius, pos.y - radius, radius * 2.0f, radius * 2.0f);
    }
};

class Pickup {
public:
    enum class Type { Heal, Score };

    bool active = false;
    Vec2 pos;
    float radius = 9.0f;
    Type type = Type::Score;
    sf::CircleShape shape;

    Pickup() {
        shape.setRadius(radius);
        shape.setOutlineThickness(2.0f);
    }

    void spawn(float x, float y, Type t) {
        active = true;
        pos = {x, y};
        type = t;
        shape.setPosition(pos.x - radius, pos.y - radius);
        shape.setFillColor(type == Type::Heal ? sf::Color(76, 220, 149) : sf::Color(255, 214, 94));
        shape.setOutlineColor(type == Type::Heal ? sf::Color(18, 85, 52) : sf::Color(130, 97, 18));
    }

    void update(float dt) {
        if (!active) return;
        (void)dt;
        shape.setPosition(pos.x - radius, pos.y - radius);
    }

    void draw(sf::RenderWindow& window) {
        if (active) {
            window.draw(shape);
        }
    }

    sf::FloatRect getBounds() const {
        return sf::FloatRect(pos.x - radius, pos.y - radius, radius * 2.0f, radius * 2.0f);
    }
};

class Player {
public:
    Vec2 pos;
    float radius = 18.0f;
    sf::CircleShape shape;
    int hp = 5;
    int score = 0;
    int maxHp = 5;
    float shootCooldown = 0.0f;

    Player() {
        pos = {kWindowWidth * 0.5f, kWindowHeight * 0.5f};
        shape.setRadius(radius);
        shape.setFillColor(sf::Color(94, 208, 255));
        shape.setOutlineColor(sf::Color(15, 58, 88));
        shape.setOutlineThickness(2.0f);
        shape.setPosition(pos.x - radius, pos.y - radius);
    }

    void setPosition(float x, float y) {
        pos = {x, y};
        shape.setPosition(pos.x - radius, pos.y - radius);
    }

    void update(float dt, const sf::Vector2f& mousePos, bool moveLeft, bool moveRight, bool moveUp, bool moveDown, bool shoot) {
        Vec2 dir = {0.0f, 0.0f};
        if (moveLeft) dir.x -= 1.0f;
        if (moveRight) dir.x += 1.0f;
        if (moveUp) dir.y -= 1.0f;
        if (moveDown) dir.y += 1.0f;

        if (dir.length() > 0.0f) {
            dir = dir.normalized();
            pos += dir * kPlayerSpeed * dt;
        }

        pos.x = clamp(pos.x, radius, static_cast<float>(kWindowWidth) - radius);
        pos.y = clamp(pos.y, radius, static_cast<float>(kWindowHeight) - radius);
        shape.setPosition(pos.x - radius, pos.y - radius);

        shootCooldown = std::max(0.0f, shootCooldown - dt);
        if (shoot && shootCooldown <= 0.0f) {
            shootCooldown = 0.18f;
        }
    }

    void draw(sf::RenderWindow& window) { window.draw(shape); }

    sf::FloatRect getBounds() const {
        return sf::FloatRect(pos.x - radius, pos.y - radius, radius * 2.0f, radius * 2.0f);
    }
};

class Game {
public:
    sf::RenderWindow window;
    sf::Font font;
    sf::Text titleText;
    sf::Text scoreText;
    sf::Text hpText;
    sf::Text gameOverText;
    sf::Text waveText;

    Player player;
    std::vector<Bullet> bullets;
    std::vector<Enemy> enemies;
    std::vector<Pickup> pickups;

    float spawnTimer = 0.0f;
    float pickupTimer = 0.0f;
    int wave = 1;
    bool gameOver = false;
    bool restartReady = false;
    bool leftMousePressed = false;

    Game() : window(sf::VideoMode(kWindowWidth, kWindowHeight), "Pixel Arena", sf::Style::Close) {
        window.setVerticalSyncEnabled(true);
        window.setFramerateLimit(60);
        bullets.resize(80);
        enemies.resize(80);
        pickups.resize(12);

        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cerr << "Warning: Default font not found, GUI may be limited.\n";
        }

        initText();
        resetGame();
    }

    void initText() {
        titleText.setFont(font);
        titleText.setString("PIXEL ARENA");
        titleText.setCharacterSize(34);
        titleText.setFillColor(sf::Color(255, 220, 110));
        titleText.setPosition(22.0f, 18.0f);

        scoreText.setFont(font);
        scoreText.setCharacterSize(22);
        scoreText.setFillColor(sf::Color(245, 245, 245));
        scoreText.setPosition(22.0f, 72.0f);

        hpText.setFont(font);
        hpText.setCharacterSize(22);
        hpText.setFillColor(sf::Color(255, 120, 120));
        hpText.setPosition(22.0f, 104.0f);

        waveText.setFont(font);
        waveText.setCharacterSize(22);
        waveText.setFillColor(sf::Color(136, 207, 255));
        waveText.setPosition(22.0f, 136.0f);

        gameOverText.setFont(font);
        gameOverText.setCharacterSize(38);
        gameOverText.setFillColor(sf::Color(255, 90, 90));
        gameOverText.setString("GAME OVER\nPress R to restart");
        gameOverText.setPosition(kWindowWidth * 0.5f - 140.0f, kWindowHeight * 0.5f - 60.0f);
    }

    void resetGame() {
        player = Player();
        gameOver = false;
        restartReady = false;
        bullets.clear();
        bullets.resize(80);
        enemies.clear();
        enemies.resize(80);
        pickups.clear();
        pickups.resize(12);
        spawnTimer = 0.0f;
        pickupTimer = 0.0f;
        wave = 1;
        for (auto& bullet : bullets) {
            bullet.active = false;
        }
        for (auto& enemy : enemies) {
            enemy.active = false;
        }
        for (auto& pickup : pickups) {
            pickup.active = false;
        }
    }

    void spawnEnemy() {
        for (auto& enemy : enemies) {
            if (!enemy.active) {
                int side = randomInt(0, 3);
                float x = 0.0f, y = 0.0f;
                if (side == 0) { x = randomInt(0, kWindowWidth); y = -30; }
                if (side == 1) { x = kWindowWidth + 30; y = randomInt(0, kWindowHeight); }
                if (side == 2) { x = randomInt(0, kWindowWidth); y = kWindowHeight + 30; }
                if (side == 3) { x = -30; y = randomInt(0, kWindowHeight); }
                enemy.spawn(x, y);
                return;
            }
        }
    }

    void spawnPickup() {
        for (auto& pickup : pickups) {
            if (!pickup.active) {
                float x = randomInt(80, static_cast<int>(kWindowWidth) - 80);
                float y = randomInt(80, static_cast<int>(kWindowHeight) - 80);
                Pickup::Type type = (randomInt(0, 1) == 0) ? Pickup::Type::Heal : Pickup::Type::Score;
                pickup.spawn(x, y, type);
                return;
            }
        }
    }

    void handleInput(float dt) {
        if (gameOver) {
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
                resetGame();
            }
            return;
        }

        bool moveLeft = sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
        bool moveRight = sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
        bool moveUp = sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
        bool moveDown = sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down);

        sf::Vector2i mousePos = sf::Mouse::getPosition(window);
        sf::Vector2f worldMouse = window.mapPixelToCoords(mousePos);

        bool shoot = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        player.update(dt, worldMouse, moveLeft, moveRight, moveUp, moveDown, shoot);

        if (shoot && player.shootCooldown <= 0.0f) {
            // fire a bullet toward mouse
            Vec2 target = {worldMouse.x, worldMouse.y};
            Vec2 from = {player.pos.x, player.pos.y};
            Vec2 dir = target - from;
            float len = dir.length();
            if (len > 0.001f) {
                dir = dir / len;
                for (auto& bullet : bullets) {
                    if (!bullet.active) {
                        bullet.fire(from, dir);
                        break;
                    }
                }
            }
        }
    }

    void update(float dt) {
        if (gameOver) {
            return;
        }

        handleInput(dt);

        // waves
        spawnTimer -= dt;
        if (spawnTimer <= 0.0f) {
            int count = 1 + wave / 2;
            for (int i = 0; i < count; ++i) {
                spawnEnemy();
            }
            spawnTimer = std::max(0.8f, kEnemySpawnDelay - (wave * 0.08f));
            wave += 1;
        }

        pickupTimer -= dt;
        if (pickupTimer <= 0.0f) {
            spawnPickup();
            pickupTimer = kPickupSpawnDelay;
        }

        for (auto& bullet : bullets) {
            bullet.update(dt);
        }

        for (auto& enemy : enemies) {
            if (!enemy.active) continue;
            enemy.update(dt, {player.pos.x, player.pos.y});

            if (enemy.getBounds().intersects(player.getBounds())) {
                player.hp -= 1;
                enemy.active = false;
                if (player.hp <= 0) {
                    gameOver = true;
                }
            }
        }

        for (auto& bullet : bullets) {
            if (!bullet.active) continue;
            for (auto& enemy : enemies) {
                if (!enemy.active) continue;
                sf::FloatRect bulletRect(
                    bullet.pos.x - bullet.radius,
                    bullet.pos.y - bullet.radius,
                    bullet.radius * 2.0f,
                    bullet.radius * 2.0f);
                if (bulletRect.intersects(enemy.getBounds())) {
                    bullet.active = false;
                    enemy.active = false;
                    player.score += enemy.scoreValue;
                    if (randomInt(0, 10) > 7) {
                        spawnPickup();
                    }
                    break;
                }
            }
        }

        for (auto& pickup : pickups) {
            if (!pickup.active) continue;
            pickup.update(dt);
            if (pickup.getBounds().intersects(player.getBounds())) {
                if (pickup.type == Pickup::Type::Heal) {
                    player.hp = std::min(player.maxHp, player.hp + 1);
                } else {
                    player.score += 25;
                }
                pickup.active = false;
            }
        }

        // clamp score and wave
        titleText.setString("PIXEL ARENA");
        scoreText.setString("Score: " + std::to_string(player.score));
        hpText.setString("HP: " + std::to_string(player.hp) + " / " + std::to_string(player.maxHp));
        waveText.setString("Wave: " + std::to_string(wave));
    }

    void draw() {
        window.clear(sf::Color(18, 22, 34));

        // draw a grid background to feel pixel-art
        for (int x = 0; x < kWindowWidth; x += 32) {
            for (int y = 0; y < kWindowHeight; y += 32) {
                sf::RectangleShape cell(sf::Vector2f(32.0f, 32.0f));
                cell.setFillColor((x / 32 + y / 32) % 2 == 0 ? sf::Color(20, 26, 38) : sf::Color(24, 31, 45));
                cell.setPosition(static_cast<float>(x), static_cast<float>(y));
                window.draw(cell);
            }
        }

        for (auto& pickup : pickups) {
            pickup.draw(window);
        }

        for (auto& bullet : bullets) {
            bullet.draw(window);
        }

        for (auto& enemy : enemies) {
            enemy.draw(window);
        }

        player.draw(window);

        window.draw(titleText);
        window.draw(scoreText);
        window.draw(hpText);
        window.draw(waveText);

        if (gameOver) {
            sf::RectangleShape overlay(sf::Vector2f(kWindowWidth, kWindowHeight));
            overlay.setFillColor(sf::Color(0, 0, 0, 150));
            window.draw(overlay);
            window.draw(gameOverText);
        }

        window.display();
    }

    void run() {
        sf::Clock clock;
        while (window.isOpen()) {
            float dt = clock.restart().asSeconds();

            sf::Event event;
            while (window.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    window.close();
                }
                if (event.type == sf::Event::MouseButtonPressed) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        leftMousePressed = true;
                    }
                }
                if (event.type == sf::Event::MouseButtonReleased) {
                    if (event.mouseButton.button == sf::Mouse::Left) {
                        leftMousePressed = false;
                    }
                }
            }

            update(dt);
            draw();
        }
    }
};
}  // namespace

int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
