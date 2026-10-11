#pragma once

#include <SFML/Graphics.hpp>
#include <array>
#include <string>

enum class EquipmentSlot {
    Helmet,
    Armor,
    Shield,
    Boots
};

struct StatBlock {
    int hp = 0;
    int attack = 0;
    int defense = 0;
    int speed = 0;
    int crit = 0;
    int dodge = 0;
};

struct EquipmentItem {
    std::string id;
    std::string name;
    std::string description;
    EquipmentSlot slot = EquipmentSlot::Helmet;
    sf::Color tint = sf::Color::White;
    StatBlock stats;
};

struct CharacterStats {
    int maxHp = 5;
    int attack = 10;
    int defense = 5;
    float speed = 220.f;
    int crit = 5;
    int dodge = 5;
};

class EquipmentSystem {
public:
    EquipmentSystem();
    void resetToDefaults();
    void equip(const EquipmentItem& item);
    void unequip(EquipmentSlot slot);
    const EquipmentItem* get(EquipmentSlot slot) const;
    CharacterStats computeStats(const CharacterStats& base) const;

private:
    std::array<EquipmentItem, 4> defaultGear_;
    std::array<const EquipmentItem*, 4> equipped_;
};
