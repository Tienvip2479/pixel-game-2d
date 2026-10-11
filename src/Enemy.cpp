#include "Enemy.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace {
int randomRange(int minv, int maxv) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(minv, maxv);
    return dist(rng);
}
}

Enemy::Enemy() {
    shape.setOutlineThickness(2.f);
    active = false;
}

void Enemy::spawn(float x, float y, int wave) {
    pos = Vec2{x, y};
    vel = Vec2{0.f, 0.f};
    active = true;
    int roll = randomRange(0, 100);
    if (roll < 60) {
        type = 0;
        radius = 18.f;
        hp = 1 + wave / 6;
        value = 10;
        shape.setFillColor(sf::Color(220, 80, 80));
        shape.setOutlineColor(sf::Color(92, 20, 20));
    } else if (roll < 86) {
        type = 1;
        radius = 14.f;
        hp = 1 + wave / 8;
        value = 15;
        shape.setFillColor(sf::Color(90, 180, 255));
        shape.setOutlineColor(sf::Color(20, 70, 110));
    } else {
        type = 2;
        radius = 22.f;
        hp = 2 + wave / 5;
        value = 30;
        shape.setFillColor(sf::Color(180, 116, 245));
        shape.setOutlineColor(sf::Color(60, 35, 100));
    }
    shape.setRadius(radius);
    shape.setPosition(pos.x - radius, pos.y - radius);
}

void Enemy::update(float dt, const Vec2& target, const Map& map) {
    if (!active) return;
    Vec2 dir = target - pos;
    float len = dir.length();
    if (len > 0.0001f) {
        dir = dir / len;
        float base = 80.f;
        if (type == 0) base = 90.f;
        if (type == 1) base = 120.f;
        if (type == 2) base = 70.f;
        vel = dir * base;
    } else {
        vel = Vec2{0.f, 0.f};
    }

    Vec2 next = pos + vel * dt;
    sf::FloatRect test(next.x - radius, next.y - radius, radius * 2.f, radius * 2.f);
    if (!map.collides(test)) {
        pos = next;
    }
    shape.setPosition(pos.x - radius, pos.y - radius);
}

sf::FloatRect Enemy::bounds() const {
    return sf::FloatRect(pos.x - radius, pos.y - radius, radius * 2.f, radius * 2.f);
}

void Enemy::hit(int damage) {
    hp -= damage;
    if (hp <= 0) active = false;
}

void Enemy::draw(sf::RenderWindow& window) const {
    if (active) window.draw(shape);
}
