#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include "Report.h"
#include "Warehouse.h"

// Helper function for floating-point comparison
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor and initial state
void test_Report_defaultConstructor() {
    Report report;
    
    assert(report.isEmpty() == true);
    assert(report.averageItemValue() == 0.0);
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");
    
    std::cout << "Test test_Report_defaultConstructor: PASS" << std::endl;
}

// Test parameterized constructor with an empty warehouse
void test_Report_constructor_emptyWarehouse() {
    Warehouse warehouse;
    warehouse.label = "Central Depot";
    
    Report report(warehouse);
    
    assert(report.isEmpty() == true);
    assert(report.averageItemValue() == 0.0);
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty"); // isEmpty() is true, so returns "empty"
    
    std::cout << "Test test_Report_constructor_emptyWarehouse: PASS" << std::endl;
}

// Test parameterized constructor with a non-empty warehouse containing in-stock items
void test_Report_constructor_populatedWarehouse() {
    Warehouse warehouse;
    warehouse.label = "North Distribution Center";
    
    // Assumes Item constructor/methods compatible with standard Warehouse implementation
    Item item1;
    item1.price = 10.0;
    item1.quantity = 5;
    
    Item item2;
    item2.price = 20.0;
    item2.quantity = 2;

    warehouse.addItem(item1);
    warehouse.addItem(item2);
    
    Report report(warehouse);
    
    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "North Distribution Center");
    
    std::cout << "Test test_Report_constructor_populatedWarehouse: PASS" << std::endl;
}

// Test calculation of average item value
void test_Report_averageItemValue() {
    Warehouse warehouse;
    warehouse.label = "Electronics Annex";
    
    // Empty warehouse check
    Report emptyReport(warehouse);
    assert(areAlmostEqual(emptyReport.averageItemValue(), 0.0));
    
    // Add items:
    // Item 1: price 15.00, quantity 2 -> value 30.00
    Item item1;
    item1.price = 15.00;
    item1.quantity = 2;
    warehouse.addItem(item1);
    
    // Item 2: price 45.00, quantity 1 -> value 45.00
    Item item2;
    item2.price = 45.00;
    item2.quantity = 1;
    warehouse.addItem(item2);
    
    // Total itemCount = 2 (or total units depending on Warehouse itemCount implementation;
    // averageItemValue computes inventoryValue() / itemCount())
    Report report(warehouse);
    double expectedAverage = warehouse.inventoryValue() / static_cast<double>(warehouse.itemCount());
    assert(areAlmostEqual(report.averageItemValue(), expectedAverage));
    
    std::cout << "Test test_Report_averageItemValue: PASS" << std::endl;
}

// Test out-of-stock item counting functionality
void test_Report_outOfStockCount() {
    Warehouse warehouse;
    warehouse.label = "Grocery Hub";
    
    // Item in stock
    Item item1;
    item1.quantity = 10;
    warehouse.addItem(item1);
    
    // Item out of stock (quantity = 0)
    Item item2;
    item2.quantity = 0;
    warehouse.addItem(item2);
    
    // Another item out of stock
    Item item3;
    item3.quantity = 0;
    warehouse.addItem(item3);
    
    Report report(warehouse);
    
    assert(report.outOfStockCount() == 2);
    
    std::cout << "Test test_Report_outOfStockCount: PASS" << std::endl;
}

// Test summaryLine under different states
void test_Report_summaryLine() {
    Warehouse warehouse;
    warehouse.label = "Specialty Goods";
    
    Report emptyReport(warehouse);
    assert(emptyReport.summaryLine() == "empty");
    
    Item item;
    item.quantity = 1;
    item.price = 5.50;
    warehouse.addItem(item);
    
    Report populatedReport(warehouse);
    assert(populatedReport.summaryLine() == "Specialty Goods");
    
    std::cout << "Test test_Report_summaryLine: PASS" << std::endl;
}

// Test isEmpty method
void test_Report_isEmpty() {
    Warehouse warehouse;
    Report report(warehouse);
    assert(report.isEmpty() == true);
    
    Item item;
    item.quantity = 1;
    warehouse.addItem(item);
    
    Report populatedReport(warehouse);
    assert(populatedReport.isEmpty() == false);
    
    std::cout << "Test test_Report_isEmpty: PASS" << std::endl;
}

int main() {
    std::cout << "--- Starting Report Unit Tests ---" << std::endl;
    
    test_Report_defaultConstructor();
    test_Report_constructor_emptyWarehouse();
    test_Report_constructor_populatedWarehouse();
    test_Report_averageItemValue();
    test_Report_outOfStockCount();
    test_Report_summaryLine();
    test_Report_isEmpty();
    
    std::cout << "--- All Report Unit Tests Passed Successfully ---" << std::endl;
    return 0;
}