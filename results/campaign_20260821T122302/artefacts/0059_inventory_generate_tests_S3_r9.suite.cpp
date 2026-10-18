#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

#include "Item.h"
#include "Warehouse.h"
#include "Report.h"

// Helper function for floating point comparisons
static bool areDoublesEqual(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// Test default constructor behavior
void test_default_constructor() {
    Report report;

    assert(report.isEmpty() == true);
    assert(report.outOfStockCount() == 0);
    assert(areDoublesEqual(report.averageItemValue(), 0.0));
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_default_constructor: PASS" << std::endl;
}

// Test parameterized constructor with an empty warehouse
void test_parameterized_constructor_empty() {
    Warehouse warehouse("North Hub");
    Report report(warehouse);

    assert(report.isEmpty() == true);
    assert(report.outOfStockCount() == 0);
    assert(areDoublesEqual(report.averageItemValue(), 0.0));
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_parameterized_constructor_empty: PASS" << std::endl;
}

// Test summaryLine and isEmpty with non-empty warehouses
void test_summaryLine_and_isEmpty_non_empty() {
    Warehouse warehouse("Central Depot");
    warehouse.addItem(Item("Screws", 100, 0.05));

    Report report(warehouse);

    assert(!report.isEmpty());
    assert(report.summaryLine() == "Central Depot");

    std::cout << "Test test_summaryLine_and_isEmpty_non_empty: PASS" << std::endl;
}

// Test averageItemValue calculation across various item combinations
void test_averageItemValue_calculations() {
    Warehouse warehouse("Electronics Store");
    
    // Single item
    warehouse.addItem(Item("Laptop", 5, 800.0)); // totalValue = 4000.0
    Report report1(warehouse);
    assert(areDoublesEqual(report1.averageItemValue(), 4000.0));

    // Multiple items
    warehouse.addItem(Item("Mouse", 20, 25.0));   // totalValue = 500.0
    warehouse.addItem(Item("Keyboard", 10, 50.0)); // totalValue = 500.0
    // Total inventory value = 5000.0, item types count = 3
    // averageItemValue = 5000.0 / 3 = 1666.666667
    Report report2(warehouse);
    assert(areDoublesEqual(report2.averageItemValue(), 5000.0 / 3.0));

    // Adding an item with 0 quantity (value = 0.0)
    warehouse.addItem(Item("Monitor", 0, 300.0));  // totalValue = 0.0
    // Total value = 5000.0, item count = 4 -> average = 1250.0
    Report report3(warehouse);
    assert(areDoublesEqual(report3.averageItemValue(), 1250.0));

    std::cout << "Test test_averageItemValue_calculations: PASS" << std::endl;
}

// Test outOfStockCount with various stock quantities
void test_outOfStockCount() {
    Warehouse warehouse("Grocery");

    // All items in stock
    warehouse.addItem(Item("Apples", 50, 0.5));
    warehouse.addItem(Item("Bananas", 30, 0.3));
    warehouse.addItem(Item("Oranges", 20, 0.8));

    Report reportInStock(warehouse);
    assert(reportInStock.outOfStockCount() == 0);

    // Add out-of-stock items (quantity == 0)
    warehouse.addItem(Item("Milk", 0, 2.5));
    warehouse.addItem(Item("Bread", 0, 1.5));

    Report reportMixed(warehouse);
    assert(reportMixed.outOfStockCount() == 2);

    // Warehouse with only out-of-stock items
    Warehouse emptyStockWarehouse("Defunct Depot");
    emptyStockWarehouse.addItem(Item("ItemA", 0, 10.0));
    emptyStockWarehouse.addItem(Item("ItemB", 0, 20.0));

    Report reportAllOut(emptyStockWarehouse);
    assert(reportAllOut.outOfStockCount() == 2);

    std::cout << "Test test_outOfStockCount: PASS" << std::endl;
}

// Test interactions when modifying the underlying warehouse
void test_state_changes() {
    Warehouse warehouse("Hardware Store");
    warehouse.addItem(Item("Drill", 2, 99.99)); // total = 199.98
    warehouse.addItem(Item("Saw", 1, 49.99));   // total = 49.99

    Report report(warehouse);
    assert(report.outOfStockCount() == 0);
    assert(areDoublesEqual(report.averageItemValue(), (199.98 + 49.99) / 2.0));

    // Consume all drills directly via report's source
    bool withdrawn = report.source.withdraw("Drill", 2);
    assert(withdrawn);
    assert(report.outOfStockCount() == 1);
    assert(areDoublesEqual(report.averageItemValue(), 49.99 / 2.0));

    std::cout << "Test test_state_changes: PASS" << std::endl;
}

int main() {
    std::cout << "Running Report test suite..." << std::endl;

    test_default_constructor();
    test_parameterized_constructor_empty();
    test_summaryLine_and_isEmpty_non_empty();
    test_averageItemValue_calculations();
    test_outOfStockCount();
    test_state_changes();

    std::cout << "All Report tests passed successfully!" << std::endl;
    return 0;
}