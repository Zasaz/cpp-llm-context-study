#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

#include "Item.h"
#include "Warehouse.h"
#include "Report.h"

// Helper function to compare floating point values
static bool areDoublesEqual(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// Test default constructor behavior
void test_Report_defaultConstructor() {
    Report report;

    assert(report.isEmpty() == true);
    assert(areDoublesEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_Report_defaultConstructor: PASS" << std::endl;
}

// Test parameterized constructor with an empty warehouse
void test_Report_parameterizedConstructor_emptyWarehouse() {
    Warehouse wh("Central Logistics");
    Report report(wh);

    assert(report.source.label == "Central Logistics");
    assert(report.isEmpty() == true);
    assert(areDoublesEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_Report_parameterizedConstructor_emptyWarehouse: PASS" << std::endl;
}

// Test averageItemValue calculations with different warehouse contents
void test_Report_averageItemValue() {
    Warehouse wh("Tech Depot");

    // Case 1: Empty warehouse -> average 0.0
    Report emptyReport(wh);
    assert(areDoublesEqual(emptyReport.averageItemValue(), 0.0));

    // Case 2: Single item
    // 10 keyboards at $15.50 each -> total = $155.0
    wh.addItem(Item("Keyboard", 10, 15.50));
    Report singleItemReport(wh);
    assert(areDoublesEqual(singleItemReport.averageItemValue(), 155.0));

    // Case 3: Multiple items
    // 5 mice at $20.00 each -> total = $100.0
    // 0 monitors at $200.00 each -> total = $0.0
    wh.addItem(Item("Mouse", 5, 20.00));
    wh.addItem(Item("Monitor", 0, 200.00));
    // Total inventory value: 155.0 + 100.0 + 0.0 = 255.0
    // Total item kinds (itemCount): 3
    // Expected average: 255.0 / 3 = 85.0
    Report multiItemReport(wh);
    assert(areDoublesEqual(multiItemReport.averageItemValue(), 85.0));

    std::cout << "Test test_Report_averageItemValue: PASS" << std::endl;
}

// Test outOfStockCount function
void test_Report_outOfStockCount() {
    Warehouse wh("Grocery Hub");

    // All items in stock
    wh.addItem(Item("Apples", 50, 0.50));
    wh.addItem(Item("Bananas", 30, 0.30));
    wh.addItem(Item("Oranges", 20, 0.80));

    Report reportAllInStock(wh);
    assert(reportAllInStock.outOfStockCount() == 0);

    // Add items that are out of stock (quantity == 0)
    wh.addItem(Item("Milk", 0, 2.50));
    wh.addItem(Item("Bread", 0, 1.99));

    Report reportWithOutOfStock(wh);
    assert(reportWithOutOfStock.outOfStockCount() == 2);

    // Withdraw entire stock of an item
    wh.withdraw("Apples", 50);
    Report reportAfterWithdraw(wh);
    assert(reportAfterWithdraw.outOfStockCount() == 3);

    std::cout << "Test test_Report_outOfStockCount: PASS" << std::endl;
}

// Test isEmpty member function
void test_Report_isEmpty() {
    Warehouse wh("Distribution Center");
    Report reportEmpty(wh);
    assert(reportEmpty.isEmpty() == true);

    // Adding an item makes it non-empty
    wh.addItem(Item("Widget", 0, 10.0)); // even with quantity 0, the item exists in warehouse
    Report reportNonEmpty(wh);
    assert(reportNonEmpty.isEmpty() == false);

    std::cout << "Test test_Report_isEmpty: PASS" << std::endl;
}

// Test summaryLine member function
void test_Report_summaryLine() {
    // Empty warehouse should yield "empty"
    Warehouse emptyWh("North Port");
    Report emptyReport(emptyWh);
    assert(emptyReport.summaryLine() == "empty");

    // Non-empty warehouse should yield warehouse label
    Warehouse populatedWh("North Port");
    populatedWh.addItem(Item("Container", 1, 5000.0));
    Report populatedReport(populatedWh);
    assert(populatedReport.summaryLine() == "North Port");

    // Default warehouse with item should return default label "unnamed"
    Warehouse defaultWh;
    defaultWh.addItem(Item("Box", 2, 5.0));
    Report defaultReport(defaultWh);
    assert(defaultReport.summaryLine() == "unnamed");

    std::cout << "Test test_Report_summaryLine: PASS" << std::endl;
}

int main() {
    std::cout << "================ Running Report Tests ================" << std::endl;

    test_Report_defaultConstructor();
    test_Report_parameterizedConstructor_emptyWarehouse();
    test_Report_averageItemValue();
    test_Report_outOfStockCount();
    test_Report_isEmpty();
    test_Report_summaryLine();

    std::cout << "================ All Tests Passed Successfully! ================" << std::endl;
    return 0;
}