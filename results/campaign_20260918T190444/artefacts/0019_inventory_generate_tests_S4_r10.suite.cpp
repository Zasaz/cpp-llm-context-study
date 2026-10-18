#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"

// Helper function for floating-point comparison
bool areDoublesEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_defaultConstructor() {
    Report report;
    assert(report.isEmpty() == true);
    assert(areDoublesEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");
    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_paramConstructor_emptyWarehouse() {
    Warehouse warehouse;
    warehouse.label = "North Warehouse";

    Report report(warehouse);
    assert(report.isEmpty() == true);
    assert(areDoublesEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");
    std::cout << "Test test_paramConstructor_emptyWarehouse: PASS" << std::endl;
}

void test_averageItemValue_withItems() {
    Warehouse warehouse;
    warehouse.label = "Central Hub";

    // Adding items to the warehouse
    // Assuming standard Warehouse item addition/mutation methods or members based on Report.cpp usage
    Item item1("Widget", 10.0, 5);  // price: 10, count: 5 -> value: 50
    Item item2("Gadget", 20.0, 2);  // price: 20, count: 2 -> value: 40
    warehouse.addItem(item1);
    warehouse.addItem(item2);

    Report report(warehouse);

    assert(!report.isEmpty());
    // Total count = 2 items (or distinct item types depending on itemCount definition), total value = 90.0
    // Verify average calculation matches inventoryValue() / itemCount()
    double expectedAvg = warehouse.inventoryValue() / static_cast<double>(warehouse.itemCount());
    assert(areDoublesEqual(report.averageItemValue(), expectedAvg));
    assert(areDoublesEqual(report.averageItemValue(), 45.0));

    std::cout << "Test test_averageItemValue_withItems: PASS" << std::endl;
}

void test_outOfStockCount() {
    Warehouse warehouse;
    warehouse.label = "East Distribution";

    Item inStock1("Laptop", 1000.0, 3);
    Item outOfStock1("Keyboard", 50.0, 0);
    Item inStock2("Monitor", 200.0, 1);
    Item outOfStock2("Mouse", 25.0, 0);

    warehouse.addItem(inStock1);
    warehouse.addItem(outOfStock1);
    warehouse.addItem(inStock2);
    warehouse.addItem(outOfStock2);

    Report report(warehouse);

    assert(report.outOfStockCount() == 2);
    std::cout << "Test test_outOfStockCount: PASS" << std::endl;
}

void test_summaryLine_nonEmptyWarehouse() {
    Warehouse warehouse;
    warehouse.label = "South Warehouse";

    Item item("Desk", 150.0, 4);
    warehouse.addItem(item);

    Report report(warehouse);

    assert(!report.isEmpty());
    assert(report.summaryLine() == "South Warehouse");
    std::cout << "Test test_summaryLine_nonEmptyWarehouse: PASS" << std::endl;
}

void test_isEmpty() {
    Warehouse emptyWarehouse;
    Report emptyReport(emptyWarehouse);
    assert(emptyReport.isEmpty() == true);

    Warehouse nonEmptyWarehouse;
    Item item("Chair", 75.0, 10);
    nonEmptyWarehouse.addItem(item);
    Report nonEmptyReport(nonEmptyWarehouse);
    assert(nonEmptyReport.isEmpty() == false);

    std::cout << "Test test_isEmpty: PASS" << std::endl;
}

int main() {
    std::cout << "Running Report test suite..." << std::endl;

    test_defaultConstructor();
    test_paramConstructor_emptyWarehouse();
    test_averageItemValue_withItems();
    test_outOfStockCount();
    test_summaryLine_nonEmptyWarehouse();
    test_isEmpty();

    std::cout << "All Report tests passed successfully!" << std::endl;
    return 0;
}