#include "Item.h"

Item::Item() {
    name = "";
    quantity = 0;
    unitPrice = 0.0;
}

Item::Item(const std::string& itemName, int itemQuantity, double price) {
    name = itemName;
    quantity = itemQuantity;
    unitPrice = price;
}

double Item::totalValue() const {
    return static_cast<double>(quantity) * unitPrice;
}

bool Item::isInStock() const {
    return quantity > 0;
}

void Item::restock(int amount) {
    if (amount > 0) {
        quantity = quantity + amount;
    }
}

bool Item::consume(int amount) {
    if (amount <= 0 || amount > quantity) {
        return false;
    }
    quantity = quantity - amount;
    return true;
}
