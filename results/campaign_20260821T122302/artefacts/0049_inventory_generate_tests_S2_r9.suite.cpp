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
    assert(report.isEmpty() == true);
    assert(report.averageItemValue() == 0.0);
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_emptyWarehouse() {
    Warehouse warehouse;
    warehouse.label = "Central Hub";

    Report report(warehouse);
    assert(report.isEmpty() == true);
    assert(report.averageItemValue() == 0.0);
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_emptyWarehouse: PASS" << std::endl;
}

void test_summaryLine_nonEmptyWarehouse() {
    Warehouse warehouse;
    warehouse.label = "North Distribution Center";

    Item item1;
    warehouse.items.push_back(item1);

    Report report(warehouse);
    assert(!report.isEmpty());
    assert(report.summaryLine() == "North Distribution Center");

    std::cout << "Test test_summaryLine_nonEmptyWarehouse: PASS" << std::endl;
}

void test_averageItemValue_calculation() {
    Warehouse warehouse;
    warehouse.label = "East Warehouse";

    Item item1;
    Item item2;
    Item item3;

    warehouse.items.push_back(item1);
    warehouse.items.push_back(item2);
    warehouse.items.push_back(item3);

    Report report(warehouse);
    int count = warehouse.itemCount();
    assert(count == 3);

    double expectedAvg = warehouse.inventoryValue() / static_cast<double>(count);
    assert(isClose(report.averageItemValue(), expectedAvg));

    std::cout << "Test test_averageItemValue_calculation: PASS" << std::endl;
}

void test_outOfStockCount_mixedStock() {
    Warehouse warehouse;
    warehouse.label = "South Warehouse";

    // Depending on Item implementation, creating items with different stock states
    Item inStockItem;
    Item outOfStockItem;

    warehouse.items.push_back(inStockItem);
    warehouse.items.push_back(outOfStockItem);

    int manualOutOfStockCount = 0;
    for (const auto& item : warehouse.items) {
        if (!item.isInStock()) {
            manualOutOfStockCount++;
        }
    }

    Report report(warehouse);
    assert(report.outOfStockCount() == manualOutOfStockCount);

    std::cout << "Test test_outOfStockCount_mixedStock: PASS" << std::endl;
}

void test_outOfStockCount_allInOrOutOfStock() {
    Warehouse warehouse;
    warehouse.label = "West Warehouse";

    // Test with empty items list
    Report emptyReport(warehouse);
    assert(emptyReport.outOfStockCount() == 0);

    // Test with multiple items
    for (int i = 0; i < 5; ++i) {
        Item item;
        warehouse.items.push_back(item);
    }

    int expectedOutOfStock = 0;
    for (const auto& item : warehouse.items) {
        if (!item.isInStock()) {
            expectedOutOfStock++;
        }
    }

    Report report(warehouse);
    assert(report.outOfStockCount() == expectedOutOfStock);

    std::cout << "Test test_outOfStockCount_allInOrOutOfStock: PASS" << std::endl;
}

void test_reportSnapshotSemantics() {
    Warehouse warehouse;
    warehouse.label = "Snapshot Test";

    Item item1;
    warehouse.items.push_back(item1);

    Report report(warehouse);
    assert(!report.isEmpty());
    assert(report.summaryLine() == "Snapshot Test");

    // Mutate the original warehouse after creating the report
    warehouse.label = "Modified Name";
    warehouse.items.clear();

    // Report should retain its own copy of the warehouse
    assert(!report.isEmpty());
    assert(report.summaryLine() == "Snapshot Test");

    std::cout << "Test test_reportSnapshotSemantics: PASS" << std::endl;
}

int main() {
    std::cout << "--- Running Report Unit Tests ---" << std::endl;

    test_defaultConstructor();
    test_emptyWarehouse();
    test_summaryLine_nonEmptyWarehouse();
    test_averageItemValue_calculation();
    test_outOfStockCount_mixedStock();
    test_outOfStockCount_allInOrOutOfStock();
    test_reportSnapshotSemantics();

    std::cout << "All Report unit tests passed successfully!" << std::endl;
    return 0;
}