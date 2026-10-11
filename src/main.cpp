#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <map>
#include <random>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int WINDOW_HEIGHT = 720;
constexpr int TILE_SIZE = 32;
constexpr int MAP_COLS = 40;
constexpr int MAP_ROWS = 22;
constexpr float GRAVITY = 0.0f;
constexpr float PLAYER_SPEED = 220.0f;
constexpr float PLAYER_ATTACK_COOLDOWN = 0.20f;
constexpr float ENEMY_BASE_SPEED = 70.0f;
constexpr float BULLET_SPEED = 520.0f;
constexpr float PLAYER_RADIUS = 16.0f;
constexpr int MAX_BULLETS = 200;
constexpr int MAX_ENEMIES = 200;

struct Vec2 {
    float x = 0.0f;
    float y = 0.0f;

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    static Vec2 zero() { return Vec2(0.0f, 0.0f); }

    float length() const { return std::sqrt(x * x + y * y); }
    Vec2 normalized() const {
        float len = length();
        if (len <= 0.0001f) return Vec2::zero();
        return Vec2(x / len, y / len);
    }

    Vec2 operator+(const Vec2& other) const { return Vec2(x + other.x, y + other.y); }
    Vec2 operator-(const Vec2& other) const { return Vec2(x - other.x, y - other.y); }
    Vec2 operator*(float value) const { return Vec2(x * value, y * value); }
    Vec2 operator/(float value) const { return Vec2(x / value, y / value); }
    Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
    Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
    Vec2& operator*=(float value) { x *= value; y *= value; return *this; }
    Vec2& operator/=(float value) { x /= value; y /= value; return *this; }
};

float clamp(float value, float low, float high) {
    return std::max(low, std::min(high, value));
}

int randomRange(int minv, int maxv) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(minv, maxv);
    return dist(rng);
}

bool rectsIntersect(const sf::FloatRect& a, const sf::FloatRect& b) {
    return a.intersects(b);
}

sf::Color lerpColor(sf::Color a, sf::Color b, float t) {
    t = clamp(t, 0.0f, 1.0f);
    return sf::Color(
        static_cast<sf::Uint8>(a.r + (b.r - a.r) * t),
        static_cast<sf::Uint8>(a.g + (b.g - a.g) * t),
        static_cast<sf::Uint8>(a.b + (b.b - a.b) * t),
        static_cast<sf::Uint8>(a.a + (b.a - a.a) * t)
    );
}

std::string toString(int value) {
    std::stringstream ss;
    ss << value;
    return ss.str();
}

class Camera {
public:
    sf::View view;
    Camera() {
        view.setSize(static_cast<float>(WINDOW_WIDTH), static_cast<float>(WINDOW_HEIGHT));
        view.setCenter(static_cast<float>(WINDOW_WIDTH) / 2.0f, static_cast<float>(WINDOW_HEIGHT) / 2.0f);
    }

    void update(float targetX, float targetY) {
        float cx = clamp(targetX, WINDOW_WIDTH * 0.5f, MAP_COLS * TILE_SIZE - WINDOW_WIDTH * 0.5f);
        float cy = clamp(targetY, WINDOW_HEIGHT * 0.5f, MAP_ROWS * TILE_SIZE - WINDOW_HEIGHT * 0.5f);
        view.setCenter(cx, cy);
    }
};

class TileMap {
public:
    std::vector<int> tiles;
    sf::Texture texture;
    sf::VertexArray vertices;

    TileMap() {
        tiles.resize(MAP_COLS * MAP_ROWS, 0);
        buildMap();
        loadTexture();
        buildVertices();
    }

