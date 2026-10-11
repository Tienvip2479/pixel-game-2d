#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include <vector>

#include "Equipment.h"

class Player;

class EquipmentPanel {
public:
    EquipmentPanel();
    void update(const Player& player);
    void draw(sf::RenderWindow& window) const;

private:
    sf::Font font_;
    sf::RectangleShape panel_;
    sf::Text title_;
    sf::Text lines_[8];
};
