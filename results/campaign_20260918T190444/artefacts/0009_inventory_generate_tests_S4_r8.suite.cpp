#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include <vector>

#include "Report.h"
#include "Warehouse.h"

// Helper to compare floating-point values
bool areDoublesEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// 1. Test default constructor and properties on empty Report
void test_Report_defaultConstructor() {
    Report report;

    assert(report.isEmpty() == true);
    assert(areDoublesEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_Report_defaultConstructor: PASS" << std::endl;
}

// 2. Test parameterized constructor with an empty Warehouse
void test_Report_parameterizedConstructor_emptyWarehouse() {
    Warehouse wh;
    wh.label = "Central Hub";
    // itemCount is 0 because no items are added
    Report report(wh);

    assert(report.isEmpty() == true);
    assert(areDoublesEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    // When empty, summaryLine() returns "empty" regardless of warehouse label
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_Report_parameterizedConstructor_emptyWarehouse: PASS" << std::endl;
}

// 3. Test averageItemValue with populated items
void test_Report_averageItemValue_populated() {
    Warehouse wh;
    wh.label = "North Warehouse";

    // Assuming standard Warehouse/Item population mechanism
    // We add items through Warehouse functions or direct item assignment depending on Warehouse interface
    // Populating warehouse to have items
    Report report;
    report.source.label = "North Warehouse";
    
    // Creating items and adding to warehouse
    Item item1;
    item1.price = 10.0;
    item1.quantity = 2; // value = 20.0
    
    Item item2;
    item2.price = 5.0;
    item2.quantity = 4; // value = 20.0

    report.source.items.push_back(item1);
    report.source.items.push_back(item2);

    // itemCount() should be 2, total inventoryValue() should be 40.0
    // Average item value = 40.0 / 2 = 20.0
    assert(!report.isEmpty());
    assert(areDoublesEqual(report.averageItemValue(), report.source.inventoryValue() / static_cast<double>(report.source.itemCount())));

    std::cout << "Test test_Report_averageItemValue_populated: PASS" << std::endl;
}

// 4. Test outOfStockCount with all in stock, some out of stock, all out of stock
void test_Report_outOfStockCount() {
    Warehouse wh;
    wh.label = "Stock Distribution Center";

    Item item1; // in stock
    item1.quantity = 5;

    Item item2; // out of stock
    item2.quantity = 0;

    Item item3; // out of stock
    item3.quantity = 0;

    Item item4; // in stock
    item4.quantity = 12;

    wh.items.push_back(item1);
    wh.items.push_back(item2);
    wh.items.push_back(item3);
    wh.items.push_back(item4);

    Report report(wh);

    assert(report.outOfStockCount() == 2);

    std::cout << "Test test_Report_outOfStockCount: PASS" << std::endl;
}

// 5. Test summaryLine and isEmpty when non-empty
void test_Report_summaryLine_and_isEmpty_nonEmpty() {
    Warehouse wh;
    wh.label = "East Warehouse Branch";

    Item item1;
    item1.quantity = 10;
    item1.price = 15.50;
    wh.items.push_back(item1);

    Report report(wh);

    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "East Warehouse Branch");

    std::cout << "Test test_Report_summaryLine_and_isEmpty_nonEmpty: PASS" << std::endl;
}

// 6. Test outOfStockCount when all items are in stock
void test_Report_outOfStockCount_allInStock() {
    Warehouse wh;
    wh.label = "Full Stock Warehouse";

    Item item1;
    item1.quantity = 1;

    Item item2;
    item2.quantity = 10;

    wh.items.push_back(item1);
    wh.items.push_back(item2);

    Report report(wh);

    assert(report.outOfStockCount() == 0);

    std::cout << "Test test_Report_outOfStockCount_allInStock: PASS" << std::endl;
}

// 7. Test outOfStockCount when all items are out of stock
void test_Report_outOfStockCount_allOutOfStock() {
    Warehouse wh;
    wh.label = "Depleted Warehouse";

    Item item1;
    item1.quantity = 0;

    Item item2;
    item2.quantity = 0;

    Item item3;
    item3.quantity = 0;

    wh.items.push_back(item1);
    wh.items.push_back(item2);
    wh.items.push_back(item3);

    Report report(wh);

    assert(report.outOfStockCount() == 3);

    std::cout << "Test test_Report_outOfStockCount_allOutOfStock: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Report Unit Tests ===" << std::endl;

    test_Report_defaultConstructor();
    test_Report_parameterizedConstructor_emptyWarehouse();
    test_Report_averageItemValue_populated();
    test_Report_outOfStockCount();
    test_Report_summaryLine_and_isEmpty_nonEmpty();
    test_Report_outOfStockCount_allInStock();
    test_Report_outOfStockCount_allOutOfStock();

    std::cout << "=== All Report tests passed successfully! ===" << std::endl;
    return 0;
}