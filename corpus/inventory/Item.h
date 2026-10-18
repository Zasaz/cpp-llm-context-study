#ifndef ITEM_H
#define ITEM_H

#include <string>

class Item {
public:
    std::string name;
    int quantity;
    double unitPrice;

    Item();
    Item(const std::string& itemName, int itemQuantity, double price);

    double totalValue() const;
    bool isInStock() const;
    void restock(int amount);
    bool consume(int amount);
};

#endif
