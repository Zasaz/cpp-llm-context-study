#include <iostream>
#include <cassert>
#include <cmath>
#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function to compare floating point numbers
bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_defaultConstructor() {
    Report report;
    
    assert(report.isEmpty() == true);
    assert(report.outOfStockCount() == 0);
    assert(areClose(report.averageItemValue(), 0.0));
    assert(report.summaryLine() == "empty");
    assert(report.source.label == "unnamed");

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_parameterizedConstructor() {
    Warehouse wh("Main Warehouse");
    wh.addItem(Item("Widget", 10, 5.0)); // totalValue = 50.0

    Report report(wh);

    assert(report.isEmpty() == false);
    assert(report.source.label == "Main Warehouse");
    assert(report.source.itemCount() == 1);

    std::cout << "Test test_parameterizedConstructor: PASS" << std::endl;
}

void test_isEmpty() {
    Warehouse emptyWh("Empty Depo");
    Report emptyReport(emptyWh);
    assert(emptyReport.isEmpty() == true);

    Warehouse nonEmptyWh("Stocked Depo");
    nonEmptyWh.addItem(Item("Gadget", 0, 15.0)); // In warehouse even if qty is 0
    Report nonEmptyReport(nonEmptyWh);
    assert(nonEmptyReport.isEmpty() == false);

    std::cout << "Test test_isEmpty: PASS" << std::endl;
}

void test_summaryLine() {
    Report emptyReport;
    assert(emptyReport.summaryLine() == "empty");

    Warehouse emptyWhNamed("Custom Name");
    Report namedEmptyReport(emptyWhNamed);
    assert(namedEmptyReport.summaryLine() == "empty");

    Warehouse wh("Central Hub");
    wh.addItem(Item("Cable", 5, 2.50));
    Report report(wh);
    assert(report.summaryLine() == "Central Hub");

    std::cout << "Test test_summaryLine: PASS" << std::endl;
}

void test_averageItemValue_empty() {
    Report report;
    assert(areClose(report.averageItemValue(), 0.0));

    Warehouse emptyWh("Empty");
    Report reportFromWh(emptyWh);
    assert(areClose(reportFromWh.averageItemValue(), 0.0));

    std::cout << "Test test_averageItemValue_empty: PASS" << std::endl;
}

void test_averageItemValue_nonEmpty() {
    Warehouse wh("Distribution Center");
    // Item 1: 10 units @ 20.0 = 200.0
    wh.addItem(Item("Laptop", 10, 20.0));
    // Item 2: 5 units @ 40.0 = 200.0
    wh.addItem(Item("Monitor", 5, 40.0));
    // Item 3: 0 units @ 100.0 = 0.0
    wh.addItem(Item("Desk", 0, 100.0));

    // Total Inventory Value = 200 + 200 + 0 = 400.0
    // Total Items count = 3
    // Expected Average = 400.0 / 3.0 = 133.333333...
    Report report(wh);
    assert(areClose(report.averageItemValue(), 400.0 / 3.0));

    std::cout << "Test test_averageItemValue_nonEmpty: PASS" << std::endl;
}

void test_outOfStockCount() {
    Warehouse wh("Logistics Hub");
    
    // Empty warehouse has 0 out-of-stock items
    Report emptyReport(wh);
    assert(emptyReport.outOfStockCount() == 0);

    // Add in-stock and out-of-stock items
    wh.addItem(Item("Item1", 10, 5.0)); // In stock
    wh.addItem(Item("Item2", 0, 15.0)); // Out of stock
    wh.addItem(Item("Item3", -2, 2.0)); // Out of stock (negative quantity)
    wh.addItem(Item("Item4", 1, 100.0)); // In stock
    wh.addItem(Item("Item5", 0, 0.0));  // Out of stock

    Report report(wh);
    assert(report.outOfStockCount() == 3);

    std::cout << "Test test_outOfStockCount: PASS" << std::endl;
}

int main() {
    test_defaultConstructor();
    test_parameterizedConstructor();
    test_isEmpty();
    test_summaryLine();
    test_averageItemValue_empty();
    test_averageItemValue_nonEmpty();
    test_outOfStockCount();

    std::cout << "\nAll Report tests passed successfully!" << std::endl;
    return 0;
}