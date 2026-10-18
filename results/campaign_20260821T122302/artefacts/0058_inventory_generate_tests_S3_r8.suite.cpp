#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

void test_defaultConstructor() {
    Report report;
    
    assert(report.isEmpty());
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");
    assert(report.source.label == "unnamed");
    assert(report.source.itemCount() == 0);

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_parameterizedConstructor_emptyWarehouse() {
    Warehouse warehouse("Central Depot");
    Report report(warehouse);

    assert(report.isEmpty());
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");
    assert(report.source.label == "Central Depot");

    std::cout << "Test test_parameterizedConstructor_emptyWarehouse: PASS" << std::endl;
}

void test_averageItemValue_populated() {
    Warehouse warehouse("Main Warehouse");
    // Item 1: 10 units @ $2.50 = $25.00
    warehouse.addItem(Item("Widget", 10, 2.50));
    // Item 2: 5 units @ $10.00 = $50.00
    warehouse.addItem(Item("Gadget", 5, 10.00));
    // Item 3: 20 units @ $1.00 = $20.00
    warehouse.addItem(Item("Doohickey", 20, 1.00));

    Report report(warehouse);

    // Total value = 25 + 50 + 20 = 95.00
    // Total item kinds = 3
    // Average value = 95.00 / 3 = 31.666667
    assert(!report.isEmpty());
    assert(isClose(report.averageItemValue(), 95.0 / 3.0));

    std::cout << "Test test_averageItemValue_populated: PASS" << std::endl;
}

void test_averageItemValue_zeroValueItems() {
    Warehouse warehouse("Zero Value Depot");
    warehouse.addItem(Item("FreeSample", 100, 0.0));
    warehouse.addItem(Item("EmptyBox", 0, 5.0));

    Report report(warehouse);

    // Total value = 0.0 + 0.0 = 0.0
    // Item count = 2
    // Average = 0.0 / 2 = 0.0
    assert(!report.isEmpty());
    assert(isClose(report.averageItemValue(), 0.0));

    std::cout << "Test test_averageItemValue_zeroValueItems: PASS" << std::endl;
}

void test_outOfStockCount_mixedStock() {
    Warehouse warehouse("Stock Facility");
    warehouse.addItem(Item("InStock1", 10, 5.00));   // in stock (qty > 0)
    warehouse.addItem(Item("OutOfStock1", 0, 2.00)); // out of stock
    warehouse.addItem(Item("InStock2", 1, 100.00));  // in stock
    warehouse.addItem(Item("OutOfStock2", 0, 0.50)); // out of stock
    warehouse.addItem(Item("OutOfStock3", 0, 15.00));// out of stock

    Report report(warehouse);

    assert(report.outOfStockCount() == 3);

    std::cout << "Test test_outOfStockCount_mixedStock: PASS" << std::endl;
}

void test_outOfStockCount_allInStock() {
    Warehouse warehouse("Fully Stocked");
    warehouse.addItem(Item("ItemA", 5, 1.00));
    warehouse.addItem(Item("ItemB", 12, 3.50));

    Report report(warehouse);

    assert(report.outOfStockCount() == 0);

    std::cout << "Test test_outOfStockCount_allInStock: PASS" << std::endl;
}

void test_outOfStockCount_allOutOfStock() {
    Warehouse warehouse("Empty Stock");
    warehouse.addItem(Item("ItemA", 0, 1.00));
    warehouse.addItem(Item("ItemB", 0, 3.50));

    Report report(warehouse);

    assert(report.outOfStockCount() == 2);

    std::cout << "Test test_outOfStockCount_allOutOfStock: PASS" << std::endl;
}

void test_summaryLine_and_isEmpty() {
    Warehouse emptyWarehouse("North Hub");
    Report emptyReport(emptyWarehouse);
    assert(emptyReport.isEmpty());
    assert(emptyReport.summaryLine() == "empty");

    Warehouse activeWarehouse("South Hub");
    activeWarehouse.addItem(Item("Pallet", 4, 25.0));
    Report activeReport(activeWarehouse);
    assert(!activeReport.isEmpty());
    assert(activeReport.summaryLine() == "South Hub");

    std::cout << "Test test_summaryLine_and_isEmpty: PASS" << std::endl;
}

void test_report_independenceFromWarehouseModifications() {
    Warehouse warehouse("Original Warehouse");
    warehouse.addItem(Item("Bolts", 100, 0.10)); // value: 10.00

    Report report(warehouse);

    // Modify original warehouse after report creation
    warehouse.addItem(Item("Nuts", 50, 0.20));
    warehouse.withdraw("Bolts", 50);

    // Report should retain its own copy of the state at construction
    assert(report.source.itemCount() == 1);
    assert(isClose(report.averageItemValue(), 10.00));
    assert(report.outOfStockCount() == 0);

    std::cout << "Test test_report_independenceFromWarehouseModifications: PASS" << std::endl;
}

int main() {
    std::cout << "Running Report test suite..." << std::endl;

    test_defaultConstructor();
    test_parameterizedConstructor_emptyWarehouse();
    test_averageItemValue_populated();
    test_averageItemValue_zeroValueItems();
    test_outOfStockCount_mixedStock();
    test_outOfStockCount_allInStock();
    test_outOfStockCount_allOutOfStock();
    test_summaryLine_and_isEmpty();
    test_report_independenceFromWarehouseModifications();

    std::cout << "All Report tests passed successfully!" << std::endl;
    return 0;
}