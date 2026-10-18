#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function for floating point comparison
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_defaultConstructor_initialState() {
    Report report;

    assert(report.isEmpty() == true);
    assert(areAlmostEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_defaultConstructor_initialState: PASS" << std::endl;
}

void test_parameterizedConstructor_emptyWarehouse() {
    Warehouse warehouse;
    warehouse.label = "Central Hub";
    
    Report report(warehouse);

    assert(report.isEmpty() == true);
    assert(areAlmostEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty"); // isEmpty() is true, so summaryLine returns "empty"

    std::cout << "Test test_parameterizedConstructor_emptyWarehouse: PASS" << std::endl;
}

void test_averageItemValue_populatedWarehouse() {
    Warehouse warehouse;
    warehouse.label = "North Warehouse";

    // Adding items to the warehouse
    Item item1;
    item1.price = 10.0;
    item1.quantity = 2; // Value = 20.0
    warehouse.addItem(item1);

    Item item2;
    item2.price = 25.0;
    item2.quantity = 4; // Value = 100.0
    warehouse.addItem(item2);

    Report report(warehouse);

    assert(report.isEmpty() == false);
    
    // Total count = 6, Total value = 120.0, Average = 20.0
    double expectedAverage = 120.0 / 6.0;
    assert(areAlmostEqual(report.averageItemValue(), expectedAverage));

    std::cout << "Test test_averageItemValue_populatedWarehouse: PASS" << std::endl;
}

void test_outOfStockCount_variousStockLevels() {
    Warehouse warehouse;
    warehouse.label = "South Warehouse";

    // In stock item
    Item item1;
    item1.quantity = 5;
    warehouse.addItem(item1);

    // Out of stock item (quantity = 0)
    Item item2;
    item2.quantity = 0;
    warehouse.addItem(item2);

    // In stock item
    Item item3;
    item3.quantity = 12;
    warehouse.addItem(item3);

    // Another out of stock item
    Item item4;
    item4.quantity = 0;
    warehouse.addItem(item4);

    Report report(warehouse);

    assert(report.outOfStockCount() == 2);

    std::cout << "Test test_outOfStockCount_variousStockLevels: PASS" << std::endl;
}

void test_summaryLine_nonEmptyWarehouse() {
    Warehouse warehouse;
    warehouse.label = "East Warehouse Distribution";

    Item item;
    item.quantity = 1;
    item.price = 15.50;
    warehouse.addItem(item);

    Report report(warehouse);

    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "East Warehouse Distribution");

    std::cout << "Test test_summaryLine_nonEmptyWarehouse: PASS" << std::endl;
}

int main() {
    std::cout << "Running Report test suite..." << std::endl;

    test_defaultConstructor_initialState();
    test_parameterizedConstructor_emptyWarehouse();
    test_averageItemValue_populatedWarehouse();
    test_outOfStockCount_variousStockLevels();
    test_summaryLine_nonEmptyWarehouse();

    std::cout << "All Report tests passed successfully!" << std::endl;
    return 0;
}