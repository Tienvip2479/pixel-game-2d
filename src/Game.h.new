#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "Enemy.h"
#include "Map.h"
#include "Player.h"
#include "UI.h"

struct Bullet {
    bool active = false;
    sf::CircleShape shape;
    Vec2 pos;
    Vec2 vel;
    float radius = 5.f;
    int damage = 1;

    Bullet();
    void fire(float x, float y, float vx, float vy, int damage_);
    void update(float dt);
    void draw(sf::RenderWindow& window) const;
    sf::FloatRect bounds() const;
};

struct Pickup {
    enum class Type { Health, Ammo, Score };
    bool active = false;
    Type type = Type::Score;
    sf::CircleShape shape;
    Vec2 pos;
    float radius = 10.f;

    Pickup();
    void spawn(float x, float y, Type type_);
    void draw(sf::RenderWindow& window) const;
    sf::FloatRect bounds() const;
};

struct Particle {
    bool active = false;
    sf::RectangleShape shape;
    Vec2 pos;
    Vec2 vel;
    float life = 0.f;
    float maxLife = 0.f;

    Particle();
    void spawn(float x, float y, float vx, float vy, float size, sf::Color color, float lifetime);
    void update(float dt);
    void draw(sf::RenderWindow& window) const;
};

class Game {
public:
    Game();
    void run();

private:
    void reset();
    void handleInput(float dt);
    void update(float dt);
    void draw();
    void drawBackground();
    void spawnEnemy();
    void spawnPickup();
    void spawnParticles(float x, float y, int count, sf::Color color);
    void firePlayerBullet(const sf::Vector2f& mouseWorld);
    void updateBullets(float dt);
    void updateEnemies(float dt);
    void updatePickups(float dt);
    void updateParticles(float dt);
    void updateWave(float dt);

    sf::RenderWindow window_;
    Map map_;
    Player player_;
    UI ui_;

    std::vector<Enemy> enemies_;
    std::vector<Bullet> bullets_;
    std::vector<Pickup> pickups_;
    std::vector<Particle> particles_;

    float enemySpawnTimer_ = 0.f;
    float pickupTimer_ = 0.f;
    int wave_ = 1;
    bool gameOver_ = false;
    bool victory_ = false;
    std::string status_ = "Survive the dungeon";
};
