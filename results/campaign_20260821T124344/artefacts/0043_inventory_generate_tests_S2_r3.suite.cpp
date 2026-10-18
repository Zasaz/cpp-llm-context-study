#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

// Include project headers
#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor and its default values
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
    // Assuming wh has a default or modified state that is empty
    Report report(wh);

    assert(report.isEmpty() == true);
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_Report_emptyWarehouse: PASS" << std::endl;
}

// Test average item value calculation with non-empty warehouse
void test_Report_averageItemValue() {
    Warehouse wh;
    wh.label = "Electronics Hub";
    
    // Add items (assuming standard Item interface or push_back/addItem)
    Item item1("Laptop", 1000.0, 5);
    Item item2("Mouse", 50.0, 10);
    Item item3("Keyboard", 100.0, 5);

    wh.addItem(item1);
    wh.addItem(item2);
    wh.addItem(item3);

    Report report(wh);

    assert(report.isEmpty() == false);
    
    // Calculate expected average: total inventory value / total item count
    double expectedAvg = wh.inventoryValue() / static_cast<double>(wh.itemCount());
    assert(isClose(report.averageItemValue(), expectedAvg));

    std::cout << "Test test_Report_averageItemValue: PASS" << std::endl;
}

// Test counting out of stock items
void test_Report_outOfStockCount() {
    Warehouse wh;
    wh.label = "Storage A";

    // Item(name, price, quantity) where quantity 0 means out of stock
    Item inStock1("Monitor", 300.0, 2);
    Item outStock1("Cable", 15.0, 0);
    Item inStock2("Desk", 250.0, 1);
    Item outStock2("Chair", 120.0, 0);
    Item outStock3("Adapter", 25.0, 0);

    wh.addItem(inStock1);
    wh.addItem(outStock1);
    wh.addItem(inStock2);
    wh.addItem(outStock2);
    wh.addItem(outStock3);

    Report report(wh);

    assert(report.outOfStockCount() == 3);

    std::cout << "Test test_Report_outOfStockCount: PASS" << std::endl;
}

// Test summaryLine functionality for empty vs non-empty warehouses
void test_Report_summaryLine() {
    // Empty warehouse
    Warehouse emptyWh;
    emptyWh.label = "Empty Facility";
    Report emptyReport(emptyWh);
    // Since it's empty, summaryLine() should return "empty"
    assert(emptyReport.summaryLine() == "empty");

    // Non-empty warehouse
    Warehouse wh;
    wh.label = "Main Distribution Center";
    Item item("Box", 5.0, 100);
    wh.addItem(item);

    Report report(wh);
    assert(report.summaryLine() == "Main Distribution Center");

    std::cout << "Test test_Report_summaryLine: PASS" << std::endl;
}

int main() {
    std::cout << "Running Report test suite..." << std::endl;

    test_Report_defaultConstructor();
    test_Report_emptyWarehouse();
    test_Report_averageItemValue();
    test_Report_outOfStockCount();
    test_Report_summaryLine();

    std::cout << "All Report tests passed successfully!" << std::endl;
    return 0;
}