#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double tolerance = 1e-6) {
    return std::fabs(a - b) < tolerance;
}

// Test default constructor behavior
void test_Report_defaultConstructor() {
    Report report;
    
    // Default warehouse has label "unnamed" and 0 items
    assert(report.isEmpty());
    assert(report.averageItemValue() == 0.0);
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");
    assert(report.source.label == "unnamed");
    
    std::cout << "Test test_Report_defaultConstructor: PASS" << std::endl;
}

// Test parameterized constructor
void test_Report_parameterizedConstructor() {
    Warehouse wh("North Facility");
    wh.addItem(Item("Widget", 10, 2.50));
    
    Report report(wh);
    assert(!report.isEmpty());
    assert(report.source.label == "North Facility");
    assert(report.source.itemCount() == 1);
    assert(report.summaryLine() == "North Facility");
    
    std::cout << "Test test_Report_parameterizedConstructor: PASS" << std::endl;
}

// Test isEmpty method
void test_Report_isEmpty() {
    Warehouse emptyWh("Empty Depot");
    Report emptyReport(emptyWh);
    assert(emptyReport.isEmpty());

    Warehouse populatedWh("Active Depot");
    populatedWh.addItem(Item("Gadget", 0, 5.00)); // Even with 0 stock, itemCount is 1
    Report populatedReport(populatedWh);
    assert(!populatedReport.isEmpty());

    std::cout << "Test test_Report_isEmpty: PASS" << std::endl;
}

// Test summaryLine method
void test_Report_summaryLine() {
    Warehouse emptyWh("Central Hub");
    Report emptyReport(emptyWh);
    // When warehouse is empty, summaryLine returns "empty" regardless of warehouse label
    assert(emptyReport.summaryLine() == "empty");

    Warehouse populatedWh("Central Hub");
    populatedWh.addItem(Item("Screws", 100, 0.05));
    Report populatedReport(populatedWh);
    // When warehouse has items, summaryLine returns the label
    assert(populatedReport.summaryLine() == "Central Hub");

    std::cout << "Test test_Report_summaryLine: PASS" << std::endl;
}

// Test averageItemValue with zero items, single item, and multiple items
void test_Report_averageItemValue() {
    Warehouse wh("Finance Test");
    Report emptyReport(wh);
    assert(emptyReport.averageItemValue() == 0.0);

    // Add 1 item: 10 units at $5.00 = $50.00 total value. Average = 50.00 / 1 = 50.00
    wh.addItem(Item("Box", 10, 5.00));
    Report singleReport(wh);
    assert(isClose(singleReport.averageItemValue(), 50.00));

    // Add 2nd item: 2 units at $15.00 = $30.00 total value. 
    // Total inventory value = 50.00 + 30.00 = $80.00. Average = 80.00 / 2 = 40.00
    wh.addItem(Item("Pallet", 2, 15.00));
    Report multiReport(wh);
    assert(isClose(multiReport.averageItemValue(), 40.00));

    // Add 3rd item with 0 quantity: 0 units at $20.00 = $0.00 total value.
    // Total inventory value = $80.00. Count = 3. Average = 80.00 / 3 = 26.666667
    wh.addItem(Item("Container", 0, 20.00));
    Report zeroQtyReport(wh);
    assert(isClose(zeroQtyReport.averageItemValue(), 80.0 / 3.0));

    std::cout << "Test test_Report_averageItemValue: PASS" << std::endl;
}

// Test outOfStockCount method
void test_Report_outOfStockCount() {
    Warehouse wh("Stock Test");
    Report report1(wh);
    assert(report1.outOfStockCount() == 0);

    wh.addItem(Item("InStock1", 5, 10.0));
    wh.addItem(Item("InStock2", 1, 2.0));
    Report report2(wh);
    assert(report2.outOfStockCount() == 0);

    wh.addItem(Item("OutStock1", 0, 15.0));
    Report report3(wh);
    assert(report3.outOfStockCount() == 1);

    wh.addItem(Item("OutStock2", 0, 50.0));
    Report report4(wh);
    assert(report4.outOfStockCount() == 2);

    // Consume all units of an in-stock item and re-verify
    wh.withdraw("InStock2", 1);
    Report report5(wh);
    assert(report5.outOfStockCount() == 3);

    std::cout << "Test test_Report_outOfStockCount: PASS" << std::endl;
}

int main() {
    test_Report_defaultConstructor();
    test_Report_parameterizedConstructor();
    test_Report_isEmpty();
    test_Report_summaryLine();
    test_Report_averageItemValue();
    test_Report_outOfStockCount();

    std::cout << "\nAll Report unit tests passed successfully!" << std::endl;
    return 0;
}