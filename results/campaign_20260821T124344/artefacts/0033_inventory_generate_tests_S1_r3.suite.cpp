#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function to compare floating-point numbers
bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor and default state
void test_Report_defaultConstructor() {
    Report report;
    assert(report.isEmpty() == true);
    assert(report.source.itemCount() == 0);
    assert(report.source.label == "unnamed");
    assert(approxEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");
    std::cout << "Test test_Report_defaultConstructor: PASS" << std::endl;
}

// Test parameterized constructor
void test_Report_parameterizedConstructor() {
    Warehouse wh("Main Warehouse");
    wh.addItem(Item("Widget", 10, 2.50));
    
    Report report(wh);
    assert(!report.isEmpty());
    assert(report.source.label == "Main Warehouse");
    assert(report.source.itemCount() == 1);
    std::cout << "Test test_Report_parameterizedConstructor: PASS" << std::endl;
}

// Test isEmpty method
void test_Report_isEmpty() {
    Warehouse emptyWh("Empty Depot");
    Report reportEmpty(emptyWh);
    assert(reportEmpty.isEmpty() == true);

    Warehouse nonEmptyWh("Supply Depot");
    nonEmptyWh.addItem(Item("Screw", 100, 0.05));
    Report reportNonEmpty(nonEmptyWh);
    assert(reportNonEmpty.isEmpty() == false);

    std::cout << "Test test_Report_isEmpty: PASS" << std::endl;
}

// Test summaryLine method for empty and non-empty reports
void test_Report_summaryLine() {
    Warehouse emptyWh("North Wing");
    Report emptyReport(emptyWh);
    assert(emptyReport.summaryLine() == "empty");

    Warehouse activeWh("North Wing");
    activeWh.addItem(Item("Hammer", 5, 12.99));
    Report activeReport(activeWh);
    assert(activeReport.summaryLine() == "North Wing");

    std::cout << "Test test_Report_summaryLine: PASS" << std::endl;
}

// Test averageItemValue method with various counts and values
void test_Report_averageItemValue() {
    // 1. Empty warehouse -> should return 0.0
    Warehouse whEmpty("Empty");
    Report reportEmpty(whEmpty);
    assert(approxEqual(reportEmpty.averageItemValue(), 0.0));

    // 2. Single item: 10 units at $5.0 each -> total $50.0. Average = 50.0 / 1 item = 50.0
    Warehouse whSingle("Single");
    whSingle.addItem(Item("ItemA", 10, 5.0));
    Report reportSingle(whSingle);
    assert(approxEqual(reportSingle.averageItemValue(), 50.0));

    // 3. Multiple items:
    // Item 1: 10 units @ $5.0 = $50.0
    // Item 2: 2 units @ $25.0 = $50.0
    // Item 3: 0 units @ $10.0 = $0.0
    // Total Inventory Value = $100.0 across 3 item entries -> Average = 100.0 / 3 = 33.333333
    Warehouse whMulti("Multi");
    whMulti.addItem(Item("ItemA", 10, 5.0));
    whMulti.addItem(Item("ItemB", 2, 25.0));
    whMulti.addItem(Item("ItemC", 0, 10.0));
    Report reportMulti(whMulti);
    assert(approxEqual(reportMulti.averageItemValue(), 100.0 / 3.0));

    std::cout << "Test test_Report_averageItemValue: PASS" << std::endl;
}

// Test outOfStockCount method
void test_Report_outOfStockCount() {
    Warehouse wh("Inventory");
    // Initially empty
    Report reportEmpty(wh);
    assert(reportEmpty.outOfStockCount() == 0);

    // Add items with positive stock
    wh.addItem(Item("Apples", 10, 0.5));
    wh.addItem(Item("Bananas", 5, 0.3));
    Report reportInStock(wh);
    assert(reportInStock.outOfStockCount() == 0);

    // Add items with 0 stock
    wh.addItem(Item("Oranges", 0, 0.8));
    wh.addItem(Item("Pears", 0, 1.2));
    Report reportWithOutOfStock(wh);
    assert(reportWithOutOfStock.outOfStockCount() == 2);

    std::cout << "Test test_Report_outOfStockCount: PASS" << std::endl;
}

// Test interaction after warehouse modification
void test_Report_snapshotBehavior() {
    Warehouse wh("Depot");
    wh.addItem(Item("Box", 10, 2.0));

    Report report(wh);
    assert(report.source.totalUnits() == 10);
    assert(report.outOfStockCount() == 0);

    // Modifying the original warehouse should not change the already constructed Report
    // because Report stores source by value (a copy).
    wh.withdraw("Box", 10);
    assert(wh.items[0].quantity == 0);
    assert(report.source.items[0].quantity == 10);
    assert(report.outOfStockCount() == 0);

    // A newly created report reflects the modified warehouse
    Report updatedReport(wh);
    assert(updatedReport.outOfStockCount() == 1);

    std::cout << "Test test_Report_snapshotBehavior: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Report Tests ===" << std::endl;

    test_Report_defaultConstructor();
    test_Report_parameterizedConstructor();
    test_Report_isEmpty();
    test_Report_summaryLine();
    test_Report_averageItemValue();
    test_Report_outOfStockCount();
    test_Report_snapshotBehavior();

    std::cout << "=== All Report Tests Passed Successfully ===" << std::endl;
    return 0;
}