#include "Map.h"

#include <SFML/Graphics.hpp>

#include <vector>

Map::Map() {
    tiles_.resize(Cols * Rows, 0);
    buildMap();
    sf::Image image;
    image.create(32, 32, sf::Color(67, 79, 92));
    for (unsigned int y = 0; y < image.getSize().y; ++y) {
        for (unsigned int x = 0; x < image.getSize().x; ++x) {
            image.setPixel(x, y, sf::Color(75, 90, 109));
        }
    }
    texture_.loadFromImage(image);
    buildVertices();
}

void Map::buildMap() {
    for (int y = 0; y < Rows; ++y) {
        for (int x = 0; x < Cols; ++x) {
            tiles_[y * Cols + x] = 0;
        }
    }

    for (int x = 0; x < Cols; ++x) {
        tiles_[0 * Cols + x] = 1;
        tiles_[(Rows - 1) * Cols + x] = 1;
    }
    for (int y = 0; y < Rows; ++y) {
        tiles_[y * Cols + 0] = 1;
        tiles_[y * Cols + (Cols - 1)] = 1;
    }

    for (int x = 8; x < 15; ++x) {
        for (int y = 8; y < 12; ++y) {
            tiles_[y * Cols + x] = 1;
        }
    }
    for (int x = 20; x < 28; ++x) {
        for (int y = 7; y < 12; ++y) {
            tiles_[y * Cols + x] = 1;
        }
    }
    for (int x = 30; x < 36; ++x) {
        for (int y = 12; y < 18; ++y) {
            tiles_[y * Cols + x] = 1;
        }
    }
}

void Map::buildVertices() {
    vertices_.setPrimitiveType(sf::Quads);
    vertices_.resize(Cols * Rows * 4);
    for (int y = 0; y < Rows; ++y) {
        for (int x = 0; x < Cols; ++x) {
            int tile = tiles_[y * Cols + x];
            if (tile == 0) continue;
            int index = (y * Cols + x) * 4;
            sf::Vertex* quad = &vertices_[index];
            quad[0].position = sf::Vector2f(x * TileSize, y * TileSize);
            quad[1].position = sf::Vector2f((x + 1) * TileSize, y * TileSize);
            quad[2].position = sf::Vector2f((x + 1) * TileSize, (y + 1) * TileSize);
            quad[3].position = sf::Vector2f(x * TileSize, (y + 1) * TileSize);

            quad[0].texCoords = sf::Vector2f(0.f, 0.f);
            quad[1].texCoords = sf::Vector2f(TileSize, 0.f);
            quad[2].texCoords = sf::Vector2f(TileSize, TileSize);
            quad[3].texCoords = sf::Vector2f(0.f, TileSize);
        }
    }
}

bool Map::isWall(int x, int y) const {
    if (x < 0 || y < 0 || x >= Cols || y >= Rows) return true;
    return tiles_[y * Cols + x] != 0;
}

bool Map::collides(const sf::FloatRect& rect) const {
    int left = static_cast<int>(rect.left / TileSize);
    int right = static_cast<int>((rect.left + rect.width) / TileSize);
    int top = static_cast<int>(rect.top / TileSize);
    int bottom = static_cast<int>((rect.top + rect.height) / TileSize);
    for (int y = top; y <= bottom; ++y) {
        for (int x = left; x <= right; ++x) {
            if (isWall(x, y)) return true;
        }
    }
    return false;
}

void Map::draw(sf::RenderWindow& window, const sf::View& view) const {
    sf::RenderStates states;
    states.texture = &texture_;
    window.setView(view);
    window.draw(vertices_, states);
}
