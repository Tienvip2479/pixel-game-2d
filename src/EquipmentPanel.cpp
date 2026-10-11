#include "EquipmentPanel.h"

#include <sstream>

#include "Player.h"

EquipmentPanel::EquipmentPanel() {
    if (!font_.loadFromFile("C:/Windows/Fonts/arial.ttf")) {
        // fallback intentionally silent
    }

    panel_.setSize(sf::Vector2f(300.f, 180.f));
    panel_.setFillColor(sf::Color(26, 30, 42, 220));
    panel_.setOutlineColor(sf::Color(160, 180, 210));
    panel_.setOutlineThickness(2.f);
    panel_.setPosition(930.f, 180.f);

    title_.setFont(font_);
    title_.setCharacterSize(22);
    title_.setFillColor(sf::Color(255, 220, 100));
    title_.setString("Equipment");
    title_.setPosition(950.f, 195.f);

    for (int i = 0; i < 8; ++i) {
        lines_[i].setFont(font_);
        lines_[i].setCharacterSize(16);
        lines_[i].setFillColor(sf::Color(230, 230, 230));
        lines_[i].setPosition(950.f, 228.f + i * 18.f);
    }
}

void EquipmentPanel::update(const Player& player) {
    const auto* helmet = player.equipment.get(EquipmentSlot::Helmet);
    const auto* armor = player.equipment.get(EquipmentSlot::Armor);
    const auto* shield = player.equipment.get(EquipmentSlot::Shield);
    const auto* boots = player.equipment.get(EquipmentSlot::Boots);

    std::array<std::string, 8> texts = {
        "Helmet: " + (helmet ? helmet->name : "Empty"),
        "Armor: " + (armor ? armor->name : "Empty"),
        "Shield: " + (shield ? shield->name : "Empty"),
        "Boots: " + (boots ? boots->name : "Empty"),
        "HP: " + std::to_string(player.effectiveStats.maxHp),
        "ATK: " + std::to_string(player.effectiveStats.attack),
        "DEF: " + std::to_string(player.effectiveStats.defense),
        "SPD: " + std::to_string(static_cast<int>(player.effectiveStats.speed))
    };

    for (int i = 0; i < 8; ++i) {
        lines_[i].setString(texts[i]);
    }
}

void EquipmentPanel::draw(sf::RenderWindow& window) const {
    window.draw(panel_);
    window.draw(title_);
    for (int i = 0; i < 8; ++i) {
        window.draw(lines_[i]);
    }
}
