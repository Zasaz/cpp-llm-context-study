#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"

// Helper function for comparing floating-point values
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_defaultConstructor() {
    Report report;
    
    // Default-constructed report should be empty
    assert(report.isEmpty());
    assert(areAlmostEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_paramConstructor_emptyWarehouse() {
    Warehouse wh; // Assuming default warehouse initialization produces an empty warehouse
    Report report(wh);

    assert(report.isEmpty());
    assert(areAlmostEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_paramConstructor_emptyWarehouse: PASS" << std::endl;
}

void test_paramConstructor_nonEmptyWarehouse() {
    Warehouse wh;
    wh.label = "Main Distribution Center";

    // Assuming Item has typical constructors or field setters (e.g., name, price, quantity/stock)
    // Add items that are in stock
    Item item1;
    item1.price = 10.0;
    item1.quantity = 5; // In stock

    Item item2;
    item2.price = 20.0;
    item2.quantity = 0; // Out of stock

    wh.items.push_back(item1);
    wh.items.push_back(item2);

    Report report(wh);

    assert(!report.isEmpty());
    assert(report.summaryLine() == "Main Distribution Center");
    assert(report.outOfStockCount() == 1);

    // Verify averageItemValue based on warehouse items
    // Inventory value / item count
    double expectedAvg = wh.inventoryValue() / static_cast<double>(wh.itemCount());
    assert(areAlmostEqual(report.averageItemValue(), expectedAvg));

    std::cout << "Test test_paramConstructor_nonEmptyWarehouse: PASS" << std::endl;
}

void test_outOfStockCount_allOutOfStock() {
    Warehouse wh;
    wh.label = "Depleted Warehouse";

    Item item1;
    item1.quantity = 0;
    Item item2;
    item2.quantity = 0;

    wh.items.push_back(item1);
    wh.items.push_back(item2);

    Report report(wh);

    assert(report.outOfStockCount() == 2);

    std::cout << "Test test_outOfStockCount_allOutOfStock: PASS" << std::endl;
}

void test_outOfStockCount_allInStock() {
    Warehouse wh;
    wh.label = "Fully Stocked Warehouse";

    Item item1;
    item1.quantity = 10;
    Item item2;
    item2.quantity = 25;

    wh.items.push_back(item1);
    wh.items.push_back(item2);

    Report report(wh);

    assert(report.outOfStockCount() == 0);

    std::cout << "Test test_outOfStockCount_allInStock: PASS" << std::endl;
}

void test_summaryLine_emptyVsNonEmpty() {
    Warehouse emptyWh;
    Report emptyReport(emptyWh);
    assert(emptyReport.summaryLine() == "empty");

    Warehouse activeWh;
    activeWh.label = "North Warehouse";
    Item item;
    item.quantity = 1;
    activeWh.items.push_back(item);

    Report activeReport(activeWh);
    assert(activeReport.summaryLine() == "North Warehouse");

    std::cout << "Test test_summaryLine_emptyVsNonEmpty: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Report test suite..." << std::endl;

    test_defaultConstructor();
    test_paramConstructor_emptyWarehouse();
    test_paramConstructor_nonEmptyWarehouse();
    test_outOfStockCount_allOutOfStock();
    test_outOfStockCount_allInStock();
    test_summaryLine_emptyVsNonEmpty();

    std::cout << "All Report unit tests passed successfully!" << std::endl;
    return 0;
}