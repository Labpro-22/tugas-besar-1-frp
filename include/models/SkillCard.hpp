#pragma once

#include "Card.hpp"

class SkillCard : public Card {
protected:
    int value;
    int duration;

public:
    SkillCard(const std::string& description, int value = 0, int duration = 0);
    ~SkillCard() override = default;

    bool canUseBeforeRoll() const;
    InventoryCardState getInventoryState() const override;

    int getValue() const;
    int getDuration() const;

    void setValue(int newValue);
    void setDuration(int newDuration);

    virtual std::string getTypeName() const = 0;
};
