#include <iostream>
#include <cassert>
#include <cmath>
#include <string>

#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function to compare floating point values
bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_defaultConstructor() {
    Report report;
    
    // Default warehouse is "unnamed" with 0 items
    assert(report.isEmpty() == true);
    assert(report.summaryLine() == "empty");
    assert(areClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_parameterizedConstructor() {
    Warehouse wh("Main Hub");
    wh.addItem(Item("Widget", 10, 2.5));
    wh.addItem(Item("Gadget", 5, 10.0));

    Report report(wh);
    assert(report.isEmpty() == false);
    assert(report.source.label == "Main Hub");
    assert(report.source.itemCount() == 2);

    std::cout << "Test test_parameterizedConstructor: PASS" << std::endl;
}

void test_isEmpty() {
    Warehouse emptyWh("Empty Depo");
    Report emptyReport(emptyWh);
    assert(emptyReport.isEmpty() == true);

    Warehouse nonEmptyWh("Active Depo");
    nonEmptyWh.addItem(Item("Screws", 100, 0.05));
    Report activeReport(nonEmptyWh);
    assert(activeReport.isEmpty() == false);

    std::cout << "Test test_isEmpty: PASS" << std::endl;
}

void test_summaryLine() {
    // When warehouse is empty, summaryLine must return "empty" regardless of label
    Warehouse emptyWh("North Wing");
    Report emptyReport(emptyWh);
    assert(emptyReport.summaryLine() == "empty");

    // When warehouse has items, summaryLine returns the warehouse label
    Warehouse populatedWh("Central Storage");
    populatedWh.addItem(Item("Box", 1, 5.0));
    Report populatedReport(populatedWh);
    assert(populatedReport.summaryLine() == "Central Storage");

    std::cout << "Test test_summaryLine: PASS" << std::endl;
}

void test_averageItemValue_emptyWarehouse() {
    Warehouse wh("Empty Storage");
    Report report(wh);

    assert(areClose(report.averageItemValue(), 0.0));

    std::cout << "Test test_averageItemValue_emptyWarehouse: PASS" << std::endl;
}

void test_averageItemValue_withItems() {
    Warehouse wh("Electronics");
    // Item 1: 10 * 5.00 = 50.00
    wh.addItem(Item("Resistor Pack", 10, 5.0));
    // Item 2: 2 * 25.00 = 50.00
    wh.addItem(Item("Multimeter", 2, 25.0));
    // Item 3: 0 * 100.00 = 0.00 (out of stock item)
    wh.addItem(Item("Oscilloscope", 0, 100.0));

    // Total inventory value = 50 + 50 + 0 = 100.0
    // Total item kinds count = 3
    // Expected average value per item kind = 100.0 / 3.0 ≈ 33.333333
    Report report(wh);
    double expectedAverage = 100.0 / 3.0;

    assert(areClose(report.averageItemValue(), expectedAverage));

    std::cout << "Test test_averageItemValue_withItems: PASS" << std::endl;
}

void test_outOfStockCount() {
    Warehouse wh("Hardware");
    wh.addItem(Item("Hammer", 5, 15.0));      // in stock (quantity > 0)
    wh.addItem(Item("Nails", 0, 0.10));       // out of stock (quantity == 0)
    wh.addItem(Item("Wrench", 2, 12.0));      // in stock (quantity > 0)
    wh.addItem(Item("Screwdriver", 0, 8.0));  // out of stock (quantity == 0)
    wh.addItem(Item("Pliers", 0, 9.5));       // out of stock (quantity == 0)

    Report report(wh);
    assert(report.outOfStockCount() == 3);

    // Test with no out-of-stock items
    Warehouse fullWh("Grocery");
    fullWh.addItem(Item("Apple", 10, 0.5));
    fullWh.addItem(Item("Banana", 20, 0.3));
    Report fullReport(fullWh);
    assert(fullReport.outOfStockCount() == 0);

    // Test with empty warehouse
    Warehouse emptyWh("Empty");
    Report emptyReport(emptyWh);
    assert(emptyReport.outOfStockCount() == 0);

    std::cout << "Test test_outOfStockCount: PASS" << std::endl;
}

int main() {
    std::cout << "--- Starting Report Unit Tests ---" << std::endl;

    test_defaultConstructor();
    test_parameterizedConstructor();
    test_isEmpty();
    test_summaryLine();
    test_averageItemValue_emptyWarehouse();
    test_averageItemValue_withItems();
    test_outOfStockCount();

    std::cout << "--- All Report Unit Tests Passed Successfully ---" << std::endl;
    return 0;
}