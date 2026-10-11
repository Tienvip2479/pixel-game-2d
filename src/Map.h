#pragma once

#include <SFML/Graphics.hpp>
#include <cmath>
#include <vector>

struct Vec2 {
    float x = 0.f;
    float y = 0.f;

    Vec2() = default;
    Vec2(float x_, float y_) : x(x_), y(y_) {}

    float length() const { return std::sqrt(x * x + y * y); }
    Vec2 normalized() const {
        float len = length();
        if (len < 0.0001f) return Vec2{0.f, 0.f};
        return Vec2{x / len, y / len};
    }

    Vec2 operator+(const Vec2& other) const { return Vec2{x + other.x, y + other.y}; }
    Vec2 operator-(const Vec2& other) const { return Vec2{x - other.x, y - other.y}; }
    Vec2 operator*(float value) const { return Vec2{x * value, y * value}; }
    Vec2 operator/(float value) const { return Vec2{x / value, y / value}; }
    Vec2& operator+=(const Vec2& other) { x += other.x; y += other.y; return *this; }
    Vec2& operator-=(const Vec2& other) { x -= other.x; y -= other.y; return *this; }
};

class Map {
public:
    static constexpr int TileSize = 32;
    static constexpr int Cols = 40;
    static constexpr int Rows = 22;

    Map();
    bool isWall(int x, int y) const;
    bool collides(const sf::FloatRect& rect) const;
    void draw(sf::RenderWindow& window, const sf::View& view) const;

private:
    std::vector<int> tiles_;
    sf::Texture texture_;
    sf::VertexArray vertices_;
    void buildMap();
    void buildVertices();
};
