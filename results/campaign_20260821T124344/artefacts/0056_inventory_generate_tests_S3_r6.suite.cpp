#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function to compare floating point numbers
bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor and initial state
void test_report_defaultConstructor() {
    Report report;
    assert(report.isEmpty());
    assert(report.source.label == "unnamed");
    assert(report.source.itemCount() == 0);
    assert(areClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");
    std::cout << "Test test_report_defaultConstructor: PASS" << std::endl;
}

// Test parameterized constructor
void test_report_parameterizedConstructor() {
    Warehouse wh("Main Warehouse");
    wh.addItem(Item("Laptop", 10, 800.0));
    
    Report report(wh);
    assert(!report.isEmpty());
    assert(report.source.label == "Main Warehouse");
    assert(report.source.itemCount() == 1);
    std::cout << "Test test_report_parameterizedConstructor: PASS" << std::endl;
}

// Test isEmpty method
void test_report_isEmpty() {
    Warehouse emptyWh("Empty Depo");
    Report reportEmpty(emptyWh);
    assert(reportEmpty.isEmpty());

    Warehouse nonEmptyWh("Active Depo");
    nonEmptyWh.addItem(Item("Widget", 5, 2.50));
    Report reportNonEmpty(nonEmptyWh);
    assert(!reportNonEmpty.isEmpty());
    std::cout << "Test test_report_isEmpty: PASS" << std::endl;
}

// Test summaryLine method for empty and non-empty warehouses
void test_report_summaryLine() {
    Warehouse emptyWh("Storage A");
    Report reportEmpty(emptyWh);
    assert(reportEmpty.summaryLine() == "empty");

    Warehouse populatedWh("Storage B");
    populatedWh.addItem(Item("Screw", 100, 0.05));
    Report reportPopulated(populatedWh);
    assert(reportPopulated.summaryLine() == "Storage B");
    std::cout << "Test test_report_summaryLine: PASS" << std::endl;
}

// Test averageItemValue calculations
void test_report_averageItemValue() {
    // 1. Empty warehouse -> should be 0.0
    Report emptyReport;
    assert(areClose(emptyReport.averageItemValue(), 0.0));

    // 2. Single item warehouse
    Warehouse wh1("Single Item WH");
    wh1.addItem(Item("Monitor", 2, 150.0)); // Total value: 300.0, Item count: 1
    Report report1(wh1);
    assert(areClose(report1.averageItemValue(), 300.0));

    // 3. Multiple items warehouse
    Warehouse wh2("Multi Item WH");
    wh2.addItem(Item("Monitor", 2, 150.0)); // Value: 300.0
    wh2.addItem(Item("Keyboard", 5, 20.0));  // Value: 100.0
    wh2.addItem(Item("Mouse", 10, 10.0));    // Value: 100.0
    // Total Inventory Value: 500.0, Number of Item entries: 3 -> Average: 500.0 / 3 = 166.666667
    Report report2(wh2);
    assert(areClose(report2.averageItemValue(), 500.0 / 3.0));

    // 4. Warehouse with 0 quantity items
    Warehouse wh3("Zero Qty WH");
    wh3.addItem(Item("Book", 0, 15.0)); // Value: 0.0
    wh3.addItem(Item("Pen", 10, 2.0));  // Value: 20.0
    // Total Inventory Value: 20.0, Number of Item entries: 2 -> Average: 10.0
    Report report3(wh3);
    assert(areClose(report3.averageItemValue(), 10.0));

    std::cout << "Test test_report_averageItemValue: PASS" << std::endl;
}

// Test outOfStockCount method
void test_report_outOfStockCount() {
    Warehouse wh("Mixed Stock WH");
    Report emptyReport(wh);
    assert(emptyReport.outOfStockCount() == 0);

    // Add items with various quantities
    wh.addItem(Item("Chair", 0, 45.0));    // Out of stock
    wh.addItem(Item("Desk", 5, 120.0));    // In stock
    wh.addItem(Item("Lamp", 0, 15.0));     // Out of stock
    wh.addItem(Item("Cable", -1, 5.0));    // Quantity <= 0 is not in stock
    wh.addItem(Item("Notebook", 10, 2.0)); // In stock

    Report report(wh);
    // Chairs, Lamps, and Cables are out of stock (total 3)
    assert(report.outOfStockCount() == 3);

    std::cout << "Test test_report_outOfStockCount: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Report class unit tests..." << std::endl;

    test_report_defaultConstructor();
    test_report_parameterizedConstructor();
    test_report_isEmpty();
    test_report_summaryLine();
    test_report_averageItemValue();
    test_report_outOfStockCount();

    std::cout << "All Report unit tests passed successfully!" << std::endl;
    return 0;
}