#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function for floating point comparisons
bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_defaultConstructor() {
    Report report;

    assert(report.isEmpty() == true);
    assert(areClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_emptyWarehouse() {
    Warehouse warehouse;
    warehouse.label = "Main Warehouse";

    Report report(warehouse);

    assert(report.isEmpty() == true);
    assert(areClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_emptyWarehouse: PASS" << std::endl;
}

void test_warehouseWithAllInStockItems() {
    Warehouse warehouse;
    warehouse.label = "North Hub";

    Item item1;
    item1.price = 10.0;
    item1.quantity = 2;

    Item item2;
    item2.price = 20.0;
    item2.quantity = 3;

    warehouse.items.push_back(item1);
    warehouse.items.push_back(item2);

    Report report(warehouse);

    assert(report.isEmpty() == false);
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "North Hub");
    // itemCount is total units: 2 + 3 = 5
    // inventoryValue is: 10*2 + 20*3 = 20 + 60 = 80
    // averageItemValue is: 80 / 5 = 16.0
    assert(areClose(report.averageItemValue(), warehouse.inventoryValue() / static_cast<double>(warehouse.itemCount())));

    std::cout << "Test test_warehouseWithAllInStockItems: PASS" << std::endl;
}

void test_warehouseWithOutOfStockItems() {
    Warehouse warehouse;
    warehouse.label = "South Depot";

    Item item1;
    item1.price = 15.0;
    item1.quantity = 0; // Out of stock

    Item item2;
    item2.price = 25.0;
    item2.quantity = 4; // In stock

    Item item3;
    item3.price = 5.0;
    item3.quantity = 0; // Out of stock

    warehouse.items.push_back(item1);
    warehouse.items.push_back(item2);
    warehouse.items.push_back(item3);

    Report report(warehouse);

    assert(report.isEmpty() == false);
    assert(report.outOfStockCount() == 2);
    assert(report.summaryLine() == "South Depot");
    assert(areClose(report.averageItemValue(), warehouse.inventoryValue() / static_cast<double>(warehouse.itemCount())));

    std::cout << "Test test_warehouseWithOutOfStockItems: PASS" << std::endl;
}

void test_warehouseWithAllOutOfStockItems() {
    Warehouse warehouse;
    warehouse.label = "Inactive Storage";

    Item item1;
    item1.price = 50.0;
    item1.quantity = 0;

    Item item2;
    item2.price = 100.0;
    item2.quantity = 0;

    warehouse.items.push_back(item1);
    warehouse.items.push_back(item2);

    Report report(warehouse);

    // If all items have quantity 0, warehouse.itemCount() is 0, so isEmpty() is true
    assert(report.isEmpty() == true);
    assert(report.outOfStockCount() == 2);
    assert(areClose(report.averageItemValue(), 0.0));
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_warehouseWithAllOutOfStockItems: PASS" << std::endl;
}

void test_summaryLineWithCustomLabels() {
    Warehouse warehouse1;
    warehouse1.label = "Custom Warehouse 123";

    Item item;
    item.price = 12.5;
    item.quantity = 1;
    warehouse1.items.push_back(item);

    Report report(warehouse1);

    assert(report.summaryLine() == "Custom Warehouse 123");

    std::cout << "Test test_summaryLineWithCustomLabels: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Report test suite..." << std::endl;

    test_defaultConstructor();
    test_emptyWarehouse();
    test_warehouseWithAllInStockItems();
    test_warehouseWithOutOfStockItems();
    test_warehouseWithAllOutOfStockItems();
    test_summaryLineWithCustomLabels();

    std::cout << "All Report tests passed successfully!" << std::endl;
    return 0;
}