    void buildMap() {
        // full floor
        for (int y = 0; y < MAP_ROWS; ++y) {
            for (int x = 0; x < MAP_COLS; ++x) {
                tiles[y * MAP_COLS + x] = 0;
            }
        }

        // walls around edges
        for (int x = 0; x < MAP_COLS; ++x) {
            tiles[0 * MAP_COLS + x] = 1;
            tiles[(MAP_ROWS - 1) * MAP_COLS + x] = 1;
        }
        for (int y = 0; y < MAP_ROWS; ++y) {
            tiles[y * MAP_COLS + 0] = 1;
            tiles[y * MAP_COLS + (MAP_COLS - 1)] = 1;
        }

        // inner obstacles
        std::vector<std::pair<int, int>> blocks = {
            {5, 5}, {8, 5}, {11, 5}, {14, 5}, {18, 5}, {23, 5},
            {30, 5}, {33, 5},
            {7, 10}, {12, 10}, {18, 10}, {24, 10}, {30, 10}, {34, 10},
            {5, 15}, {10, 15}, {14, 15}, {21, 15}, {27, 15}, {32, 15},
            {7, 18}, {12, 18}, {18, 18}, {24, 18}, {29, 18}, {34, 18},
        };

        for (auto& b : blocks) {
            int x = b.first;
            int y = b.second;
            for (int yy = 0; yy < 2; ++yy) {
                for (int xx = 0; xx < 2; ++xx) {
                    if (x + xx < MAP_COLS && y + yy < MAP_ROWS) {
                        tiles[(y + yy) * MAP_COLS + (x + xx)] = 1;
                    }
                }
            }
        }

        // rectangles and extra rooms
        for (int x = 20; x < 26; ++x) {
            for (int y = 7; y < 13; ++y) {
                tiles[y * MAP_COLS + x] = 1;
            }
        }
        for (int x = 27; x < 34; ++x) {
            for (int y = 12; y < 17; ++y) {
                tiles[y * MAP_COLS + x] = 1;
            }
        }
    }

    void loadTexture() {
        sf::Image image;
        image.create(64, 64, sf::Color::White);

        for (unsigned int y = 0; y < image.getSize().y; ++y) {
            for (unsigned int x = 0; x < image.getSize().x; ++x) {
                bool edge = x < 10 || y < 10 || x > 53 || y > 53;
                sf::Color c = edge ? sf::Color(71, 82, 99) : sf::Color(34, 49, 63);
                image.setPixel(x, y, c);
            }
        }

        texture.loadFromImage(image);
    }

    void buildVertices() {
        vertices.setPrimitiveType(sf::Quads);
        vertices.resize(MAP_COLS * MAP_ROWS * 4);

        for (int y = 0; y < MAP_ROWS; ++y) {
            for (int x = 0; x < MAP_COLS; ++x) {
                int tile = tiles[y * MAP_COLS + x];
                if (tile == 0) continue;

                int index = (y * MAP_COLS + x) * 4;
                sf::Vertex* quad = &vertices[index];

                quad[0].position = sf::Vector2f(x * TILE_SIZE, y * TILE_SIZE);
                quad[1].position = sf::Vector2f((x + 1) * TILE_SIZE, y * TILE_SIZE);
                quad[2].position = sf::Vector2f((x + 1) * TILE_SIZE, (y + 1) * TILE_SIZE);
                quad[3].position = sf::Vector2f(x * TILE_SIZE, (y + 1) * TILE_SIZE);

                quad[0].texCoords = sf::Vector2f(0.0f, 0.0f);
                quad[1].texCoords = sf::Vector2f(32.0f, 0.0f);
                quad[2].texCoords = sf::Vector2f(32.0f, 32.0f);
                quad[3].texCoords = sf::Vector2f(0.0f, 32.0f);
            }
        }
    }

    bool isWall(int x, int y) const {
        if (x < 0 || y < 0 || x >= MAP_COLS || y >= MAP_ROWS) return true;
        return tiles[y * MAP_COLS + x] != 0;
    }

    bool collides(const sf::FloatRect& rect) const {
        int left = static_cast<int>(rect.left / TILE_SIZE);
        int right = static_cast<int>((rect.left + rect.width) / TILE_SIZE);
        int top = static_cast<int>(rect.top / TILE_SIZE);
        int bottom = static_cast<int>((rect.top + rect.height) / TILE_SIZE);

        for (int y = top; y <= bottom; ++y) {
            for (int x = left; x <= right; ++x) {
                if (isWall(x, y)) return true;
            }
        }
        return false;
    }

    void draw(sf::RenderWindow& window, const sf::View& view) {
        sf::RenderStates states;
        states.texture = &texture;
        window.setView(view);
        window.draw(vertices, states);
    }
};

class Particle {
public:
    Vec2 pos;
    Vec2 vel;
    sf::RectangleShape shape;
    float life = 0.0f;
    float maxLife = 0.0f;
    bool active = false;

    Particle() = default;

    void spawn(float x, float y, float vx, float vy, float size, sf::Color color, float duration) {
        pos = Vec2(x, y);
        vel = Vec2(vx, vy);
        shape.setSize(sf::Vector2f(size, size));
        shape.setFillColor(color);
        shape.setPosition(x, y);
        life = duration;
        maxLife = duration;
        active = true;
    }

