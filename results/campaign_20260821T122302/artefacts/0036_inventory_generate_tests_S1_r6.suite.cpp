#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

#include "Item.h"
#include "Warehouse.h"
#include "Report.h"

// Helper function for floating point equality checks
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_default_constructor() {
    Report report;
    
    assert(report.isEmpty() == true);
    assert(report.outOfStockCount() == 0);
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_default_constructor: PASS" << std::endl;
}

void test_parameterized_constructor_with_empty_warehouse() {
    Warehouse wh("Central Logistics");
    Report report(wh);

    assert(report.source.label == "Central Logistics");
    assert(report.isEmpty() == true);
    assert(report.outOfStockCount() == 0);
    assert(isClose(report.averageItemValue(), 0.0));
    // Since warehouse is empty, summaryLine should return "empty" regardless of label
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_parameterized_constructor_with_empty_warehouse: PASS" << std::endl;
}

void test_isEmpty() {
    Warehouse wh("North Hub");
    Report report(wh);
    assert(report.isEmpty() == true);

    wh.addItem(Item("Widget", 10, 2.50));
    Report reportWithItems(wh);
    assert(reportWithItems.isEmpty() == false);

    std::cout << "Test test_isEmpty: PASS" << std::endl;
}

void test_summaryLine() {
    // Empty warehouse
    Warehouse emptyWh("East Branch");
    Report emptyReport(emptyWh);
    assert(emptyReport.summaryLine() == "empty");

    // Non-empty warehouse
    Warehouse activeWh("East Branch");
    activeWh.addItem(Item("Gadget", 5, 12.00));
    Report activeReport(activeWh);
    assert(activeReport.summaryLine() == "East Branch");

    std::cout << "Test test_summaryLine: PASS" << std::endl;
}

void test_averageItemValue_empty() {
    Warehouse wh("Empty Storage");
    Report report(wh);

    // Division by zero guard verification
    assert(isClose(report.averageItemValue(), 0.0));

    std::cout << "Test test_averageItemValue_empty: PASS" << std::endl;
}

void test_averageItemValue_with_items() {
    Warehouse wh("Main Warehouse");
    // Item 1: 10 * 5.00 = 50.00
    wh.addItem(Item("ItemA", 10, 5.00));
    // Item 2: 2 * 25.00 = 50.00
    wh.addItem(Item("ItemB", 2, 25.00));
    // Item 3: 0 * 100.00 = 0.00
    wh.addItem(Item("ItemC", 0, 100.00));

    // Total inventory value = 100.00, count of item types = 3
    // Average item value = 100.00 / 3 = 33.333333...
    Report report(wh);
    assert(isClose(report.averageItemValue(), 100.0 / 3.0));

    std::cout << "Test test_averageItemValue_with_items: PASS" << std::endl;
}

void test_outOfStockCount_none_out_of_stock() {
    Warehouse wh("Active Stock");
    wh.addItem(Item("Apple", 10, 0.50));
    wh.addItem(Item("Banana", 5, 0.30));
    wh.addItem(Item("Orange", 20, 0.80));

    Report report(wh);
    assert(report.outOfStockCount() == 0);

    std::cout << "Test test_outOfStockCount_none_out_of_stock: PASS" << std::endl;
}

void test_outOfStockCount_multiple_out_of_stock() {
    Warehouse wh("Mixed Stock");
    wh.addItem(Item("Keyboard", 0, 45.00)); // Out of stock
    wh.addItem(Item("Mouse", 15, 20.00));    // In stock
    wh.addItem(Item("Monitor", 0, 150.00));  // Out of stock
    wh.addItem(Item("Cable", 50, 5.00));     // In stock
    wh.addItem(Item("Adapter", 0, 10.00));   // Out of stock

    Report report(wh);
    assert(report.outOfStockCount() == 3);

    std::cout << "Test test_outOfStockCount_multiple_out_of_stock: PASS" << std::endl;
}

void test_state_mutation_via_source() {
    Report report;
    report.source.label = "South Depot";
    report.source.addItem(Item("Chair", 4, 25.00));

    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "South Depot");
    assert(report.outOfStockCount() == 0);
    assert(isClose(report.averageItemValue(), 100.0));

    // Consume all stock for the item
    bool consumed = report.source.withdraw("Chair", 4);
    assert(consumed == true);
    assert(report.outOfStockCount() == 1);
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.isEmpty() == false); // Still has 1 item registered, even if quantity is 0

    std::cout << "Test test_state_mutation_via_source: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Report Class Unit Tests ===" << std::endl;

    test_default_constructor();
    test_parameterized_constructor_with_empty_warehouse();
    test_isEmpty();
    test_summaryLine();
    test_averageItemValue_empty();
    test_averageItemValue_with_items();
    test_outOfStockCount_none_out_of_stock();
    test_outOfStockCount_multiple_out_of_stock();
    test_state_mutation_via_source();

    std::cout << "=== All Report Tests Passed Successfully! ===" << std::endl;
    return 0;
}