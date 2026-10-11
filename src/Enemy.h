#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

#include "Map.h"

class Enemy {
public:
    Enemy();
    void spawn(float x, float y, int wave);
    void update(float dt, const Vec2& target, const Map& map);
    void draw(sf::RenderWindow& window) const;
    sf::FloatRect bounds() const;
    void hit(int damage);

    Vec2 pos;
    Vec2 vel;
    float radius = 16.f;
    int hp = 1;
    int value = 10;
    bool active = false;
    int type = 0;
    sf::CircleShape shape;
};