    void update(float dt) {
        if (!active) return;
        pos += vel * dt;
        vel.y += 160.0f * dt;
        shape.setPosition(pos.x, pos.y);
        life -= dt;
        if (life <= 0.0f) active = false;
    }

    void draw(sf::RenderWindow& window) {
        if (active) window.draw(shape);
    }
};

class Projectile {
public:
    Vec2 pos;
    Vec2 dir;
    float radius = 5.0f;
    sf::CircleShape shape;
    bool active = false;
    int damage = 1;
    sf::Color color;

    Projectile() {
        shape.setRadius(radius);
        shape.setFillColor(sf::Color(255, 200, 80));
        shape.setOutlineThickness(1.0f);
        shape.setOutlineColor(sf::Color(110, 80, 20));
    }

    void fire(float x, float y, float dx, float dy, sf::Color c = sf::Color(255, 200, 80), int dmg = 1) {
        pos = Vec2(x, y);
        dir = Vec2(dx, dy).normalized();
        active = true;
        damage = dmg;
        color = c;
        shape.setFillColor(c);
        shape.setPosition(pos.x - radius, pos.y - radius);
    }

    void update(float dt) {
        if (!active) return;
        pos += dir * BULLET_SPEED * dt;
        shape.setPosition(pos.x - radius, pos.y - radius);
        if (pos.x < -20 || pos.x > MAP_COLS * TILE_SIZE + 20 || pos.y < -20 || pos.y > MAP_ROWS * TILE_SIZE + 20) {
            active = false;
        }
    }

    void draw(sf::RenderWindow& window) {
        if (active) window.draw(shape);
    }

    sf::FloatRect bounds() const {
        return sf::FloatRect(pos.x - radius, pos.y - radius, radius * 2.0f, radius * 2.0f);
    }
};

class Enemy {
public:
    Vec2 pos;
    Vec2 vel;
    float radius = 16.0f;
    sf::CircleShape shape;
    bool active = false;
    int hp = 1;
    int scoreValue = 10;
    float attackCooldown = 0.0f;
    int type = 0; // 0 normal, 1 fast, 2 tank

    Enemy() {
        shape.setRadius(radius);
        shape.setOutlineThickness(2.0f);
    }

    void spawn(float x, float y, int level) {
        pos = Vec2(x, y);
        vel = Vec2::zero();
        active = true;
        int roll = randomRange(0, 100);
        if (roll < 60) {
            type = 0;
            hp = 1 + (level / 6);
            shape.setFillColor(sf::Color(220, 80, 80));
            shape.setOutlineColor(sf::Color(90, 25, 25));
            radius = 16.0f;
            scoreValue = 10;
        } else if (roll < 88) {
            type = 1;
            hp = 1 + (level / 8);
            shape.setFillColor(sf::Color(80, 180, 235));
            shape.setOutlineColor(sf::Color(20, 80, 110));
            radius = 13.0f;
            scoreValue = 18;
        } else {
            type = 2;
            hp = 2 + (level / 5);
            shape.setFillColor(sf::Color(170, 110, 245));
            shape.setOutlineColor(sf::Color(70, 40, 100));
            radius = 20.0f;
            scoreValue = 35;
        }
        shape.setRadius(radius);
        shape.setPosition(pos.x - radius, pos.y - radius);
    }

    void update(float dt, const Vec2& target, const TileMap& map) {
        if (!active) return;
        Vec2 dir = target - pos;
        float dist = dir.length();
        if (dist > 0.001f) {
            dir = dir / dist;
            if (type == 0) vel = dir * (ENEMY_BASE_SPEED + (float)randomRange(0, 10));
            if (type == 1) vel = dir * (ENEMY_BASE_SPEED * 1.5f);
            if (type == 2) vel = dir * (ENEMY_BASE_SPEED * 0.8f);
        } else {
            vel = Vec2::zero();
        }

        Vec2 next = pos + vel * dt;
        sf::FloatRect testRect(next.x - radius, next.y - radius, radius * 2.0f, radius * 2.0f);
        if (!map.collides(testRect)) {
            pos = next;
        } else {
            vel = Vec2::zero();
        }

        shape.setPosition(pos.x - radius, pos.y - radius);
        attackCooldown -= dt;
    }

    void hit(int damage) {
        hp -= damage;
        if (hp <= 0) active = false;
    }

