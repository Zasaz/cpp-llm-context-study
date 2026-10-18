#ifndef WAREHOUSE_H
#define WAREHOUSE_H

#include "Item.h"
#include <string>
#include <vector>

class Warehouse {
public:
    std::string label;
    std::vector<Item> items;

    Warehouse();
    Warehouse(const std::string& warehouseLabel);

    void addItem(const Item& item);
    int itemCount() const;
    double inventoryValue() const;
    int findIndex(const std::string& name) const;
    bool hasItem(const std::string& name) const;
    int totalUnits() const;
    bool withdraw(const std::string& name, int amount);
};

#endif
