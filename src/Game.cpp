#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

#include <algorithm>
#include <cmath>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "Game.h"

namespace {
constexpr unsigned int WINDOW_WIDTH = 1280;
constexpr unsigned int WINDOW_HEIGHT = 720;
constexpr float PLAYER_BULLET_SPEED = 720.f;
constexpr int MAX_ENEMIES = 200;
constexpr int MAX_BULLETS = 200;
constexpr int MAX_PICKUPS = 16;
constexpr int MAX_PARTICLES = 600;

float clamp(float value, float low, float high) {
    return std::max(low, std::min(high, value));
}

int randomRange(int minv, int maxv) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(minv, maxv);
    return dist(rng);
}

} // namespace

Bullet::Bullet() {
    shape.setRadius(radius);
    shape.setFillColor(sf::Color(255, 216, 96));
    shape.setOutlineColor(sf::Color(120, 88, 18));
    shape.setOutlineThickness(1.f);
    active = false;
}

void Bullet::fire(float x, float y, float vx, float vy, int damage_) {
    active = true;
    pos = Vec2{x, y};
    vel = Vec2{vx, vy};
    radius = 5.f;
    damage = damage_;
    shape.setRadius(radius);
    shape.setPosition(pos.x - radius, pos.y - radius);
}

void Bullet::update(float dt) {
    if (!active) return;
    pos.x += vel.x * PLAYER_BULLET_SPEED * dt;
    pos.y += vel.y * PLAYER_BULLET_SPEED * dt;
    shape.setPosition(pos.x - radius, pos.y - radius);

    if (pos.x < -50 || pos.x > 2000 || pos.y < -50 || pos.y > 2000) {
        active = false;
    }
}

void Bullet::draw(sf::RenderWindow& window) const {
    if (active) window.draw(shape);
}

sf::FloatRect Bullet::bounds() const {
    return sf::FloatRect(pos.x - radius, pos.y - radius, radius * 2.f, radius * 2.f);
}

Pickup::Pickup() {
    shape.setRadius(radius);
    shape.setOutlineThickness(2.f);
}

void Pickup::spawn(float x, float y, Type type_) {
    active = true;
    pos = Vec2{x, y};
    type = type_;
    shape.setRadius(radius);
    shape.setPosition(x - radius, y - radius);
    if (type == Type::Health) {
        shape.setFillColor(sf::Color(110, 255, 160));
        shape.setOutlineColor(sf::Color(23, 102, 60));
    } else if (type == Type::Ammo) {
        shape.setFillColor(sf::Color(120, 170, 255));
        shape.setOutlineColor(sf::Color(24, 45, 110));
    } else {
        shape.setFillColor(sf::Color(255, 220, 100));
        shape.setOutlineColor(sf::Color(110, 80, 10));
    }
}

void Pickup::draw(sf::RenderWindow& window) const {
    if (active) window.draw(shape);
}

sf::FloatRect Pickup::bounds() const {
    return sf::FloatRect(pos.x - radius, pos.y - radius, radius * 2.f, radius * 2.f);
}

Particle::Particle() {
    shape.setSize(sf::Vector2f(4.f, 4.f));
}

void Particle::spawn(float x, float y, float vx, float vy, float size, sf::Color color, float lifetime) {
    active = true;
    pos = Vec2{x, y};
    vel = Vec2{vx, vy};
    shape.setSize(sf::Vector2f(size, size));
    shape.setFillColor(color);
    shape.setPosition(x, y);
    life = lifetime;
    maxLife = lifetime;
}

void Particle::update(float dt) {
    if (!active) return;
    pos.x += vel.x * dt;
    pos.y += vel.y * dt;
    vel.y += 120.f * dt;
    shape.setPosition(pos.x, pos.y);
    life -= dt;
    if (life <= 0.f) active = false;
}

void Particle::draw(sf::RenderWindow& window) const {
    if (active) window.draw(shape);
}

Game::Game()
    : window_(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), "Pixel Dungeon Survivor", sf::Style::Close),
      map_(),
      ui_() {
    window_.setVerticalSyncEnabled(true);
    window_.setFramerateLimit(60);
    enemies_.resize(MAX_ENEMIES);
    bullets_.resize(MAX_BULLETS);
    pickups_.resize(MAX_PICKUPS);
    particles_.resize(MAX_PARTICLES);
    reset();
}