    sf::FloatRect bounds() const {
        return sf::FloatRect(pos.x - radius, pos.y - radius, radius * 2.0f, radius * 2.0f);
    }

    void draw(sf::RenderWindow& window) {
        if (active) window.draw(shape);
    }
};

class Pickup {
public:
    enum class Type { Health, Ammo, Score };
    Vec2 pos;
    float radius = 10.0f;
    sf::CircleShape shape;
    bool active = false;
    Type type = Type::Score;

    void spawn(float x, float y, Type t) {
        pos = Vec2(x, y);
        type = t;
        active = true;
        if (type == Type::Health) {
            shape.setFillColor(sf::Color(130, 255, 160));
            shape.setOutlineColor(sf::Color(25, 100, 50));
        } else if (type == Type::Ammo) {
            shape.setFillColor(sf::Color(110, 170, 255));
            shape.setOutlineColor(sf::Color(30, 60, 115));
        } else {
            shape.setFillColor(sf::Color(255, 215, 80));
            shape.setOutlineColor(sf::Color(140, 90, 35));
        }
        shape.setRadius(radius);
        shape.setPosition(x - radius, y - radius);
    }

    sf::FloatRect bounds() const {
        return sf::FloatRect(pos.x - radius, pos.y - radius, radius * 2.0f, radius * 2.0f);
    }

    void draw(sf::RenderWindow& window) {
        if (active) window.draw(shape);
    }
};

class Player {
public:
    Vec2 pos;
    float radius = PLAYER_RADIUS;
    sf::CircleShape shape;
    int hp = 5;
    int maxHp = 5;
    int ammo = 12;
    int maxAmmo = 12;
    int score = 0;
    int level = 1;
    float fireCooldown = 0.0f;
    sf::Vector2f aimDir;

    Player() {
        pos = Vec2(200.0f, 200.0f);
        shape.setRadius(radius);
        shape.setFillColor(sf::Color(80, 220, 180));
        shape.setOutlineColor(sf::Color(18, 90, 75));
        shape.setOutlineThickness(2.0f);
        shape.setPosition(pos.x - radius, pos.y - radius);
    }

    void reset() {
        pos = Vec2(200.0f, 200.0f);
        hp = 5;
        maxHp = 5;
        ammo = 12;
        maxAmmo = 12;
        score = 0;
        level = 1;
        fireCooldown = 0.0f;
        shape.setPosition(pos.x - radius, pos.y - radius);
    }

    void update(float dt, bool moveLeft, bool moveRight, bool moveUp, bool moveDown, const sf::Vector2f& mouseWorld) {
        Vec2 mov = Vec2::zero();
        if (moveLeft) mov.x -= 1.0f;
        if (moveRight) mov.x += 1.0f;
        if (moveUp) mov.y -= 1.0f;
        if (moveDown) mov.y += 1.0f;
        if (mov.length() > 0.0f) {
            mov = mov.normalized();
            pos += mov * PLAYER_SPEED * dt;
        }

        pos.x = clamp(pos.x, radius, MAP_COLS * TILE_SIZE - radius);
        pos.y = clamp(pos.y, radius, MAP_ROWS * TILE_SIZE - radius);
        shape.setPosition(pos.x - radius, pos.y - radius);

        fireCooldown = std::max(0.0f, fireCooldown - dt);

        sf::Vector2f delta = mouseWorld - sf::Vector2f(pos.x, pos.y);
        if (std::abs(delta.x) > 0.001f || std::abs(delta.y) > 0.001f) {
            aimDir = delta;
        }
    }

    sf::FloatRect bounds() const {
        return sf::FloatRect(pos.x - radius, pos.y - radius, radius * 2.0f, radius * 2.0f);
    }

    void draw(sf::RenderWindow& window) {
        window.draw(shape);

        // pointer line showing aim
        if (aimDir.x != 0 || aimDir.y != 0) {
            sf::Vertex line[] = {
                sf::Vertex(sf::Vector2f(pos.x, pos.y), sf::Color(210, 250, 245)),
                sf::Vertex(sf::Vector2f(pos.x + aimDir.x * 0.15f, pos.y + aimDir.y * 0.15f), sf::Color(210, 250, 245))
            };
            window.draw(line, 2, sf::Lines);
        }
    }
};

