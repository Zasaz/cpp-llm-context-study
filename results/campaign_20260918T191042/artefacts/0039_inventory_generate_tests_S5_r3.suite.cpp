#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function for floating point comparisons
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor and initial state
void test_Report_defaultConstructor() {
    Report report;
    
    assert(report.isEmpty() == true);
    assert(report.source.label == "unnamed");
    assert(report.source.itemCount() == 0);
    assert(areAlmostEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_Report_defaultConstructor: PASS" << std::endl;
}

// Test parameterized constructor
void test_Report_parameterizedConstructor() {
    Warehouse wh("Main Warehouse");
    wh.addItem(Item("Widget", 10, 2.5));
    
    Report report(wh);

    assert(report.isEmpty() == false);
    assert(report.source.label == "Main Warehouse");
    assert(report.source.itemCount() == 1);

    std::cout << "Test test_Report_parameterizedConstructor: PASS" << std::endl;
}

// Test isEmpty method
void test_Report_isEmpty() {
    Warehouse emptyWh("Empty Depot");
    Report emptyReport(emptyWh);
    assert(emptyReport.isEmpty() == true);

    Warehouse populatedWh("Active Depot");
    populatedWh.addItem(Item("Box", 5, 1.0));
    Report populatedReport(populatedWh);
    assert(populatedReport.isEmpty() == false);

    std::cout << "Test test_Report_isEmpty: PASS" << std::endl;
}

// Test summaryLine method for empty and populated warehouses
void test_Report_summaryLine() {
    // Empty warehouse should return "empty"
    Warehouse emptyWh("Empty Depot");
    Report emptyReport(emptyWh);
    assert(emptyReport.summaryLine() == "empty");

    // Populated warehouse should return its label
    Warehouse populatedWh("North Facility");
    populatedWh.addItem(Item("Scanner", 2, 99.99));
    Report populatedReport(populatedWh);
    assert(populatedReport.summaryLine() == "North Facility");

    std::cout << "Test test_Report_summaryLine: PASS" << std::endl;
}

// Test averageItemValue method with various counts and values
void test_Report_averageItemValue() {
    // Edge case: Empty warehouse
    Warehouse emptyWh("Empty Depot");
    Report emptyReport(emptyWh);
    assert(areAlmostEqual(emptyReport.averageItemValue(), 0.0));

    // Single item
    // Item 1: 10 * 5.0 = 50.0 -> average = 50.0 / 1 = 50.0
    Warehouse singleWh("Single Item Depot");
    singleWh.addItem(Item("Gadget", 10, 5.0));
    Report singleReport(singleWh);
    assert(areAlmostEqual(singleReport.averageItemValue(), 50.0));

    // Multiple items
    // Item 1: 10 * 5.0 = 50.0
    // Item 2: 2 * 25.0 = 50.0
    // Item 3: 0 * 100.0 = 0.0
    // Total Inventory Value = 100.0; Total Items = 3; Average = 100.0 / 3 = 33.333333
    Warehouse multiWh("Central Depot");
    multiWh.addItem(Item("Gadget", 10, 5.0));
    multiWh.addItem(Item("Tool", 2, 25.0));
    multiWh.addItem(Item("Device", 0, 100.0));
    Report multiReport(multiWh);
    assert(areAlmostEqual(multiReport.averageItemValue(), 100.0 / 3.0));

    std::cout << "Test test_Report_averageItemValue: PASS" << std::endl;
}

// Test outOfStockCount method
void test_Report_outOfStockCount() {
    // Empty warehouse
    Warehouse emptyWh("Empty Depot");
    Report emptyReport(emptyWh);
    assert(emptyReport.outOfStockCount() == 0);

    // All items in stock
    Warehouse inStockWh("In Stock Depot");
    inStockWh.addItem(Item("Pen", 10, 1.5));
    inStockWh.addItem(Item("Paper", 5, 4.0));
    Report inStockReport(inStockWh);
    assert(inStockReport.outOfStockCount() == 0);

    // Mix of in-stock and out-of-stock items
    Warehouse mixedWh("Mixed Depot");
    mixedWh.addItem(Item("Pen", 10, 1.5));        // In stock
    mixedWh.addItem(Item("Paper", 0, 4.0));       // Out of stock (quantity 0)
    mixedWh.addItem(Item("Eraser", -1, 0.5));     // Out of stock (negative quantity)
    mixedWh.addItem(Item("Notebook", 2, 3.0));    // In stock
    Report mixedReport(mixedWh);
    assert(mixedReport.outOfStockCount() == 2);

    // All items out of stock
    Warehouse outOfStockWh("Depleted Depot");
    outOfStockWh.addItem(Item("Pencil", 0, 0.5));
    outOfStockWh.addItem(Item("Ruler", 0, 1.2));
    Report outOfStockReport(outOfStockWh);
    assert(outOfStockReport.outOfStockCount() == 2);

    std::cout << "Test test_Report_outOfStockCount: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Report Unit Tests..." << std::endl;

    test_Report_defaultConstructor();
    test_Report_parameterizedConstructor();
    test_Report_isEmpty();
    test_Report_summaryLine();
    test_Report_averageItemValue();
    test_Report_outOfStockCount();

    std::cout << "All Report unit tests passed successfully!" << std::endl;
    return 0;
}