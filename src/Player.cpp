#include "Player.h"

#include <algorithm>
#include <cmath>

#include "Map.h"

Player::Player() {
    pos = Vec2{200.f, 200.f};
    shape.setRadius(radius);
    shape.setFillColor(sf::Color(110, 220, 180));
    shape.setOutlineColor(sf::Color(16, 80, 70));
    shape.setOutlineThickness(2.f);
    shape.setPosition(pos.x - radius, pos.y - radius);
}

void Player::update(float dt, bool left, bool right, bool up, bool down,
                    const sf::Vector2f& mouseWorld, const Map& map) {
    Vec2 dir{0.f, 0.f};
    if (left) dir.x -= 1.f;
    if (right) dir.x += 1.f;
    if (up) dir.y -= 1.f;
    if (down) dir.y += 1.f;

    if (dir.x != 0.f || dir.y != 0.f) {
        float length = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        dir.x /= length;
        dir.y /= length;
        pos.x += dir.x * 220.f * dt;
        pos.y += dir.y * 220.f * dt;
    }

    pos.x = std::max(radius, std::min(pos.x, static_cast<float>(Map::Cols * Map::TileSize) - radius));
    pos.y = std::max(radius, std::min(pos.y, static_cast<float>(Map::Rows * Map::TileSize) - radius));
    shape.setPosition(pos.x - radius, pos.y - radius);

    float dx = mouseWorld.x - pos.x;
    float dy = mouseWorld.y - pos.y;
    float len = std::sqrt(dx * dx + dy * dy);
    if (len > 0.0001f) {
        aim = Vec2{dx / len, dy / len};
    }

    shootCooldown = std::max(0.f, shootCooldown - dt);
    (void)map;
}

void Player::draw(sf::RenderWindow& window) const {
    window.draw(shape);
}

sf::FloatRect Player::bounds() const {
    return sf::FloatRect(pos.x - radius, pos.y - radius, radius * 2.f, radius * 2.f);
}