class HUD {
public:
    sf::Font font;
    sf::Text hpText;
    sf::Text ammoText;
    sf::Text scoreText;
    sf::Text levelText;
    sf::Text messageText;
    sf::Text waveText;

    HUD() {
        if (!font.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
            std::cerr << "Font fallback not loaded.\n";
        }

        hpText.setFont(font); hpText.setCharacterSize(22); hpText.setFillColor(sf::Color(255, 120, 120));
        ammoText.setFont(font); ammoText.setCharacterSize(22); ammoText.setFillColor(sf::Color(120, 180, 255));
        scoreText.setFont(font); scoreText.setCharacterSize(22); scoreText.setFillColor(sf::Color(255, 220, 110));
        levelText.setFont(font); levelText.setCharacterSize(22); levelText.setFillColor(sf::Color(150, 220, 255));
        waveText.setFont(font); waveText.setCharacterSize(22); waveText.setFillColor(sf::Color(170, 240, 190));
        messageText.setFont(font); messageText.setCharacterSize(30); messageText.setFillColor(sf::Color(240, 240, 240));
    }

    void update(const Player& player, int currentWave, const std::string& message) {
        hpText.setString("HP: " + std::to_string(player.hp) + " / " + std::to_string(player.maxHp));
        ammoText.setString("Ammo: " + std::to_string(player.ammo) + " / " + std::to_string(player.maxAmmo));
        scoreText.setString("Score: " + std::to_string(player.score));
        levelText.setString("Level: " + std::to_string(player.level));
        waveText.setString("Wave: " + std::to_string(currentWave));
        messageText.setString(message);
    }

    void draw(sf::RenderWindow& window) {
        sf::View old = window.getView();
        window.setView(window.getDefaultView());

        hpText.setPosition(20.0f, 20.0f);
        ammoText.setPosition(20.0f, 52.0f);
        scoreText.setPosition(20.0f, 84.0f);
        levelText.setPosition(20.0f, 116.0f);
        waveText.setPosition(20.0f, 148.0f);
        messageText.setPosition(350.0f, 20.0f);

        window.draw(hpText);
        window.draw(ammoText);
        window.draw(scoreText);
        window.draw(levelText);
        window.draw(waveText);
        window.draw(messageText);

        window.setView(old);
    }
};

class Game {
public:
    sf::RenderWindow window;
    TileMap map;
    Camera camera;
    Player player;
    HUD hud;
    std::vector<Projectile> projectiles;
    std::vector<Enemy> enemies;
    std::vector<Pickup> pickups;
    std::vector<Particle> particles;
    float spawnTimer = 0.0f;
    float pickupTimer = 0.0f;
    int currentWave = 1;
    bool gameOver = false;
    bool victory = false;
    std::string statusMessage = "Escort the relic. Survive.";

