#include "UI.h"

#include <sstream>

UI::UI() {
    if (!font_.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        // fallback does nothing; not critical
    }

    hpText_.setFont(font_);
    hpText_.setCharacterSize(22);
    hpText_.setFillColor(sf::Color(255, 120, 120));

    ammoText_.setFont(font_);
    ammoText_.setCharacterSize(22);
    ammoText_.setFillColor(sf::Color(120, 170, 255));

    scoreText_.setFont(font_);
    scoreText_.setCharacterSize(22);
    scoreText_.setFillColor(sf::Color(255, 220, 100));

    waveText_.setFont(font_);
    waveText_.setCharacterSize(22);
    waveText_.setFillColor(sf::Color(170, 220, 255));

    statusText_.setFont(font_);
    statusText_.setCharacterSize(24);
    statusText_.setFillColor(sf::Color(240, 240, 240));
}

void UI::update(const Player& player, int wave, const std::string& status) {
    std::stringstream ss;
    ss << "HP: " << player.hp << " / " << player.maxHp;
    hpText_.setString(ss.str());

    std::stringstream ammo;
    ammo << "Ammo: " << player.ammo << " / " << player.maxAmmo;
    ammoText_.setString(ammo.str());

    std::stringstream score;
    score << "Score: " << player.score;
    scoreText_.setString(score.str());

    std::stringstream waveStr;
    waveStr << "Wave: " << wave;
    waveText_.setString(waveStr.str());

    statusText_.setString(status);
}

void UI::draw(sf::RenderWindow& window) const {
    sf::View old = window.getView();
    window.setView(window.getDefaultView());

    hpText_.setPosition(20.f, 20.f);
    ammoText_.setPosition(20.f, 50.f);
    scoreText_.setPosition(20.f, 80.f);
    waveText_.setPosition(20.f, 110.f);
    statusText_.setPosition(340.f, 20.f);

    window.draw(hpText_);
    window.draw(ammoText_);
    window.draw(scoreText_);
    window.draw(waveText_);
    window.draw(statusText_);

    window.setView(old);
}

sf::Font& UI::font() {
    return font_;
}
