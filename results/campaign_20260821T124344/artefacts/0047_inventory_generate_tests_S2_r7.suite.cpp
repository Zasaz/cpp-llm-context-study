#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor and its default state
void test_Report_defaultConstructor() {
    Report report;

    assert(report.isEmpty() == true);
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_Report_defaultConstructor: PASS" << std::endl;
}

// Test parameterized constructor with an empty warehouse
void test_Report_emptyWarehouse() {
    Warehouse wh;
    // Assuming warehouse might have an empty label or default state
    wh.label = "Main Warehouse";
    
    Report report(wh);

    assert(report.isEmpty() == true);
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_Report_emptyWarehouse: PASS" << std::endl;
}

// Test average item value calculation with multiple items
void test_Report_averageItemValue() {
    Warehouse wh;
    wh.label = "Distribution Center";

    // Adding items to the warehouse
    Item item1;
    item1.price = 10.0;
    item1.quantity = 2; // value = 20.0

    Item item2;
    item2.price = 25.0;
    item2.quantity = 4; // value = 100.0

    Item item3;
    item3.price = 5.0;
    item3.quantity = 6; // value = 30.0

    wh.items.push_back(item1);
    wh.items.push_back(item2);
    wh.items.push_back(item3);

    // Total count = 3 items (or total quantity depending on itemCount implementation)
    // If itemCount() returns items.size() -> 3
    // inventoryValue() -> 20 + 100 + 30 = 150
    // Expected avg = 150 / 3 = 50.0 (or 150 / 12 = 12.5)
    double expectedAvg = wh.itemCount() > 0 ? (wh.inventoryValue() / static_cast<double>(wh.itemCount())) : 0.0;

    Report report(wh);

    assert(!report.isEmpty());
    assert(isClose(report.averageItemValue(), expectedAvg));

    std::cout << "Test test_Report_averageItemValue: PASS" << std::endl;
}

// Test out of stock count logic
void test_Report_outOfStockCount() {
    Warehouse wh;
    wh.label = "Stockroom";

    Item inStock1;
    inStock1.quantity = 5;

    Item inStock2;
    inStock2.quantity = 1;

    Item outOfStock1;
    outOfStock1.quantity = 0;

    Item outOfStock2;
    outOfStock2.quantity = 0;

    wh.items.push_back(inStock1);
    wh.items.push_back(outOfStock1);
    wh.items.push_back(inStock2);
    wh.items.push_back(outOfStock2);

    Report report(wh);

    assert(report.outOfStockCount() == 2);

    std::cout << "Test test_Report_outOfStockCount: PASS" << std::endl;
}

// Test summaryLine logic when warehouse is non-empty vs empty
void test_Report_summaryLine() {
    Warehouse emptyWh;
    emptyWh.label = "Empty Storage";
    Report emptyReport(emptyWh);
    assert(emptyReport.summaryLine() == "empty");

    Warehouse populatedWh;
    populatedWh.label = "North Hub";
    Item item;
    item.quantity = 10;
    item.price = 15.0;
    populatedWh.items.push_back(item);

    Report populatedReport(populatedWh);
    assert(populatedReport.summaryLine() == "North Hub");

    std::cout << "Test test_Report_summaryLine: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Report test suite..." << std::endl;

    test_Report_defaultConstructor();
    test_Report_emptyWarehouse();
    test_Report_averageItemValue();
    test_Report_outOfStockCount();
    test_Report_summaryLine();

    std::cout << "All Report tests passed successfully!" << std::endl;
    return 0;
}