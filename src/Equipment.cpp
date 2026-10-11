#include "Equipment.h"

EquipmentSystem::EquipmentSystem() {
    defaultGear_[0] = {"helmet_iron", "Iron Cap", "Basic helmet, improves defense.", EquipmentSlot::Helmet, sf::Color(180, 180, 190), {8, 0, 5, 0, 0, 0}};
    defaultGear_[1] = {"armor_steel", "Steel Mail", "A solid chestpiece that reduces damage.", EquipmentSlot::Armor, sf::Color(155, 155, 180), {12, 0, 10, 0, 0, 0}};
    defaultGear_[2] = {"shield_wooden", "Wooden Shield", "Adds block and sturdiness.", EquipmentSlot::Shield, sf::Color(170, 120, 90), {5, 0, 8, 0, 0, 0}};
    defaultGear_[3] = {"boots_runner", "Runner Boots", "Improves speed and dodge.", EquipmentSlot::Boots, sf::Color(120, 200, 180), {0, 0, 0, 18, 0, 4}};
    resetToDefaults();
}

void EquipmentSystem::resetToDefaults() {
    for (std::size_t i = 0; i < equipped_.size(); ++i) {
        equipped_[i] = &defaultGear_[i];
    }
}

void EquipmentSystem::equip(const EquipmentItem& item) {
    for (std::size_t i = 0; i < defaultGear_.size(); ++i) {
        if (defaultGear_[i].slot == item.slot) {
            equipped_[i] = &defaultGear_[i];
            return;
        }
    }
}

void EquipmentSystem::unequip(EquipmentSlot slot) {
    for (std::size_t i = 0; i < defaultGear_.size(); ++i) {
        if (defaultGear_[i].slot == slot) {
            equipped_[i] = &defaultGear_[i];
            return;
        }
    }
}

const EquipmentItem* EquipmentSystem::get(EquipmentSlot slot) const {
    for (std::size_t i = 0; i < defaultGear_.size(); ++i) {
        if (defaultGear_[i].slot == slot) {
            return equipped_[i];
        }
    }
    return nullptr;
}

CharacterStats EquipmentSystem::computeStats(const CharacterStats& base) const {
    CharacterStats result = base;
    for (const auto* item : equipped_) {
        if (!item) continue;
        result.maxHp += item->stats.hp;
        result.attack += item->stats.attack;
        result.defense += item->stats.defense;
        result.speed += item->stats.speed;
        result.crit += item->stats.crit;
        result.dodge += item->stats.dodge;
    }
    return result;
}
