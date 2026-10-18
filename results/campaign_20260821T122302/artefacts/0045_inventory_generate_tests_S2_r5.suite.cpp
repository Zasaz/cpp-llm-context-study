#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function for floating point comparisons
bool doubleEquals(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// Test default constructor behavior
void test_default_constructor() {
    Report report;
    
    assert(report.isEmpty() == true);
    assert(doubleEquals(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_default_constructor: PASS" << std::endl;
}

// Test parameterized constructor with an empty warehouse
void test_empty_warehouse() {
    Warehouse warehouse;
    warehouse.label = "Main Warehouse";

    Report report(warehouse);

    assert(report.isEmpty() == true);
    assert(doubleEquals(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_empty_warehouse: PASS" << std::endl;
}

// Test warehouse with items that are all in stock
void test_warehouse_all_in_stock() {
    Warehouse warehouse;
    warehouse.label = "North Hub";

    Item item1("Widget A", 10.0, 5);  // value: 50.0
    Item item2("Widget B", 20.0, 2);  // value: 40.0
    warehouse.addItem(item1);
    warehouse.addItem(item2);

    Report report(warehouse);

    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "North Hub");
    assert(report.outOfStockCount() == 0);

    // Total count = 2, total value = 90.0, average = 45.0
    assert(doubleEquals(report.averageItemValue(), 45.0));

    std::cout << "Test test_warehouse_all_in_stock: PASS" << std::endl;
}

// Test warehouse with items where some or all are out of stock
void test_warehouse_out_of_stock() {
    Warehouse warehouse;
    warehouse.label = "South Hub";

    Item item1("InStock", 15.0, 3);
    Item item2("OutOfStock1", 25.0, 0);
    Item item3("OutOfStock2", 5.0, 0);

    warehouse.addItem(item1);
    warehouse.addItem(item2);
    warehouse.addItem(item3);

    Report report(warehouse);

    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "South Hub");
    assert(report.outOfStockCount() == 2);

    // Total count = 3, total value = 45.0, average = 15.0
    assert(doubleEquals(report.averageItemValue(), 15.0));

    std::cout << "Test test_warehouse_out_of_stock: PASS" << std::endl;
}

// Test averageItemValue calculation precision with varying prices and quantities
void test_average_item_value_precision() {
    Warehouse warehouse;
    warehouse.label = "Precision Hub";

    Item item1("Item1", 3.33, 3); // value: 9.99
    Item item2("Item2", 7.77, 1); // value: 7.77
    warehouse.addItem(item1);
    warehouse.addItem(item2);

    Report report(warehouse);

    double expectedAverage = (9.99 + 7.77) / 2.0;
    assert(doubleEquals(report.averageItemValue(), expectedAverage));

    std::cout << "Test test_average_item_value_precision: PASS" << std::endl;
}

// Test summaryLine returns exact label for non-empty warehouse
void test_summary_line_non_empty() {
    Warehouse warehouse;
    warehouse.label = "Central Depot #42";

    Item item("Gadget", 1.0, 1);
    warehouse.addItem(item);

    Report report(warehouse);

    assert(report.summaryLine() == "Central Depot #42");

    std::cout << "Test test_summary_line_non_empty: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Report test suite..." << std::endl;

    test_default_constructor();
    test_empty_warehouse();
    test_warehouse_all_in_stock();
    test_warehouse_out_of_stock();
    test_average_item_value_precision();
    test_summary_line_non_empty();

    std::cout << "All Report tests passed successfully!" << std::endl;
    return 0;
}