void Game::reset() {
    player_ = Player();
    wave_ = 1;
    enemySpawnTimer_ = 1.0f;
    pickupTimer_ = 5.0f;
    gameOver_ = false;
    victory_ = false;
    status_ = "Survive the dungeon";

    for (auto& enemy : enemies_) {
        enemy.active = false;
    }
    for (auto& bullet : bullets_) {
        bullet.active = false;
    }
    for (auto& pickup : pickups_) {
        pickup.active = false;
    }
    for (auto& particle : particles_) {
        particle.active = false;
    }
}

void Game::spawnEnemy() {
    for (auto& enemy : enemies_) {
        if (!enemy.active) {
            int side = randomRange(0, 3);
            float x = 0.f;
            float y = 0.f;
            if (side == 0) {
                x = randomRange(0, Map::Cols * Map::TileSize);
                y = -30.f;
            } else if (side == 1) {
                x = Map::Cols * Map::TileSize + 30.f;
                y = randomRange(0, Map::Rows * Map::TileSize);
            } else if (side == 2) {
                x = randomRange(0, Map::Cols * Map::TileSize);
                y = Map::Rows * Map::TileSize + 30.f;
            } else {
                x = -30.f;
                y = randomRange(0, Map::Rows * Map::TileSize);
            }
            enemy.spawn(x, y, wave_);
            return;
        }
    }
}

void Game::spawnPickup() {
    for (auto& pickup : pickups_) {
        if (!pickup.active) {
            float x = randomRange(80, Map::Cols * Map::TileSize - 80);
            float y = randomRange(80, Map::Rows * Map::TileSize - 80);
            int roll = randomRange(0, 100);
            Pickup::Type type = Pickup::Type::Score;
            if (roll < 35) type = Pickup::Type::Health;
            else if (roll < 65) type = Pickup::Type::Ammo;
            pickup.spawn(x, y, type);
            return;
        }
    }
}

void Game::spawnParticles(float x, float y, int count, sf::Color color) {
    for (int i = 0; i < count; ++i) {
        for (auto& particle : particles_) {
            if (!particle.active) {
                float vx = randomRange(-100, 100);
                float vy = randomRange(-120, 30);
                particle.spawn(x, y, vx, vy, randomRange(2, 6), color, 0.6f + (randomRange(0, 50) / 100.f));
                break;
            }
        }
    }
}

void Game::firePlayerBullet(const sf::Vector2f& mouseWorld) {
    if (player_.shootCooldown > 0.f || player_.ammo <= 0 || gameOver_ || victory_) return;

    player_.shootCooldown = 0.18f;
    player_.ammo -= 1;

    float dx = mouseWorld.x - player_.pos.x;
    float dy = mouseWorld.y - player_.pos.y;
    float length = std::sqrt(dx * dx + dy * dy);
    if (length < 0.0001f) return;

    dx /= length;
    dy /= length;

    for (auto& bullet : bullets_) {
        if (!bullet.active) {
            bullet.fire(player_.pos.x, player_.pos.y, dx, dy, 1);
            break;
        }
    }
}

void Game::handleInput(float dt) {
    if (gameOver_ || victory_) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
            reset();
        }
        return;
    }

    bool left = sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
    bool right = sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
    bool up = sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
    bool down = sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down);

    sf::Vector2i mousePos = sf::Mouse::getPosition(window_);
    sf::Vector2f mouseWorld = window_.mapPixelToCoords(mousePos);
    player_.update(dt, left, right, up, down, mouseWorld, map_);

    if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
        firePlayerBullet(mouseWorld);
    }

    if (player_.ammo < player_.maxAmmo && sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        player_.ammo = std::min(player_.maxAmmo, player_.ammo + 1);
    }
}

void Game::updateBullets(float dt) {
    for (auto& bullet : bullets_) {
        if (!bullet.active) continue;
        bullet.update(dt);

        for (auto& enemy : enemies_) {
            if (!enemy.active) continue;
            if (bullet.bounds().intersects(enemy.bounds())) {
                enemy.hit(bullet.damage);
                bullet.active = false;
                if (!enemy.active) {
                    player_.score += enemy.value;
                    spawnParticles(enemy.pos.x, enemy.pos.y, 12, sf::Color(255, 116, 116));
                }
                break;
            }
        }
    }
}

void Game::updateEnemies(float dt) {
    for (auto& enemy : enemies_) {
        if (!enemy.active) continue;
        enemy.update(dt, Vec2{player_.pos.x, player_.pos.y}, map_);

        if (enemy.bounds().intersects(player_.bounds())) {
            player_.hp -= 1;
            spawnParticles(player_.pos.x, player_.pos.y, 18, sf::Color(255, 70, 70));
            enemy.active = false;
            if (player_.hp <= 0) {
                gameOver_ = true;
                status_ = "The dungeon overwhelms you";
            }
            break;
        }
    }
}

