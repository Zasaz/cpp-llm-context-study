#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Item.h"
#include "Warehouse.h"
#include "Report.h"

// Helper function for floating point comparisons
bool areAlmostEqual(double a, double b, double epsilon = 0.0001) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor and its default values
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
    Warehouse wh("Central Storage");
    wh.addItem(Item("Widget", 10, 5.0));

    Report report(wh);

    assert(report.isEmpty() == false);
    assert(report.source.label == "Central Storage");
    assert(report.source.itemCount() == 1);
    assert(report.summaryLine() == "Central Storage");

    std::cout << "Test test_Report_parameterizedConstructor: PASS" << std::endl;
}

// Test averageItemValue with empty warehouse
void test_Report_averageItemValue_empty() {
    Warehouse wh("Empty Storage");
    Report report(wh);

    assert(areAlmostEqual(report.averageItemValue(), 0.0));

    std::cout << "Test test_Report_averageItemValue_empty: PASS" << std::endl;
}

// Test averageItemValue with multiple items of varying quantities and prices
void test_Report_averageItemValue_multipleItems() {
    Warehouse wh("Hub");
    // Item 1: 5 * 10.0 = 50.0
    wh.addItem(Item("ItemA", 5, 10.0));
    // Item 2: 2 * 25.0 = 50.0
    wh.addItem(Item("ItemB", 2, 25.0));
    // Item 3: 0 * 100.0 = 0.0
    wh.addItem(Item("ItemC", 0, 100.0));
    // Total Inventory Value = 100.0
    // Distinct Item Types Count = 3
    // Expected Average Item Value = 100.0 / 3 = 33.3333...

    Report report(wh);

    assert(report.source.itemCount() == 3);
    assert(areAlmostEqual(report.source.inventoryValue(), 100.0));
    assert(areAlmostEqual(report.averageItemValue(), 100.0 / 3.0));

    std::cout << "Test test_Report_averageItemValue_multipleItems: PASS" << std::endl;
}

// Test outOfStockCount with empty, all in-stock, mixed, and all out-of-stock items
void test_Report_outOfStockCount() {
    Warehouse wh("Stock Warehouse");
    Report emptyReport(wh);
    assert(emptyReport.outOfStockCount() == 0);

    wh.addItem(Item("InStock1", 10, 2.0));
    wh.addItem(Item("InStock2", 1, 5.0));
    Report allInStockReport(wh);
    assert(allInStockReport.outOfStockCount() == 0);

    wh.addItem(Item("OutOfStock1", 0, 15.0));
    wh.addItem(Item("OutOfStock2", -5, 20.0)); // Negative/zero quantity counts as not in stock
    Report mixedReport(wh);
    assert(mixedReport.outOfStockCount() == 2);

    Warehouse whEmptyStock("Empty Stocks");
    whEmptyStock.addItem(Item("OutOfStockA", 0, 1.0));
    whEmptyStock.addItem(Item("OutOfStockB", 0, 2.0));
    Report allOutOfStockReport(whEmptyStock);
    assert(allOutOfStockReport.outOfStockCount() == 2);

    std::cout << "Test test_Report_outOfStockCount: PASS" << std::endl;
}

// Test isEmpty function
void test_Report_isEmpty() {
    Warehouse wh("Test WH");
    Report report(wh);
    assert(report.isEmpty() == true);

    wh.addItem(Item("Gadget", 0, 10.0)); // Even with quantity 0, the item entry exists
    Report nonEmptyReport(wh);
    assert(nonEmptyReport.isEmpty() == false);

    std::cout << "Test test_Report_isEmpty: PASS" << std::endl;
}

// Test summaryLine under empty and non-empty conditions
void test_Report_summaryLine() {
    // Empty warehouse
    Warehouse emptyWh("Main Depot");
    Report emptyReport(emptyWh);
    assert(emptyReport.summaryLine() == "empty");

    // Non-empty warehouse
    Warehouse wh("North Facility");
    wh.addItem(Item("Hammer", 3, 12.50));
    Report populatedReport(wh);
    assert(populatedReport.summaryLine() == "North Facility");

    std::cout << "Test test_Report_summaryLine: PASS" << std::endl;
}

// Test dynamic modifications on report's internal warehouse
void test_Report_warehouseModifications() {
    Report report;
    report.source.label = "Custom Label";
    report.source.addItem(Item("Bolts", 100, 0.10)); // Total: 10.0
    report.source.addItem(Item("Nuts", 50, 0.20));   // Total: 10.0

    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "Custom Label");
    assert(report.source.itemCount() == 2);
    assert(areAlmostEqual(report.averageItemValue(), 10.0));
    assert(report.outOfStockCount() == 0);

    // Consume all bolts
    bool success = report.source.withdraw("Bolts", 100);
    assert(success == true);
    assert(report.outOfStockCount() == 1);
    assert(areAlmostEqual(report.averageItemValue(), 5.0)); // 10.0 total / 2 items = 5.0

    std::cout << "Test test_Report_warehouseModifications: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Report Unit Tests ===" << std::endl;

    test_Report_defaultConstructor();
    test_Report_parameterizedConstructor();
    test_Report_averageItemValue_empty();
    test_Report_averageItemValue_multipleItems();
    test_Report_outOfStockCount();
    test_Report_isEmpty();
    test_Report_summaryLine();
    test_Report_warehouseModifications();

    std::cout << "=== All Report tests passed successfully! ===" << std::endl;
    return 0;
}