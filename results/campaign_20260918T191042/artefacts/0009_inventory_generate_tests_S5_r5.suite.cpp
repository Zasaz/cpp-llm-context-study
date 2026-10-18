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
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_parameterizedConstructor_emptyWarehouse() {
    Warehouse wh("Main Warehouse");
    Report report(wh);

    assert(report.isEmpty() == true);
    assert(report.source.label == "Main Warehouse");
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_parameterizedConstructor_emptyWarehouse: PASS" << std::endl;
}

void test_averageItemValue_withItems() {
    Warehouse wh("Electronics");
    // Item 1: 10 * 50.0 = 500.0
    wh.addItem(Item("Widget", 10, 50.0));
    // Item 2: 5 * 100.0 = 500.0
    wh.addItem(Item("Gadget", 5, 100.0));
    // Total Inventory Value = 1000.0; Item Count = 2
    // Average = 1000.0 / 2 = 500.0

    Report report(wh);

    assert(!report.isEmpty());
    assert(isClose(report.averageItemValue(), 500.0));

    // Add another item with 0 quantity
    // Item 3: 0 * 20.0 = 0.0
    // Total Inventory Value = 1000.0; Item Count = 3
    // Average = 1000.0 / 3 = 333.333333
    wh.addItem(Item("Doohickey", 0, 20.0));
    Report report2(wh);

    assert(isClose(report2.averageItemValue(), 1000.0 / 3.0));

    std::cout << "Test test_averageItemValue_withItems: PASS" << std::endl;
}

void test_outOfStockCount() {
    Warehouse wh("Supplies");
    
    // Add in-stock items
    wh.addItem(Item("Paper", 100, 0.05));
    wh.addItem(Item("Pens", 50, 1.50));
    
    Report report1(wh);
    assert(report1.outOfStockCount() == 0);

    // Add out-of-stock items (quantity <= 0)
    wh.addItem(Item("Stapler", 0, 12.0));
    wh.addItem(Item("Eraser", 0, 0.50));

    Report report2(wh);
    assert(report2.outOfStockCount() == 2);

    // Add another in-stock item
    wh.addItem(Item("Markers", 15, 2.0));

    Report report3(wh);
    assert(report3.outOfStockCount() == 2);

    std::cout << "Test test_outOfStockCount: PASS" << std::endl;
}

void test_isEmpty() {
    Warehouse wh("Storage");
    Report reportEmpty(wh);
    assert(reportEmpty.isEmpty() == true);

    wh.addItem(Item("Box", 1, 2.5));
    Report reportNonEmpty(wh);
    assert(reportNonEmpty.isEmpty() == false);

    std::cout << "Test test_isEmpty: PASS" << std::endl;
}

void test_summaryLine() {
    Warehouse emptyWh("Logistics Hub A");
    Report reportEmpty(emptyWh);
    // When warehouse is empty, summaryLine() returns "empty" regardless of label
    assert(reportEmpty.summaryLine() == "empty");

    Warehouse populatedWh("Logistics Hub B");
    populatedWh.addItem(Item("Pallet", 20, 15.0));
    Report reportPopulated(populatedWh);
    // When warehouse is not empty, summaryLine() returns warehouse label
    assert(reportPopulated.summaryLine() == "Logistics Hub B");

    std::cout << "Test test_summaryLine: PASS" << std::endl;
}

void test_reportReflectsWarehouseStateAtConstruction() {
    Warehouse wh("Dynamic Hub");
    wh.addItem(Item("Item1", 10, 10.0));
    
    Report report(wh);
    assert(report.source.itemCount() == 1);
    assert(isClose(report.averageItemValue(), 100.0));

    // Modifying warehouse afterwards should not affect the created report (copied by value)
    wh.addItem(Item("Item2", 10, 10.0));
    assert(report.source.itemCount() == 1);
    assert(isClose(report.averageItemValue(), 100.0));

    // Direct mutation on report's internal source
    report.source.addItem(Item("Item3", 10, 10.0));
    assert(report.source.itemCount() == 2);
    assert(isClose(report.averageItemValue(), 100.0));

    std::cout << "Test test_reportReflectsWarehouseStateAtConstruction: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Report class unit tests..." << std::endl;

    test_defaultConstructor();
    test_parameterizedConstructor_emptyWarehouse();
    test_averageItemValue_withItems();
    test_outOfStockCount();
    test_isEmpty();
    test_summaryLine();
    test_reportReflectsWarehouseStateAtConstruction();

    std::cout << "All Report unit tests completed successfully!" << std::endl;
    return 0;
}