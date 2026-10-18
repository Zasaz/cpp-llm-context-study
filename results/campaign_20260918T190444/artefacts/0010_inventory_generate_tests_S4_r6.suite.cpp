#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"

// Helper function for floating point comparison
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_defaultConstructor() {
    Report report;
    assert(report.isEmpty() == true);
    assert(areAlmostEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");
    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_paramConstructor_emptyWarehouse() {
    Warehouse warehouse;
    warehouse.label = "Main Hub";
    
    Report report(warehouse);
    assert(report.isEmpty() == true);
    assert(areAlmostEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    // When warehouse has 0 items, summaryLine should return "empty" regardless of label
    assert(report.summaryLine() == "empty");
    std::cout << "Test test_paramConstructor_emptyWarehouse: PASS" << std::endl;
}

void test_averageItemValue_withItems() {
    Warehouse warehouse;
    warehouse.label = "Electronics Dept";

    // Assuming standard Item / Warehouse setup where items can be added
    // If Item has fields like quantity, price or isInStock methods, we populate via available Warehouse API
    // Let's create items and add them to the warehouse
    Item item1;
    item1.price = 10.0;
    item1.quantity = 2; // Value: 20.0
    warehouse.items.push_back(item1);

    Item item2;
    item2.price = 30.0;
    item2.quantity = 1; // Value: 30.0
    warehouse.items.push_back(item2);

    Report report(warehouse);

    assert(report.isEmpty() == false);
    // itemCount() == 2 items in collection (or sum of quantities depending on Warehouse implementation)
    // inventoryValue() == 20.0 + 30.0 = 50.0
    // averageItemValue() returns inventoryValue() / itemCount()
    double expectedAvg = warehouse.inventoryValue() / static_cast<double>(warehouse.itemCount());
    assert(areAlmostEqual(report.averageItemValue(), expectedAvg));

    std::cout << "Test test_averageItemValue_withItems: PASS" << std::endl;
}

void test_outOfStockCount() {
    Warehouse warehouse;
    
    Item itemInStock;
    itemInStock.quantity = 5;
    
    Item itemOutOfStock1;
    itemOutOfStock1.quantity = 0;

    Item itemOutOfStock2;
    itemOutOfStock2.quantity = 0;

    warehouse.items.push_back(itemInStock);
    warehouse.items.push_back(itemOutOfStock1);
    warehouse.items.push_back(itemOutOfStock2);

    Report report(warehouse);

    assert(report.outOfStockCount() == 2);
    std::cout << "Test test_outOfStockCount: PASS" << std::endl;
}

void test_summaryLine_nonEmptyWarehouse() {
    Warehouse warehouse;
    warehouse.label = "North Sector Logistics";

    Item item;
    item.quantity = 10;
    item.price = 5.0;
    warehouse.items.push_back(item);

    Report report(warehouse);

    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "North Sector Logistics");
    std::cout << "Test test_summaryLine_nonEmptyWarehouse: PASS" << std::endl;
}

void test_directSourceModification() {
    Report report;
    assert(report.isEmpty() == true);

    // Modify the public member 'source' directly
    report.source.label = "Storage Yard B";
    Item item;
    item.quantity = 4;
    item.price = 25.0;
    report.source.items.push_back(item);

    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "Storage Yard B");
    assert(report.outOfStockCount() == 0);
    assert(areAlmostEqual(report.averageItemValue(), report.source.inventoryValue() / static_cast<double>(report.source.itemCount())));

    std::cout << "Test test_directSourceModification: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Report test suite..." << std::endl;

    test_defaultConstructor();
    test_paramConstructor_emptyWarehouse();
    test_averageItemValue_withItems();
    test_outOfStockCount();
    test_summaryLine_nonEmptyWarehouse();
    test_directSourceModification();

    std::cout << "All tests passed successfully!" << std::endl;
    return 0;
}