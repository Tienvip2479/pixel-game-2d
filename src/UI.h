#pragma once

#include <SFML/Graphics.hpp>
#include <string>
#include "Player.h"

class UI {
public:
    UI();
    void update(const Player& player, int wave, const std::string& status);
    void draw(sf::RenderWindow& window) const;
    sf::Font& font();

private:
    sf::Font font_;
    sf::Text hpText_;
    sf::Text ammoText_;
    sf::Text scoreText_;
    sf::Text waveText_;
    sf::Text statusText_;
    sf::Text statText_;
    sf::Text equipmentText_;
};
