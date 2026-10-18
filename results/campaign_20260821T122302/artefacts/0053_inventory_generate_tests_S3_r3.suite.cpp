#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

#include "Item.h"
#include "Warehouse.h"
#include "Report.h"

// Helper function for floating-point comparisons
static bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_defaultConstructor() {
    Report report;
    assert(report.isEmpty() == true);
    assert(report.source.label == "unnamed");
    assert(report.source.itemCount() == 0);
    assert(areClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_parameterizedConstructor() {
    Warehouse wh("North Hub");
    wh.addItem(Item("Widget", 10, 2.5));

    Report report(wh);
    assert(report.source.label == "North Hub");
    assert(report.source.itemCount() == 1);
    assert(!report.isEmpty());

    std::cout << "Test test_parameterizedConstructor: PASS" << std::endl;
}

void test_isEmpty() {
    // Empty warehouse
    Warehouse emptyWh("Empty Depot");
    Report emptyReport(emptyWh);
    assert(emptyReport.isEmpty() == true);

    // Non-empty warehouse
    Warehouse activeWh("Active Depot");
    activeWh.addItem(Item("Screws", 100, 0.05));
    Report activeReport(activeWh);
    assert(activeReport.isEmpty() == false);

    // Non-empty warehouse with zero-quantity item
    Warehouse zeroQtyWh("Zero Depot");
    zeroQtyWh.addItem(Item("Out-of-Stock Item", 0, 15.0));
    Report zeroQtyReport(zeroQtyWh);
    assert(zeroQtyReport.isEmpty() == false);

    std::cout << "Test test_isEmpty: PASS" << std::endl;
}

void test_averageItemValue_empty() {
    Warehouse wh("Empty");
    Report report(wh);
    assert(areClose(report.averageItemValue(), 0.0));

    std::cout << "Test test_averageItemValue_empty: PASS" << std::endl;
}

void test_averageItemValue_populated() {
    Warehouse wh("Central Logistics");
    // Item 1: 10 * $5.00 = $50.00
    wh.addItem(Item("Gadget A", 10, 5.0));
    // Item 2: 2 * $25.00 = $50.00
    wh.addItem(Item("Gadget B", 2, 25.0));
    // Item 3: 0 * $100.00 = $0.00
    wh.addItem(Item("Gadget C", 0, 100.0));

    // Total inventory value = 100.00, count of distinct items = 3
    // Expected average per item entry = 100.0 / 3.0 = 33.333333...
    Report report(wh);
    double expectedAvg = 100.0 / 3.0;
    assert(areClose(report.averageItemValue(), expectedAvg));

    // Single item case
    Warehouse singleWh("Single");
    singleWh.addItem(Item("Single Item", 4, 12.50)); // Total value = 50.00
    Report singleReport(singleWh);
    assert(areClose(singleReport.averageItemValue(), 50.0));

    std::cout << "Test test_averageItemValue_populated: PASS" << std::endl;
}

void test_outOfStockCount() {
    Warehouse wh("Stock Check");
    Report emptyReport(wh);
    assert(emptyReport.outOfStockCount() == 0);

    // Add in-stock items
    wh.addItem(Item("Apples", 10, 0.50));
    wh.addItem(Item("Bananas", 5, 0.30));
    Report report1(wh);
    assert(report1.outOfStockCount() == 0);

    // Add out-of-stock items
    wh.addItem(Item("Cherries", 0, 2.00));
    Report report2(wh);
    assert(report2.outOfStockCount() == 1);

    wh.addItem(Item("Dates", 0, 1.50));
    wh.addItem(Item("Elderberries", 20, 3.00));
    Report report3(wh);
    assert(report3.outOfStockCount() == 2);

    // Warehouse with only out-of-stock items
    Warehouse outWh("All Out");
    outWh.addItem(Item("Item1", 0, 10.0));
    outWh.addItem(Item("Item2", 0, 20.0));
    Report outReport(outWh);
    assert(outReport.outOfStockCount() == 2);

    std::cout << "Test test_outOfStockCount: PASS" << std::endl;
}

void test_summaryLine() {
    // Empty warehouse report should always return "empty" regardless of warehouse label
    Warehouse emptyNamed("Warehouse 99");
    Report emptyReport(emptyNamed);
    assert(emptyReport.summaryLine() == "empty");

    // Populated warehouse should return its warehouse label
    Warehouse activeWh("Warehouse 99");
    activeWh.addItem(Item("Pallet", 1, 150.0));
    Report activeReport(activeWh);
    assert(activeReport.summaryLine() == "Warehouse 99");

    // Populated warehouse with custom label
    Warehouse customWh("East Wing Depot");
    customWh.addItem(Item("Box", 5, 2.0));
    Report customReport(customWh);
    assert(customReport.summaryLine() == "East Wing Depot");

    std::cout << "Test test_summaryLine: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Report Unit Tests ===" << std::endl;

    test_defaultConstructor();
    test_parameterizedConstructor();
    test_isEmpty();
    test_averageItemValue_empty();
    test_averageItemValue_populated();
    test_outOfStockCount();
    test_summaryLine();

    std::cout << "=== All Report Unit Tests Passed Successfully ===" << std::endl;
    return 0;
}