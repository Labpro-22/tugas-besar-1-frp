#include "../../include/models/Card.hpp"

Card::Card(const std::string& description)
    : description(description) {}

InventoryCardState Card::getInventoryState() const {
    return InventoryCardState::NONE;
}

std::string Card::getDescription() const {
    return description;
}
