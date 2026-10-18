#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Item.h"
#include "Warehouse.h"
#include "Report.h"

// Helper function for floating-point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor and default state
void test_Report_defaultConstructor() {
    Report report;
    assert(report.isEmpty() == true);
    assert(report.outOfStockCount() == 0);
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.summaryLine() == "empty");
    assert(report.source.label == "unnamed");

    std::cout << "Test test_Report_defaultConstructor: PASS" << std::endl;
}

// Test parameterized constructor with an empty warehouse
void test_Report_parameterizedConstructor_emptyWarehouse() {
    Warehouse wh("North Depot");
    Report report(wh);

    assert(report.isEmpty() == true);
    assert(report.outOfStockCount() == 0);
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.summaryLine() == "empty");
    assert(report.source.label == "North Depot");

    std::cout << "Test test_Report_parameterizedConstructor_emptyWarehouse: PASS" << std::endl;
}

// Test averageItemValue with zero items, one item, and multiple items
void test_Report_averageItemValue() {
    Warehouse wh("Main Warehouse");
    wh.addItem(Item("Laptop", 5, 1000.0)); // totalValue = 5000.0
    wh.addItem(Item("Mouse", 10, 25.0));   // totalValue = 250.0
    wh.addItem(Item("Keyboard", 2, 75.0)); // totalValue = 150.0
    // Total inventory value = 5400.0, item count = 3 -> average = 1800.0

    Report report(wh);
    assert(!report.isEmpty());
    assert(isClose(report.averageItemValue(), 1800.0));

    // Adding zero-value / zero-quantity item
    Warehouse wh2("Empty Item Warehouse");
    wh2.addItem(Item("Broken Item", 0, 100.0)); // totalValue = 0.0
    Report report2(wh2);
    assert(isClose(report2.averageItemValue(), 0.0));

    std::cout << "Test test_Report_averageItemValue: PASS" << std::endl;
}

// Test outOfStockCount under various conditions
void test_Report_outOfStockCount() {
    Warehouse wh("Storage");
    wh.addItem(Item("ItemA", 0, 10.0)); // out of stock
    wh.addItem(Item("ItemB", 5, 20.0)); // in stock
    wh.addItem(Item("ItemC", 0, 50.0)); // out of stock
    wh.addItem(Item("ItemD", 1, 100.0)); // in stock

    Report report(wh);
    assert(report.outOfStockCount() == 2);

    // Empty warehouse outOfStockCount
    Warehouse emptyWh("Empty");
    Report emptyReport(emptyWh);
    assert(emptyReport.outOfStockCount() == 0);

    // All in-stock warehouse
    Warehouse fullWh("Full");
    fullWh.addItem(Item("A", 1, 1.0));
    fullWh.addItem(Item("B", 2, 2.0));
    Report fullReport(fullWh);
    assert(fullReport.outOfStockCount() == 0);

    std::cout << "Test test_Report_outOfStockCount: PASS" << std::endl;
}

// Test summaryLine logic when warehouse is empty vs non-empty
void test_Report_summaryLine() {
    Warehouse wh("East Wing Storage");
    Report emptyReport(wh);
    assert(emptyReport.summaryLine() == "empty");

    wh.addItem(Item("Box", 10, 5.0));
    Report populatedReport(wh);
    assert(populatedReport.summaryLine() == "East Wing Storage");

    std::cout << "Test test_Report_summaryLine: PASS" << std::endl;
}

// Test isEmpty method directly
void test_Report_isEmpty() {
    Warehouse wh("West Depot");
    Report report1(wh);
    assert(report1.isEmpty() == true);

    wh.addItem(Item("Tool", 0, 15.0)); // quantity 0 still counts as an item entry
    Report report2(wh);
    assert(report2.isEmpty() == false);

    std::cout << "Test test_Report_isEmpty: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Report test suite..." << std::endl;

    test_Report_defaultConstructor();
    test_Report_parameterizedConstructor_emptyWarehouse();
    test_Report_averageItemValue();
    test_Report_outOfStockCount();
    test_Report_summaryLine();
    test_Report_isEmpty();

    std::cout << "All Report tests passed successfully!" << std::endl;
    return 0;
}