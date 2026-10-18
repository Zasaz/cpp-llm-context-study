#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Item.h"
#include "Warehouse.h"
#include "Report.h"

// Helper function for floating point comparisons
static bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// Test default construction of Report
void test_defaultConstructor() {
    Report report;

    assert(report.isEmpty() == true);
    assert(areClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");
    assert(report.source.label == "unnamed");

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

// Test Report constructed from an empty warehouse with a custom label
void test_parametrizedConstructor_emptyWarehouse() {
    Warehouse warehouse("North Hub");
    Report report(warehouse);

    assert(report.isEmpty() == true);
    assert(areClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    // An empty warehouse report should return "empty" regardless of label
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_parametrizedConstructor_emptyWarehouse: PASS" << std::endl;
}

// Test averageItemValue with non-empty warehouses
void test_averageItemValue_populatedWarehouse() {
    Warehouse warehouse("Electronics");
    warehouse.addItem(Item("Monitor", 2, 200.0)); // totalValue = 400.0
    warehouse.addItem(Item("Keyboard", 5, 40.0));  // totalValue = 200.0
    // Total inventory value = 600.0 across 2 item kinds
    // Average item value = 600.0 / 2 = 300.0

    Report report(warehouse);
    assert(!report.isEmpty());
    assert(areClose(report.averageItemValue(), 300.0));

    // Add another item with zero quantity
    warehouse.addItem(Item("Mouse", 0, 50.0)); // totalValue = 0.0
    // Total inventory value = 600.0 across 3 item kinds
    // Average item value = 600.0 / 3 = 200.0
    Report report2(warehouse);
    assert(areClose(report2.averageItemValue(), 200.0));

    std::cout << "Test test_averageItemValue_populatedWarehouse: PASS" << std::endl;
}

// Test outOfStockCount with mix of in-stock and out-of-stock items
void test_outOfStockCount_mixedStock() {
    Warehouse warehouse("Grocery");
    warehouse.addItem(Item("Apples", 10, 0.5));
    warehouse.addItem(Item("Bananas", 0, 0.3));   // Out of stock
    warehouse.addItem(Item("Oranges", 0, 0.8));   // Out of stock
    warehouse.addItem(Item("Milk", 4, 1.2));
    warehouse.addItem(Item("Bread", 0, 2.0));     // Out of stock

    Report report(warehouse);
    assert(report.outOfStockCount() == 3);

    // All items in stock
    Warehouse fullyStocked("Hardware");
    fullyStocked.addItem(Item("Hammer", 5, 15.0));
    fullyStocked.addItem(Item("Nails", 100, 0.05));
    Report reportStocked(fullyStocked);
    assert(reportStocked.outOfStockCount() == 0);

    std::cout << "Test test_outOfStockCount_mixedStock: PASS" << std::endl;
}

// Test summaryLine behavior for empty vs non-empty warehouse reports
void test_summaryLine_labeledWarehouse() {
    Warehouse warehouse("Central Depot");
    Report reportEmpty(warehouse);
    assert(reportEmpty.summaryLine() == "empty");

    warehouse.addItem(Item("Widget", 3, 10.0));
    Report reportPopulated(warehouse);
    assert(reportPopulated.summaryLine() == "Central Depot");

    std::cout << "Test test_summaryLine_labeledWarehouse: PASS" << std::endl;
}

// Test isEmpty accuracy across modifications
void test_isEmpty() {
    Warehouse warehouse("Logistics");
    Report r1(warehouse);
    assert(r1.isEmpty() == true);

    warehouse.addItem(Item("Parcel", 1, 5.0));
    Report r2(warehouse);
    assert(r2.isEmpty() == false);

    std::cout << "Test test_isEmpty: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Report test suite..." << std::endl;

    test_defaultConstructor();
    test_parametrizedConstructor_emptyWarehouse();
    test_averageItemValue_populatedWarehouse();
    test_outOfStockCount_mixedStock();
    test_summaryLine_labeledWarehouse();
    test_isEmpty();

    std::cout << "All Report tests passed successfully!" << std::endl;
    return 0;
}