void Game::updatePickups(float dt) {
    (void)dt;
    for (auto& pickup : pickups_) {
        if (!pickup.active) continue;
        if (pickup.bounds().intersects(player_.bounds())) {
            if (pickup.type == Pickup::Type::Health) {
                player_.hp = std::min(player_.maxHp, player_.hp + 2);
                status_ = "Recovered health";
            } else if (pickup.type == Pickup::Type::Ammo) {
                player_.ammo = std::min(player_.maxAmmo, player_.ammo + 6);
                status_ = "Reloaded ammo";
            } else {
                player_.score += 50;
                status_ = "Treasure collected";
            }
            spawnParticles(pickup.pos.x, pickup.pos.y, 18, sf::Color(255, 220, 120));
            pickup.active = false;
        }
    }
}

void Game::updateParticles(float dt) {
    for (auto& particle : particles_) {
        if (particle.active) particle.update(dt);
    }
}

void Game::updateWave(float dt) {
    enemySpawnTimer_ -= dt;
    if (enemySpawnTimer_ <= 0.f) {
        int spawns = 1 + wave_ / 2;
        for (int i = 0; i < spawns; ++i) {
            spawnEnemy();
        }
        wave_ += 1;
        enemySpawnTimer_ = std::max(0.6f, 2.2f - wave_ * 0.08f);
        status_ = "Wave increased";
    }

    pickupTimer_ -= dt;
    if (pickupTimer_ <= 0.f) {
        spawnPickup();
        pickupTimer_ = 6.5f;
    }

    if (player_.score >= 800 && wave_ >= 9) {
        victory_ = true;
        status_ = "You survived the dungeon";
    }
}

void Game::update(float dt) {
    if (gameOver_ || victory_) {
        ui_.update(player_, wave_, status_);
        return;
    }

    handleInput(dt);
    updateWave(dt);
    updateBullets(dt);
    updateEnemies(dt);
    updatePickups(dt);
    updateParticles(dt);

    ui_.update(player_, wave_, status_);
}

void Game::drawBackground() {
    sf::RectangleShape bg(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
    bg.setFillColor(sf::Color(15, 18, 26));
    window_.draw(bg);

    for (int x = 0; x < WINDOW_WIDTH; x += 32) {
        for (int y = 0; y < WINDOW_HEIGHT; y += 32) {
            sf::RectangleShape cell(sf::Vector2f(32.f, 32.f));
            cell.setFillColor((x / 32 + y / 32) % 2 == 0 ? sf::Color(22, 28, 38) : sf::Color(18, 23, 34));
            cell.setPosition(static_cast<float>(x), static_cast<float>(y));
            window_.draw(cell);
        }
    }
}

void Game::draw() {
    window_.clear(sf::Color(12, 18, 25));
    drawBackground();

    window_.setView(window_.getDefaultView());
    map_.draw(window_, window_.getDefaultView());

    for (auto& pickup : pickups_) {
        pickup.draw(window_);
    }

    for (auto& bullet : bullets_) {
        bullet.draw(window_);
    }

    for (auto& enemy : enemies_) {
        enemy.draw(window_);
    }

    for (auto& particle : particles_) {
        particle.draw(window_);
    }

    player_.draw(window_);
    ui_.draw(window_);

    if (gameOver_ || victory_) {
        sf::RectangleShape overlay(sf::Vector2f(WINDOW_WIDTH, WINDOW_HEIGHT));
        overlay.setFillColor(sf::Color(0, 0, 0, 170));
        window_.draw(overlay);
        sf::Text endText;
        endText.setFont(ui_.font());
        endText.setCharacterSize(38);
        endText.setFillColor(sf::Color(255, 255, 255));
        endText.setString(victory_ ? "YOU WON\nPress R to restart" : "GAME OVER\nPress R to restart");
        endText.setPosition(WINDOW_WIDTH * 0.5f - 200.f, WINDOW_HEIGHT * 0.5f - 60.f);
        window_.draw(endText);
    }

    window_.display();
}

void Game::run() {
    sf::Clock clock;
    while (window_.isOpen()) {
        float dt = clock.restart().asSeconds();

        sf::Event event;
        while (window_.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window_.close();
            }
            if (event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left) {
                if (!gameOver_ && !victory_) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window_);
                    sf::Vector2f mouseWorld = window_.mapPixelToCoords(mousePos);
                    firePlayerBullet(mouseWorld);
                }
            }
        }

        update(dt);
        draw();
    }
}
