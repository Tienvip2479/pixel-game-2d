#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "Equipment.h"

struct Vec2;

class Player {
public:
    Player();
    void update(float dt, bool left, bool right, bool up, bool down,
                const sf::Vector2f& mouseWorld, const Map& map);
    void draw(sf::RenderWindow& window) const;
    sf::FloatRect bounds() const;
    void recalcStats();

    Vec2 pos;
    float radius = 16.f;
    int hp = 5;
    int maxHp = 5;
    int score = 0;
    int ammo = 12;
    int maxAmmo = 12;
    float shootCooldown = 0.f;
    sf::CircleShape shape;
    Vec2 aim;
    bool alive = true;

    CharacterStats baseStats;
    CharacterStats effectiveStats;
    EquipmentSystem equipment;
    float moveSpeed = 220.f;
};
