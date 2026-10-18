#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Report.h"
#include "Warehouse.h"

// Helper function for floating point comparison
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_defaultConstructor_and_isEmpty() {
    Report report;
    assert(report.isEmpty() == true);
    assert(report.averageItemValue() == 0.0);
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_defaultConstructor_and_isEmpty: PASS" << std::endl;
}

void test_parameterizedConstructor_emptyWarehouse() {
    Warehouse w;
    Report report(w);

    assert(report.isEmpty() == true);
    assert(report.averageItemValue() == 0.0);
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_parameterizedConstructor_emptyWarehouse: PASS" << std::endl;
}

void test_averageItemValue_withItems() {
    Warehouse w;
    w.label = "Electronics Hub";

    // Assuming standard Warehouse item addition / direct manipulation supported by Warehouse struct/class
    // We populate items through standard methods or fields
    Item item1("Laptop", 1000.0, 5);  // value: 1000.0
    Item item2("Mouse", 50.0, 10);    // value: 50.0
    Item item3("Monitor", 200.0, 0);  // value: 200.0

    w.items.push_back(item1);
    w.items.push_back(item2);
    w.items.push_back(item3);

    Report report(w);

    assert(!report.isEmpty());
    // average = total inventoryValue / itemCount
    double expectedAvg = w.inventoryValue() / static_cast<double>(w.itemCount());
    assert(isClose(report.averageItemValue(), expectedAvg));

    std::cout << "Test test_averageItemValue_withItems: PASS" << std::endl;
}

void test_outOfStockCount() {
    Warehouse w;
    w.label = "Grocery Depot";

    Item item1("Apple", 1.0, 10);  // in stock
    Item item2("Banana", 0.5, 0);  // out of stock
    Item item3("Orange", 0.8, -1); // out of stock (quantity <= 0)
    Item item4("Milk", 3.0, 5);    // in stock

    w.items.push_back(item1);
    w.items.push_back(item2);
    w.items.push_back(item3);
    w.items.push_back(item4);

    Report report(w);

    assert(report.outOfStockCount() == 2);

    std::cout << "Test test_outOfStockCount: PASS" << std::endl;
}

void test_summaryLine_nonEmpty() {
    Warehouse w;
    w.label = "Central Storage";

    Item item1("Desk", 150.0, 2);
    w.items.push_back(item1);

    Report report(w);

    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "Central Storage");

    std::cout << "Test test_summaryLine_nonEmpty: PASS" << std::endl;
}

void test_summaryLine_emptyWithLabelSet() {
    // Warehouse has a label but no items, summaryLine should still report "empty"
    Warehouse w;
    w.label = "Empty Facility";

    Report report(w);

    assert(report.isEmpty() == true);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_summaryLine_emptyWithLabelSet: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Report class unit tests..." << std::endl;

    test_defaultConstructor_and_isEmpty();
    test_parameterizedConstructor_emptyWarehouse();
    test_averageItemValue_withItems();
    test_outOfStockCount();
    test_summaryLine_nonEmpty();
    test_summaryLine_emptyWithLabelSet();

    std::cout << "All Report tests passed successfully!" << std::endl;
    return 0;
}