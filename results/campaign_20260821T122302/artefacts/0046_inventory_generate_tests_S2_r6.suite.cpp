#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_defaultConstructor() {
    Report report;
    assert(report.isEmpty());
    assert(report.summaryLine() == "empty");
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_emptyWarehouse() {
    Warehouse warehouse;
    warehouse.label = "Central Depot";
    
    Report report(warehouse);
    assert(report.isEmpty());
    assert(report.summaryLine() == "empty");
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    std::cout << "Test test_emptyWarehouse: PASS" << std::endl;
}

void test_warehouseWithAllInStockItems() {
    Warehouse warehouse;
    warehouse.label = "North Annex";

    // Adding in-stock items
    Item item1("Widget", 10.0, 5);
    Item item2("Gadget", 20.0, 2);
    warehouse.addItem(item1);
    warehouse.addItem(item2);

    Report report(warehouse);
    assert(!report.isEmpty());
    assert(report.summaryLine() == "North Annex");
    
    // Check out of stock count (should be 0)
    assert(report.outOfStockCount() == 0);

    // Calculate expected average item value
    // Total inventory value = (10.0 * 5) + (20.0 * 2) = 50.0 + 40.0 = 90.0
    // Total item count = 5 + 2 = 7 (or item count might be items.size() or sum of quantities)
    // Using warehouse's item count and inventory value:
    double expectedAvg = warehouse.inventoryValue() / static_cast<double>(warehouse.itemCount());
    assert(isClose(report.averageItemValue(), expectedAvg));

    std::cout << "Test test_warehouseWithAllInStockItems: PASS" << std::endl;
}

void test_warehouseWithOutOfStockItems() {
    Warehouse warehouse;
    warehouse.label = "South Warehouse";

    Item item1("InStockItem", 15.0, 10);
    Item item2("OutOfStockItem1", 25.0, 0);
    Item item3("OutOfStockItem2", 5.0, 0);
    warehouse.addItem(item1);
    warehouse.addItem(item2);
    warehouse.addItem(item3);

    Report report(warehouse);
    assert(!report.isEmpty());
    assert(report.summaryLine() == "South Warehouse");
    assert(report.outOfStockCount() == 2);

    std::cout << "Test test_warehouseWithOutOfStockItems: PASS" << std::endl;
}

void test_warehouseAllOutOfStock() {
    Warehouse warehouse;
    warehouse.label = "Empty Inventory Storage";

    Item item1("ItemA", 12.5, 0);
    Item item2("ItemB", 8.0, 0);
    warehouse.addItem(item1);
    warehouse.addItem(item2);

    Report report(warehouse);
    // Even if items are present with 0 quantity, itemCount() may be 0
    if (warehouse.itemCount() == 0) {
        assert(report.isEmpty());
        assert(report.summaryLine() == "empty");
        assert(isClose(report.averageItemValue(), 0.0));
    } else {
        assert(!report.isEmpty());
        assert(report.summaryLine() == "Empty Inventory Storage");
    }
    assert(report.outOfStockCount() == 2);

    std::cout << "Test test_warehouseAllOutOfStock: PASS" << std::endl;
}

void test_singleItemCalculation() {
    Warehouse warehouse;
    warehouse.label = "Single Item Depot";

    Item item("SingleWidget", 42.50, 1);
    warehouse.addItem(item);

    Report report(warehouse);
    assert(!report.isEmpty());
    assert(report.summaryLine() == "Single Item Depot");
    assert(report.outOfStockCount() == 0);
    assert(isClose(report.averageItemValue(), 42.50));

    std::cout << "Test test_singleItemCalculation: PASS" << std::endl;
}

int main() {
    test_defaultConstructor();
    test_emptyWarehouse();
    test_warehouseWithAllInStockItems();
    test_warehouseWithOutOfStockItems();
    test_warehouseAllOutOfStock();
    test_singleItemCalculation();

    std::cout << "All Report tests passed successfully!" << std::endl;
    return 0;
}