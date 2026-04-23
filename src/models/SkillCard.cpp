#include "../../include/models/SkillCard.hpp"

SkillCard::SkillCard(const std::string& description, int value, int duration)
    : Card(description), value(value), duration(duration) {}

bool SkillCard::canUseBeforeRoll() const {
    return true;
}

InventoryCardState SkillCard::getInventoryState() const {
    return InventoryCardState::SKILL;
}

int SkillCard::getValue() const {
    return value;
}

int SkillCard::getDuration() const {
    return duration;
}

void SkillCard::setValue(int newValue) {
    value = newValue;
}

void SkillCard::setDuration(int newDuration) {
    duration = newDuration;
}
