#include "UI.h"

#include <sstream>

UI::UI() {
    if (!font_.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        // fallback intentionally silent
    }

    hpText_.setFont(font_); hpText_.setCharacterSize(22); hpText_.setFillColor(sf::Color(255, 120, 120));
    ammoText_.setFont(font_); ammoText_.setCharacterSize(22); ammoText_.setFillColor(sf::Color(120, 170, 255));
    scoreText_.setFont(font_); scoreText_.setCharacterSize(22); scoreText_.setFillColor(sf::Color(255, 220, 100));
    waveText_.setFont(font_); waveText_.setCharacterSize(22); waveText_.setFillColor(sf::Color(170, 220, 255));
    statusText_.setFont(font_); statusText_.setCharacterSize(24); statusText_.setFillColor(sf::Color(240, 240, 240));
    statText_.setFont(font_); statText_.setCharacterSize(18); statText_.setFillColor(sf::Color(180, 220, 255));
    equipmentText_.setFont(font_); equipmentText_.setCharacterSize(18); equipmentText_.setFillColor(sf::Color(255, 210, 120));
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

    std::stringstream stats;
    stats << "ATK " << player.effectiveStats.attack << " | DEF " << player.effectiveStats.defense
          << " | SPD " << static_cast<int>(player.effectiveStats.speed)
          << " | CRIT " << player.effectiveStats.crit << " | DODGE " << player.effectiveStats.dodge;
    statText_.setString(stats.str());

    std::stringstream eq;
    eq << "Helmet: " << (player.equipment.get(EquipmentSlot::Helmet) ? player.equipment.get(EquipmentSlot::Helmet)->name : "Empty")
       << " | Armor: " << (player.equipment.get(EquipmentSlot::Armor) ? player.equipment.get(EquipmentSlot::Armor)->name : "Empty")
       << " | Shield: " << (player.equipment.get(EquipmentSlot::Shield) ? player.equipment.get(EquipmentSlot::Shield)->name : "Empty")
       << " | Boots: " << (player.equipment.get(EquipmentSlot::Boots) ? player.equipment.get(EquipmentSlot::Boots)->name : "Empty");
    equipmentText_.setString(eq.str());

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
    statText_.setPosition(20.f, 150.f);
    equipmentText_.setPosition(20.f, 182.f);

    window.draw(hpText_);
    window.draw(ammoText_);
    window.draw(scoreText_);
    window.draw(waveText_);
    window.draw(statusText_);
    window.draw(statText_);
    window.draw(equipmentText_);

    window.setView(old);
}

sf::Font& UI::font() {
    return font_;
}
