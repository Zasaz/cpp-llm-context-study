#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

#include "Item.h"
#include "Warehouse.h"
#include "Report.h"

// Helper function to compare floating point numbers
bool areDoublesEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor and default state
void test_Report_defaultConstructor() {
    Report report;
    
    assert(report.isEmpty() == true);
    assert(areDoublesEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");
    assert(report.source.label == "unnamed");
    assert(report.source.itemCount() == 0);

    std::cout << "Test test_Report_defaultConstructor: PASS" << std::endl;
}

// Test parameterized constructor with an empty warehouse
void test_Report_parameterizedConstructor_emptyWarehouse() {
    Warehouse wh("North Wing");
    Report report(wh);

    assert(report.isEmpty() == true);
    assert(areDoublesEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");
    assert(report.source.label == "North Wing");

    std::cout << "Test test_Report_parameterizedConstructor_emptyWarehouse: PASS" << std::endl;
}

// Test isEmpty method
void test_Report_isEmpty() {
    Warehouse wh("Storage");
    Report emptyReport(wh);
    assert(emptyReport.isEmpty() == true);

    wh.addItem(Item("Widget", 5, 2.50));
    Report populatedReport(wh);
    assert(populatedReport.isEmpty() == false);

    std::cout << "Test test_Report_isEmpty: PASS" << std::endl;
}

// Test summaryLine method for empty and non-empty warehouses
void test_Report_summaryLine() {
    Warehouse emptyWh("Main Depot");
    Report emptyReport(emptyWh);
    assert(emptyReport.summaryLine() == "empty");

    Warehouse activeWh("Main Depot");
    activeWh.addItem(Item("Gadget", 10, 15.0));
    Report activeReport(activeWh);
    assert(activeReport.summaryLine() == "Main Depot");

    std::cout << "Test test_Report_summaryLine: PASS" << std::endl;
}

// Test averageItemValue method
void test_Report_averageItemValue() {
    Warehouse wh("Electronics");
    
    // Case 1: Empty warehouse returns 0.0
    Report emptyReport(wh);
    assert(areDoublesEqual(emptyReport.averageItemValue(), 0.0));

    // Case 2: Single item type
    // Item: 10 units @ $5.00 = $50.00 total value. Count = 1 item type. Average = 50.0 / 1 = 50.0
    wh.addItem(Item("Keyboard", 10, 5.0));
    Report singleReport(wh);
    assert(areDoublesEqual(singleReport.averageItemValue(), 50.0));

    // Case 3: Multiple item types
    // Item 1: 10 @ $5.0 = $50
    // Item 2: 2 @ $100.0 = $200
    // Item 3: 0 @ $20.0 = $0 (out of stock item)
    // Total value = 50 + 200 + 0 = $250 across 3 items. Average = 250 / 3 = 83.333333...
    wh.addItem(Item("Monitor", 2, 100.0));
    wh.addItem(Item("Mouse", 0, 20.0));
    Report multiReport(wh);
    assert(areDoublesEqual(multiReport.averageItemValue(), 250.0 / 3.0));

    std::cout << "Test test_Report_averageItemValue: PASS" << std::endl;
}

// Test outOfStockCount method
void test_Report_outOfStockCount() {
    Warehouse wh("Produce");
    wh.addItem(Item("Apples", 50, 0.5));   // in stock
    wh.addItem(Item("Bananas", 0, 0.3));   // out of stock (0 qty)
    wh.addItem(Item("Cherries", 10, 2.0));  // in stock
    wh.addItem(Item("Dates", -1, 1.0));     // out of stock (< 0 qty)
    wh.addItem(Item("Elderberry", 0, 4.0)); // out of stock (0 qty)

    Report report(wh);
    assert(report.outOfStockCount() == 3);

    std::cout << "Test test_Report_outOfStockCount: PASS" << std::endl;
}

// Test state isolation (verifying Report holds its own copy of the Warehouse)
void test_Report_stateIsolation() {
    Warehouse wh("Depot");
    wh.addItem(Item("Boxes", 10, 2.0));

    Report report(wh);
    assert(report.source.itemCount() == 1);
    assert(areDoublesEqual(report.averageItemValue(), 20.0));

    // Modify original warehouse
    wh.addItem(Item("Tape", 5, 1.0));
    wh.withdraw("Boxes", 10);

    // Report should reflect the state at the time of construction
    assert(report.source.itemCount() == 1);
    assert(report.outOfStockCount() == 0);
    assert(areDoublesEqual(report.averageItemValue(), 20.0));

    std::cout << "Test test_Report_stateIsolation: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Report Unit Tests ===" << std::endl;

    test_Report_defaultConstructor();
    test_Report_parameterizedConstructor_emptyWarehouse();
    test_Report_isEmpty();
    test_Report_summaryLine();
    test_Report_averageItemValue();
    test_Report_outOfStockCount();
    test_Report_stateIsolation();

    std::cout << "=== All Report Unit Tests Passed Successfully ===" << std::endl;
    return 0;
}