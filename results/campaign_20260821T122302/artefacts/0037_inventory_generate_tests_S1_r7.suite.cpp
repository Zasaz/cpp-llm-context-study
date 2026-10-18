#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

void test_default_constructor() {
    Report report;
    
    assert(report.isEmpty() == true);
    assert(report.source.label == "unnamed");
    assert(report.source.itemCount() == 0);
    assert(report.averageItemValue() == 0.0);
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_default_constructor: PASS" << std::endl;
}

void test_parameterized_constructor() {
    Warehouse wh("North Facility");
    wh.addItem(Item("Widget", 10, 2.50));
    wh.addItem(Item("Gadget", 5, 4.00));

    Report report(wh);

    assert(report.source.label == "North Facility");
    assert(report.source.itemCount() == 2);
    assert(report.isEmpty() == false);

    std::cout << "Test test_parameterized_constructor: PASS" << std::endl;
}

void test_isEmpty() {
    // Empty warehouse report
    Warehouse emptyWh("Central Depot");
    Report emptyReport(emptyWh);
    assert(emptyReport.isEmpty() == true);

    // Non-empty warehouse report
    Warehouse populatedWh("Central Depot");
    populatedWh.addItem(Item("Screws", 100, 0.05));
    Report populatedReport(populatedWh);
    assert(populatedReport.isEmpty() == false);

    // Item with 0 quantity is still an item in the warehouse
    Warehouse zeroStockWh("Empty Stock Depot");
    zeroStockWh.addItem(Item("Bolts", 0, 0.10));
    Report zeroStockReport(zeroStockWh);
    assert(zeroStockReport.isEmpty() == false);

    std::cout << "Test test_isEmpty: PASS" << std::endl;
}

void test_averageItemValue_empty() {
    Warehouse emptyWh("Empty Depot");
    Report report(emptyWh);

    assert(isClose(report.averageItemValue(), 0.0));

    Report defaultReport;
    assert(isClose(defaultReport.averageItemValue(), 0.0));

    std::cout << "Test test_averageItemValue_empty: PASS" << std::endl;
}

void test_averageItemValue_with_items() {
    Warehouse wh("Main Warehouse");
    // Item 1: 10 * 5.00 = 50.00
    wh.addItem(Item("ItemA", 10, 5.00));
    // Item 2: 2 * 25.00 = 50.00
    wh.addItem(Item("ItemB", 2, 25.00));
    // Item 3: 4 * 12.50 = 50.00
    wh.addItem(Item("ItemC", 4, 12.50));

    // Total Inventory Value = 150.00, Number of distinct item entries = 3
    // Expected average item value = 150.00 / 3 = 50.00
    Report report(wh);
    assert(isClose(report.averageItemValue(), 50.00));

    // Add an item with 0 quantity (value = 0)
    wh.addItem(Item("ItemD", 0, 100.00));
    // Total Inventory Value = 150.00, Number of items = 4
    // Expected average item value = 150.00 / 4 = 37.50
    Report updatedReport(wh);
    assert(isClose(updatedReport.averageItemValue(), 37.50));

    std::cout << "Test test_averageItemValue_with_items: PASS" << std::endl;
}

void test_outOfStockCount() {
    Warehouse wh("Storage");
    
    Report emptyReport(wh);
    assert(emptyReport.outOfStockCount() == 0);

    // All items in stock
    wh.addItem(Item("Apples", 10, 0.50));
    wh.addItem(Item("Oranges", 5, 0.75));
    Report allInStockReport(wh);
    assert(allInStockReport.outOfStockCount() == 0);

    // Add out-of-stock items
    wh.addItem(Item("Bananas", 0, 0.30));
    Report oneOutOfStockReport(wh);
    assert(oneOutOfStockReport.outOfStockCount() == 1);

    wh.addItem(Item("Grapes", 0, 1.20));
    Report twoOutOfStockReport(wh);
    assert(twoOutOfStockReport.outOfStockCount() == 2);

    std::cout << "Test test_outOfStockCount: PASS" << std::endl;
}

void test_summaryLine() {
    // When empty, summaryLine must return "empty"
    Warehouse emptyWh("Alpha Hub");
    Report emptyReport(emptyWh);
    assert(emptyReport.summaryLine() == "empty");

    Report defaultReport;
    assert(defaultReport.summaryLine() == "empty");

    // When not empty, summaryLine must return warehouse label
    Warehouse populatedWh("Beta Hub");
    populatedWh.addItem(Item("Cables", 15, 3.99));
    Report populatedReport(populatedWh);
    assert(populatedReport.summaryLine() == "Beta Hub");

    std::cout << "Test test_summaryLine: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Report class unit tests..." << std::endl;

    test_default_constructor();
    test_parameterized_constructor();
    test_isEmpty();
    test_averageItemValue_empty();
    test_averageItemValue_with_items();
    test_outOfStockCount();
    test_summaryLine();

    std::cout << "All Report unit tests passed successfully!" << std::endl;
    return 0;
}