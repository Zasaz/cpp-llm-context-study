#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

// Include the header for the Report class under test
#include "Report.h"

// Helper function to compare floating-point values
bool areDoublesEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor initializes an empty report
void test_defaultConstructor_createsEmptyReport() {
    Report report;

    assert(report.isEmpty() == true);
    assert(areDoublesEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_defaultConstructor_createsEmptyReport: PASS" << std::endl;
}

// Test parameterized constructor with an empty Warehouse
void test_constructor_withEmptyWarehouse() {
    Warehouse wh;
    wh.label = "Main Warehouse";

    Report report(wh);

    assert(report.isEmpty() == true);
    assert(areDoublesEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    // When warehouse is empty, summaryLine() returns "empty" regardless of warehouse label
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_constructor_withEmptyWarehouse: PASS" << std::endl;
}

// Test averageItemValue calculation with various items
void test_averageItemValue_withItems() {
    Warehouse wh;
    wh.label = "Electronics Hub";

    // Adding items (Assuming standard Item construction or field setting available via Warehouse)
    // Adding 3 items:
    // Item 1: price 10.0, quantity 2 (value = 20.0)
    // Item 2: price 30.0, quantity 1 (value = 30.0)
    // Item 3: price 5.0,  quantity 4 (value = 20.0)
    // Total count = 3 items, Total inventory value = 70.0 -> Average = 70.0 / 3 ≈ 23.333333
    Item item1;
    item1.price = 10.0;
    item1.quantity = 2;

    Item item2;
    item2.price = 30.0;
    item2.quantity = 1;

    Item item3;
    item3.price = 5.0;
    item3.quantity = 4;

    wh.items.push_back(item1);
    wh.items.push_back(item2);
    wh.items.push_back(item3);

    Report report(wh);

    assert(!report.isEmpty());
    double expectedAverage = wh.inventoryValue() / static_cast<double>(wh.itemCount());
    assert(areDoublesEqual(report.averageItemValue(), expectedAverage));

    std::cout << "Test test_averageItemValue_withItems: PASS" << std::endl;
}

// Test outOfStockCount with mixed in-stock and out-of-stock items
void test_outOfStockCount_mixedStockItems() {
    Warehouse wh;
    wh.label = "Groceries";

    Item inStock1;
    inStock1.quantity = 5;

    Item inStock2;
    inStock2.quantity = 1;

    Item outOfStock1;
    outOfStock1.quantity = 0;

    Item outOfStock2;
    outOfStock2.quantity = 0;

    wh.items.push_back(inStock1);
    wh.items.push_back(outOfStock1);
    wh.items.push_back(inStock2);
    wh.items.push_back(outOfStock2);

    Report report(wh);

    assert(report.outOfStockCount() == 2);

    std::cout << "Test test_outOfStockCount_mixedStockItems: PASS" << std::endl;
}

// Test outOfStockCount when all items are in stock
void test_outOfStockCount_allInStock() {
    Warehouse wh;
    wh.label = "Fully Stocked";

    Item item1;
    item1.quantity = 10;

    Item item2;
    item2.quantity = 3;

    wh.items.push_back(item1);
    wh.items.push_back(item2);

    Report report(wh);

    assert(report.outOfStockCount() == 0);

    std::cout << "Test test_outOfStockCount_allInStock: PASS" << std::endl;
}

// Test summaryLine when warehouse has items
void test_summaryLine_nonEmptyWarehouse() {
    Warehouse wh;
    wh.label = "Regional Distribution Center 42";

    Item item;
    item.quantity = 10;
    item.price = 15.50;

    wh.items.push_back(item);

    Report report(wh);

    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "Regional Distribution Center 42");

    std::cout << "Test test_summaryLine_nonEmptyWarehouse: PASS" << std::endl;
}

// Entry point for the unit tests
int main() {
    std::cout << "Starting Report class unit tests..." << std::endl;

    test_defaultConstructor_createsEmptyReport();
    test_constructor_withEmptyWarehouse();
    test_averageItemValue_withItems();
    test_outOfStockCount_mixedStockItems();
    test_outOfStockCount_allInStock();
    test_summaryLine_nonEmptyWarehouse();

    std::cout << "All Report unit tests passed successfully!" << std::endl;
    return 0;
}