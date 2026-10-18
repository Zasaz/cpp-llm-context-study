#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"

// Helper function for floating point comparison
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor and empty state
void test_Report_defaultConstructor() {
    Report r;
    assert(r.isEmpty());
    assert(isClose(r.averageItemValue(), 0.0));
    assert(r.outOfStockCount() == 0);
    assert(r.summaryLine() == "empty");

    std::cout << "Test test_Report_defaultConstructor: PASS" << std::endl;
}

// Test parameterized constructor with an empty warehouse
void test_Report_parameterizedConstructor_emptyWarehouse() {
    Warehouse w;
    Report r(w);

    assert(r.isEmpty());
    assert(isClose(r.averageItemValue(), 0.0));
    assert(r.outOfStockCount() == 0);
    assert(r.summaryLine() == "empty");

    std::cout << "Test test_Report_parameterizedConstructor_emptyWarehouse: PASS" << std::endl;
}

// Test averageItemValue with populated items
void test_Report_averageItemValue() {
    Warehouse w;
    w.label = "Central Depot";

    // Adding items or manipulating warehouse depending on Warehouse/Item structure
    // Since Report uses source.itemCount() and source.inventoryValue() / source.items
    // We populate warehouse items
    Item item1;
    item1.price = 10.0;
    item1.quantity = 2; // Value: 20.0 (or item value depending on implementation)
    
    Item item2;
    item2.price = 30.0;
    item2.quantity = 1;

    w.items.push_back(item1);
    w.items.push_back(item2);

    Report r(w);
    assert(!r.isEmpty());
    
    int count = w.itemCount();
    assert(count > 0);
    double expectedAvg = w.inventoryValue() / static_cast<double>(count);
    assert(isClose(r.averageItemValue(), expectedAvg));

    std::cout << "Test test_Report_averageItemValue: PASS" << std::endl;
}

// Test outOfStockCount with mixed in-stock and out-of-stock items
void test_Report_outOfStockCount() {
    Warehouse w;
    w.label = "Distribution Center";

    Item inStock1;
    inStock1.quantity = 5;

    Item inStock2;
    inStock2.quantity = 1;

    Item outOfStock1;
    outOfStock1.quantity = 0;

    Item outOfStock2;
    outOfStock2.quantity = 0;

    w.items.push_back(inStock1);
    w.items.push_back(outOfStock1);
    w.items.push_back(inStock2);
    w.items.push_back(outOfStock2);

    Report r(w);

    assert(r.outOfStockCount() == 2);

    std::cout << "Test test_Report_outOfStockCount: PASS" << std::endl;
}

// Test summaryLine for non-empty warehouse
void test_Report_summaryLine_nonEmpty() {
    Warehouse w;
    w.label = "North Warehouse #42";
    
    Item item;
    item.quantity = 10;
    item.price = 5.5;
    w.items.push_back(item);

    Report r(w);

    assert(!r.isEmpty());
    assert(r.summaryLine() == "North Warehouse #42");

    std::cout << "Test test_Report_summaryLine_nonEmpty: PASS" << std::endl;
}

// Test modifying source warehouse after report construction
void test_Report_sourceDirectAccess() {
    Report r;
    r.source.label = "Direct Access Depot";
    
    Item item;
    item.quantity = 0;
    r.source.items.push_back(item);

    assert(!r.isEmpty());
    assert(r.outOfStockCount() == 1);
    assert(r.summaryLine() == "Direct Access Depot");

    std::cout << "Test test_Report_sourceDirectAccess: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Report Unit Tests ===" << std::endl;

    test_Report_defaultConstructor();
    test_Report_parameterizedConstructor_emptyWarehouse();
    test_Report_averageItemValue();
    test_Report_outOfStockCount();
    test_Report_summaryLine_nonEmpty();
    test_Report_sourceDirectAccess();

    std::cout << "=== All Report Unit Tests Passed Successfully ===" << std::endl;
    return 0;
}