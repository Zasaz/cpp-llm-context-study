#include "Warehouse.h"

Warehouse::Warehouse() {
    label = "unnamed";
}

Warehouse::Warehouse(const std::string& warehouseLabel) {
    label = warehouseLabel;
}

void Warehouse::addItem(const Item& item) {
    items.push_back(item);
}

int Warehouse::itemCount() const {
    return static_cast<int>(items.size());
}

double Warehouse::inventoryValue() const {
    double sum = 0.0;
    for (unsigned int i = 0; i < items.size(); ++i) {
        sum = sum + items[i].totalValue();
    }
    return sum;
}

int Warehouse::findIndex(const std::string& name) const {
    for (unsigned int i = 0; i < items.size(); ++i) {
        if (items[i].name == name) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool Warehouse::hasItem(const std::string& name) const {
    return findIndex(name) >= 0;
}

int Warehouse::totalUnits() const {
    int sum = 0;
    for (unsigned int i = 0; i < items.size(); ++i) {
        sum = sum + items[i].quantity;
    }
    return sum;
}

bool Warehouse::withdraw(const std::string& name, int amount) {
    int index = findIndex(name);
    if (index < 0) {
        return false;
    }
    return items[index].consume(amount);
}
