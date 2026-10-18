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

void test_defaultConstructor() {
    Report report;
    
    assert(report.isEmpty());
    assert(report.source.label == "unnamed");
    assert(report.source.itemCount() == 0);
    assert(areAlmostEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_parameterizedConstructor() {
    Warehouse wh("North Facility");
    wh.addItem(Item("Widget", 10, 5.0)); // totalValue = 50.0

    Report report(wh);

    assert(!report.isEmpty());
    assert(report.source.label == "North Facility");
    assert(report.source.itemCount() == 1);
    assert(report.summaryLine() == "North Facility");

    std::cout << "Test test_parameterizedConstructor: PASS" << std::endl;
}

void test_isEmpty() {
    Warehouse emptyWh("Empty Depot");
    Report emptyReport(emptyWh);
    assert(emptyReport.isEmpty());

    Warehouse populatedWh("Active Depot");
    populatedWh.addItem(Item("Gizmo", 0, 15.0)); // In warehouse even if quantity is 0
    Report populatedReport(populatedWh);
    assert(!populatedReport.isEmpty());

    std::cout << "Test test_isEmpty: PASS" << std::endl;
}

void test_summaryLine() {
    // Empty warehouse should return "empty" regardless of warehouse label
    Warehouse emptyWh("South Hub");
    Report report1(emptyWh);
    assert(report1.summaryLine() == "empty");

    // Default constructed report is empty
    Report reportDefault;
    assert(reportDefault.summaryLine() == "empty");

    // Populated warehouse should return its label
    Warehouse whWithItems("South Hub");
    whWithItems.addItem(Item("Cable", 5, 2.5));
    Report report2(whWithItems);
    assert(report2.summaryLine() == "South Hub");

    std::cout << "Test test_summaryLine: PASS" << std::endl;
}

void test_averageItemValue() {
    // 1. Empty Report
    Report emptyReport;
    assert(areAlmostEqual(emptyReport.averageItemValue(), 0.0));

    // 2. Single item
    Warehouse wh1("Single Item WH");
    wh1.addItem(Item("Box", 4, 12.5)); // Total value: 50.0
    Report report1(wh1);
    assert(areAlmostEqual(report1.averageItemValue(), 50.0)); // 50.0 / 1 item = 50.0

    // 3. Multiple items
    Warehouse wh2("Multi Item WH");
    wh2.addItem(Item("ItemA", 10, 5.0));  // Value = 50.0
    wh2.addItem(Item("ItemB", 2, 25.0));  // Value = 50.0
    wh2.addItem(Item("ItemC", 0, 100.0)); // Value = 0.0
    // Total inventory value = 100.0, Count = 3 items
    Report report2(wh2);
    assert(areAlmostEqual(report2.averageItemValue(), 100.0 / 3.0));

    std::cout << "Test test_averageItemValue: PASS" << std::endl;
}

void test_outOfStockCount() {
    Warehouse wh("Inventory Center");
    Report emptyReport(wh);
    assert(emptyReport.outOfStockCount() == 0);

    // Add in-stock items
    wh.addItem(Item("Item1", 5, 10.0));
    wh.addItem(Item("Item2", 1, 20.0));
    Report report1(wh);
    assert(report1.outOfStockCount() == 0);

    // Add out-of-stock items (quantity <= 0)
    wh.addItem(Item("Item3", 0, 15.0));
    wh.addItem(Item("Item4", 0, 5.0));
    Report report2(wh);
    assert(report2.outOfStockCount() == 2);

    // Withdraw remaining stock from an item
    wh.withdraw("Item2", 1); // quantity goes from 1 to 0
    Report report3(wh);
    assert(report3.outOfStockCount() == 3);

    std::cout << "Test test_outOfStockCount: PASS" << std::endl;
}

int main() {
    std::cout << "--- Running Report Unit Tests ---" << std::endl;

    test_defaultConstructor();
    test_parameterizedConstructor();
    test_isEmpty();
    test_summaryLine();
    test_averageItemValue();
    test_outOfStockCount();

    std::cout << "--- All Report Tests Passed Successfully ---" << std::endl;
    return 0;
}