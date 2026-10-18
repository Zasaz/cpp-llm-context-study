#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"
#include "Item.h"

// Helper function for floating point comparisons
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor behavior
void test_defaultConstructor() {
    Report report;

    assert(report.isEmpty() == true);
    assert(areAlmostEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

// Test parameterized constructor with an empty warehouse
void test_emptyWarehouse() {
    Warehouse warehouse;
    Report report(warehouse);

    assert(report.isEmpty() == true);
    assert(areAlmostEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_emptyWarehouse: PASS" << std::endl;
}

// Test warehouse with items all in stock
void test_allInStockItems() {
    Warehouse warehouse;
    warehouse.label = "Downtown Warehouse";

    Item item1("Widget", 10.0, 5);  // in stock
    Item item2("Gadget", 20.0, 2);  // in stock

    warehouse.items.push_back(item1);
    warehouse.items.push_back(item2);

    Report report(warehouse);

    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "Downtown Warehouse");
    assert(report.outOfStockCount() == 0);
    
    // Average item value = total inventory value / total item count
    double expectedAvg = warehouse.inventoryValue() / static_cast<double>(warehouse.itemCount());
    assert(areAlmostEqual(report.averageItemValue(), expectedAvg));

    std::cout << "Test test_allInStockItems: PASS" << std::endl;
}

// Test warehouse with out-of-stock items
void test_outOfStockItems() {
    Warehouse warehouse;
    warehouse.label = "North Annex";

    Item inStockItem("Hammer", 15.50, 10);
    Item outStockItem1("Nails", 2.50, 0);
    Item outStockItem2("Screws", 1.25, 0);

    warehouse.items.push_back(inStockItem);
    warehouse.items.push_back(outStockItem1);
    warehouse.items.push_back(outStockItem2);

    Report report(warehouse);

    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "North Annex");
    assert(report.outOfStockCount() == 2);

    std::cout << "Test test_outOfStockItems: PASS" << std::endl;
}

// Test warehouse with all items out of stock
void test_allOutOfStockItems() {
    Warehouse warehouse;
    warehouse.label = "Depleted Depot";

    Item out1("ItemA", 100.0, 0);
    Item out2("ItemB", 50.0, 0);

    warehouse.items.push_back(out1);
    warehouse.items.push_back(out2);

    Report report(warehouse);

    assert(report.isEmpty() == false);
    assert(report.outOfStockCount() == 2);
    assert(report.summaryLine() == "Depleted Depot");

    std::cout << "Test test_allOutOfStockItems: PASS" << std::endl;
}

// Test averageItemValue with varied values
void test_averageItemValueCalculation() {
    Warehouse warehouse;
    warehouse.label = "Electronics Hub";

    Item item1("Laptop", 1000.0, 2);
    Item item2("Mouse", 25.0, 4);

    warehouse.items.push_back(item1);
    warehouse.items.push_back(item2);

    Report report(warehouse);

    double expectedAvg = warehouse.inventoryValue() / static_cast<double>(warehouse.itemCount());
    assert(expectedAvg > 0.0);
    assert(areAlmostEqual(report.averageItemValue(), expectedAvg));

    std::cout << "Test test_averageItemValueCalculation: PASS" << std::endl;
}

// Test summaryLine logic when label is custom vs empty
void test_summaryLine() {
    Warehouse warehouseEmpty;
    Report reportEmpty(warehouseEmpty);
    assert(reportEmpty.summaryLine() == "empty");

    Warehouse warehouseCustom;
    warehouseCustom.label = "Special Storage";
    Item item("Item", 5.0, 1);
    warehouseCustom.items.push_back(item);

    Report reportCustom(warehouseCustom);
    assert(reportCustom.summaryLine() == "Special Storage");

    std::cout << "Test test_summaryLine: PASS" << std::endl;
}

int main() {
    std::cout << "Running Report test suite..." << std::endl;

    test_defaultConstructor();
    test_emptyWarehouse();
    test_allInStockItems();
    test_outOfStockItems();
    test_allOutOfStockItems();
    test_averageItemValueCalculation();
    test_summaryLine();

    std::cout << "All Report tests passed successfully!" << std::endl;
    return 0;
}