    Game() : window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Pixel Dungeon Survivor", sf::Style::Close) {
        window.setFramerateLimit(60);
        window.setVerticalSyncEnabled(true);
        projectiles.resize(MAX_BULLETS);
        enemies.resize(MAX_ENEMIES);
        pickups.resize(30);
        for (auto& projectile : projectiles) { projectile.active = false; }
        for (auto& enemy : enemies) { enemy.active = false; }
        for (auto& pickup : pickups) { pickup.active = false; }
        particles.resize(500);
        reset();
    }

    void reset() {
        player.reset();
        gameOver = false;
        victory = false;
        currentWave = 1;
        spawnTimer = 0.2f;
        pickupTimer = 4.0f;
        statusMessage = "Survive the dungeon. Find loot.";
        for (auto& projectile : projectiles) { projectile.active = false; }
        for (auto& enemy : enemies) { enemy.active = false; }
        for (auto& pickup : pickups) { pickup.active = false; }
        for (auto& particle : particles) { particle.active = false; }
    }

    void spawnEnemy() {
        for (auto& enemy : enemies) {
            if (!enemy.active) {
                float x = 0.0f;
                float y = 0.0f;
                int side = randomRange(0, 3);
                if (side == 0) { x = randomRange(0, MAP_COLS * TILE_SIZE); y = -50.0f; }
                if (side == 1) { x = MAP_COLS * TILE_SIZE + 50.0f; y = randomRange(0, MAP_ROWS * TILE_SIZE); }
                if (side == 2) { x = randomRange(0, MAP_COLS * TILE_SIZE); y = MAP_ROWS * TILE_SIZE + 50.0f; }
                if (side == 3) { x = -50.0f; y = randomRange(0, MAP_ROWS * TILE_SIZE); }
                enemy.spawn(x, y, currentWave);
                return;
            }
        }
    }

    void spawnPickup(float x, float y) {
        int roll = randomRange(0, 100);
        Pickup::Type type = Pickup::Type::Score;
        if (roll < 30) type = Pickup::Type::Health;
        else if (roll < 60) type = Pickup::Type::Ammo;
        else type = Pickup::Type::Score;

        for (auto& pickup : pickups) {
            if (!pickup.active) {
                pickup.spawn(x, y, type);
                return;
            }
        }
    }

    void spawnParticles(float x, float y, int count, sf::Color color) {
        for (int i = 0; i < count; ++i) {
            for (auto& p : particles) {
                if (!p.active) {
                    float vx = randomRange(-80, 80);
                    float vy = randomRange(-80, 80);
                    p.spawn(x, y, vx, vy, randomRange(2, 5), color, 0.5f + (float)randomRange(0, 30) / 100.0f);
                    break;
                }
            }
        }
    }

    void firePlayerBullet(const sf::Vector2f& mouseWorld) {
        if (player.fireCooldown > 0.0f || player.ammo <= 0) return;

        player.fireCooldown = PLAYER_ATTACK_COOLDOWN;
        player.ammo -= 1;
        float dx = mouseWorld.x - player.pos.x;
        float dy = mouseWorld.y - player.pos.y;
        float len = std::sqrt(dx * dx + dy * dy);

        if (len > 0.0001f) {
            dx /= len;
            dy /= len;
        }

        for (auto& projectile : projectiles) {
            if (!projectile.active) {
                projectile.fire(player.pos.x, player.pos.y, dx, dy, sf::Color(255, 210, 80), 1);
                break;
            }
        }
    }

    void updatePlayer(float dt, const sf::Vector2f& mouseWorld) {
        bool left = sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
        bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
        bool up = sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
        bool down = sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
        player.update(dt, left, right, up, down, mouseWorld);

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            firePlayerBullet(mouseWorld);
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R) && (gameOver || victory)) {
            reset();
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && player.ammo < player.maxAmmo) {
            player.ammo = std::min(player.maxAmmo, player.ammo + 1);
        }
    }

    void updateProjectiles(float dt) {
        for (auto& projectile : projectiles) {
            if (!projectile.active) continue;
            projectile.update(dt);

            sf::FloatRect pb = projectile.bounds();
            for (auto& enemy : enemies) {
                if (!enemy.active) continue;
                if (rectsIntersect(pb, enemy.bounds())) {
                    enemy.hit(projectile.damage);
                    projectile.active = false;
                    spawnParticles(projectile.pos.x, projectile.pos.y, 8, sf::Color(255, 180, 80));
                    if (!enemy.active) {
                        player.score += enemy.scoreValue;
                        spawnParticles(enemy.pos.x, enemy.pos.y, 15, sf::Color(245, 90, 90));
                    }
                    break;
                }
            }
        }
    }

    void updateEnemies(float dt) {
        for (auto& enemy : enemies) {
            if (!enemy.active) continue;
            enemy.update(dt, Vec2(player.pos.x, player.pos.y), map);

            if (enemy.bounds().intersects(player.bounds())) {
                player.hp -= 1;
                spawnParticles(player.pos.x, player.pos.y, 20, sf::Color(255, 70, 70));
                enemy.active = false;
                if (player.hp <= 0) {
                    gameOver = true;
                    statusMessage = "You were overwhelmed by the dungeon.";
                }
            }
        }
    }

    void updatePickups(float dt) {
        for (auto& pickup : pickups) {
            if (!pickup.active) continue;
            pickup.shape.setPosition(pickup.pos.x - pickup.radius, pickup.pos.y - pickup.radius);
            if (pickup.bounds().intersects(player.bounds())) {
                if (pickup.type == Pickup::Type::Health) {
                    player.hp = std::min(player.maxHp, player.hp + 2);
                    statusMessage = "You recovered health.";
                } else if (pickup.type == Pickup::Type::Ammo) {
                    player.ammo = std::min(player.maxAmmo, player.ammo + 6);
                    statusMessage = "Ammo restocked.";
                } else {
                    player.score += 50;
                    statusMessage = "You found treasure.";
                }
                spawnParticles(pickup.pos.x, pickup.pos.y, 20, sf::Color(255, 220, 100));
                pickup.active = false;
            }
        }
    }

    void updateParticles(float dt) {
        for (auto& p : particles) {
            if (p.active) p.update(dt);
        }
    }

    void updateWave(float dt) {
        spawnTimer -= dt;
        if (spawnTimer <= 0.0f) {
            int count = 1 + currentWave / 2;
            for (int i = 0; i < count; ++i) {
                spawnEnemy();
            }
            currentWave += 1;
            spawnTimer = std::max(0.55f, 2.8f - static_cast<float>(currentWave) * 0.12f);
            statusMessage = "Wave increased. The dungeon is getting faster.";
        }

        pickupTimer -= dt;
        if (pickupTimer <= 0.0f) {
            float x = randomRange(120, MAP_COLS * TILE_SIZE - 120);
            float y = randomRange(120, MAP_ROWS * TILE_SIZE - 120);
            spawnPickup(x, y);
            pickupTimer = 7.0f;
        }
    }

    void updateGame(float dt) {
        if (gameOver || victory) return;

        sf::Vector2i mouse = sf::Mouse::getPosition(window);
        sf::Vector2f mouseWorld = window.mapPixelToCoords(mouse, camera.view);

        updatePlayer(dt, mouseWorld);
        updateWave(dt);
        updateProjectiles(dt);
        updateEnemies(dt);
        updatePickups(dt);
        updateParticles(dt);

        if (player.score >= 800 && currentWave >= 8) {
            victory = true;
            statusMessage = "The relic is safe. You survived the dungeon.";
        }

        camera.update(player.pos.x, player.pos.y);
        hud.update(player, currentWave, statusMessage);
    }

    void drawWorld() {
        window.setView(camera.view);

        // ground
        sf::RectangleShape ground(sf::Vector2f(MAP_COLS * TILE_SIZE, MAP_ROWS * TILE_SIZE));
        ground.setFillColor(sf::Color(24, 30, 42));
        ground.setPosition(0.0f, 0.0f);
        window.draw(ground);

        // decorative tiles
        for (int y = 0; y < MAP_ROWS; ++y) {
            for (int x = 0; x < MAP_COLS; ++x) {
                if (map.isWall(x, y)) {
                    sf::RectangleShape wall(sf::Vector2f(TILE_SIZE, TILE_SIZE));
                    wall.setPosition(x * TILE_SIZE, y * TILE_SIZE);
                    wall.setFillColor(sf::Color(59, 72, 88));
                    window.draw(wall);
                    sf::RectangleShape highlight(sf::Vector2f(TILE_SIZE - 6, TILE_SIZE - 6));
                    highlight.setPosition(x * TILE_SIZE + 3, y * TILE_SIZE + 3);
                    highlight.setFillColor(sf::Color(78, 96, 117));
                    window.draw(highlight);
                }
            }
        }

        for (auto& pickup : pickups) {
            pickup.draw(window);
        }

        for (auto& projectile : projectiles) {
            projectile.draw(window);
        }

        for (auto& enemy : enemies) {
            enemy.draw(window);
        }

        for (auto& p : particles) {
            p.draw(window);
        }

        player.draw(window);
    }

    void drawOverlay() {
        window.setView(window.getDefaultView());
        hud.draw(window);

        if (gameOver || victory) {
            sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
            overlay.setFillColor(sf::Color(0, 0, 0, 130));
            window.draw(overlay);

            sf::Text endText;
            endText.setFont(hud.font);
            endText.setCharacterSize(42);
            endText.setFillColor(sf::Color(255, 255, 255));
            endText.setString(victory ? "YOU WON!\nPress R to restart" : "GAME OVER\nPress R to restart");
            endText.setPosition(WINDOW_WIDTH * 0.5f - 210.0f, WINDOW_HEIGHT * 0.5f - 60.0f);
            window.draw(endText);
        }
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
                if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                    if (!gameOver && !victory) {
                        sf::Vector2i mouse = sf::Mouse::getPosition(window);
                        sf::Vector2f mouseWorld = window.mapPixelToCoords(mouse, camera.view);
                        firePlayerBullet(mouseWorld);
                    }
                }
            }

            updateGame(dt);
            window.clear(sf::Color(12, 18, 24));
            drawWorld();
            drawOverlay();
            window.display();
        }
    }
};

} // namespace

int main() {
    try {
        Game game;
        game.run();
    } catch (const std::exception& e) {
        std::cerr << "Fatal exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
