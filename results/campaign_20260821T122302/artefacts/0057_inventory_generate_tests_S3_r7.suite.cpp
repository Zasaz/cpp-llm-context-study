#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_defaultConstructor() {
    Report report;

    assert(report.isEmpty() == true);
    assert(report.source.label == "unnamed");
    assert(report.source.itemCount() == 0);
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_paramConstructor() {
    Warehouse wh("Central Hub");
    wh.addItem(Item("Widget", 10, 2.5));

    Report report(wh);

    assert(report.source.label == "Central Hub");
    assert(report.source.itemCount() == 1);
    assert(!report.isEmpty());

    std::cout << "Test test_paramConstructor: PASS" << std::endl;
}

void test_isEmpty() {
    // Empty report with default constructor
    Report emptyReport;
    assert(emptyReport.isEmpty() == true);

    // Empty report initialized with empty warehouse
    Warehouse emptyWh("Empty Depot");
    Report reportWithEmptyWh(emptyWh);
    assert(reportWithEmptyWh.isEmpty() == true);

    // Report with items
    Warehouse nonEmptyWh("Active Depot");
    nonEmptyWh.addItem(Item("Gadget", 0, 15.0)); // Item with 0 stock still counts as an item in warehouse
    Report reportWithItem(nonEmptyWh);
    assert(reportWithItem.isEmpty() == false);

    std::cout << "Test test_isEmpty: PASS" << std::endl;
}

void test_summaryLine() {
    // When empty, should return "empty"
    Warehouse emptyWh("North Facility");
    Report emptyReport(emptyWh);
    assert(emptyReport.summaryLine() == "empty");

    Report defaultReport;
    assert(defaultReport.summaryLine() == "empty");

    // When not empty, should return the warehouse label
    Warehouse wh("South Facility");
    wh.addItem(Item("Bolt", 100, 0.05));
    Report activeReport(wh);
    assert(activeReport.summaryLine() == "South Facility");

    std::cout << "Test test_summaryLine: PASS" << std::endl;
}

void test_averageItemValue_empty() {
    Report emptyReport;
    assert(isClose(emptyReport.averageItemValue(), 0.0));

    Warehouse emptyWh("Empty Stock");
    Report reportWithEmptyWh(emptyWh);
    assert(isClose(reportWithEmptyWh.averageItemValue(), 0.0));

    std::cout << "Test test_averageItemValue_empty: PASS" << std::endl;
}

void test_averageItemValue_multipleItems() {
    Warehouse wh("Supply Depot");
    // Item 1: 10 * $5.0 = $50.0
    wh.addItem(Item("Hammer", 10, 5.0));
    // Item 2: 2 * $25.0 = $50.0
    wh.addItem(Item("Drill", 2, 25.0));
    // Item 3: 5 * $4.0 = $20.0
    wh.addItem(Item("Nails", 5, 4.0));
    // Total Inventory Value = $120.0, Item Count = 3
    // Average Item Value = 120.0 / 3 = 40.0

    Report report(wh);
    assert(isClose(report.averageItemValue(), 40.0));

    // Add an item with 0 quantity (total value = 0.0)
    // New total value = 120.0, Item Count = 4 -> average = 30.0
    wh.addItem(Item("Saw", 0, 15.0));
    Report updatedReport(wh);
    assert(isClose(updatedReport.averageItemValue(), 30.0));

    std::cout << "Test test_averageItemValue_multipleItems: PASS" << std::endl;
}

void test_outOfStockCount() {
    Warehouse wh("Inventory");
    
    // 0 items total
    Report emptyReport(wh);
    assert(emptyReport.outOfStockCount() == 0);

    // All in stock
    wh.addItem(Item("Apples", 10, 1.0));
    wh.addItem(Item("Oranges", 5, 1.5));
    Report allInStockReport(wh);
    assert(allInStockReport.outOfStockCount() == 0);

    // Mix of in-stock and out-of-stock
    wh.addItem(Item("Bananas", 0, 0.5));
    wh.addItem(Item("Pears", 0, 1.2));
    Report mixedReport(wh);
    assert(mixedReport.outOfStockCount() == 2);

    // After consuming remaining stock from an item
    wh.withdraw("Apples", 10);
    Report afterWithdrawReport(wh);
    assert(afterWithdrawReport.outOfStockCount() == 3);

    std::cout << "Test test_outOfStockCount: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Report class unit tests..." << std::endl;

    test_defaultConstructor();
    test_paramConstructor();
    test_isEmpty();
    test_summaryLine();
    test_averageItemValue_empty();
    test_averageItemValue_multipleItems();
    test_outOfStockCount();

    std::cout << "All Report class unit tests passed successfully!" << std::endl;
    return 0;
}