#include <iostream>
#include <cassert>
#include <cmath>
#include <string>
#include "Item.h"
#include "Warehouse.h"
#include "Report.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

void test_defaultConstructor() {
    Report report;
    assert(report.isEmpty() == true);
    assert(report.summaryLine() == "empty");
    assert(isClose(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_parameterizedConstructor() {
    Warehouse wh("NorthHub");
    wh.addItem(Item("Widget", 10, 2.5));
    
    Report report(wh);
    assert(report.isEmpty() == false);
    assert(report.source.label == "NorthHub");
    assert(report.source.itemCount() == 1);
    assert(report.summaryLine() == "NorthHub");

    std::cout << "Test test_parameterizedConstructor: PASS" << std::endl;
}

void test_isEmpty() {
    // Empty warehouse
    Warehouse emptyWh("EmptyWh");
    Report reportEmpty(emptyWh);
    assert(reportEmpty.isEmpty() == true);

    // Warehouse with items
    Warehouse nonEmptyWh("NonEmptyWh");
    nonEmptyWh.addItem(Item("Gadget", 0, 5.0)); // Quantity 0 still counts as an item entry in the list
    Report reportNonEmpty(nonEmptyWh);
    assert(reportNonEmpty.isEmpty() == false);

    std::cout << "Test test_isEmpty: PASS" << std::endl;
}

void test_summaryLine() {
    // Empty report should return "empty"
    Report reportEmpty;
    assert(reportEmpty.summaryLine() == "empty");

    // Warehouse with items should return the warehouse label
    Warehouse wh("CentralWarehouse");
    wh.addItem(Item("Box", 5, 1.2));
    Report report(wh);
    assert(report.summaryLine() == "CentralWarehouse");

    // Warehouse with custom label but no items should still return "empty"
    Warehouse whNoItems("CustomEmpty");
    Report reportNoItems(whNoItems);
    assert(reportNoItems.summaryLine() == "empty");

    std::cout << "Test test_summaryLine: PASS" << std::endl;
}

void test_averageItemValue_empty() {
    Report report;
    assert(isClose(report.averageItemValue(), 0.0));

    Warehouse emptyWh("Empty");
    Report reportWh(emptyWh);
    assert(isClose(reportWh.averageItemValue(), 0.0));

    std::cout << "Test test_averageItemValue_empty: PASS" << std::endl;
}

void test_averageItemValue_nonEmpty() {
    Warehouse wh("Main");
    // Item 1: 10 * 2.0 = 20.0
    wh.addItem(Item("A", 10, 2.0));
    // Item 2: 5 * 6.0 = 30.0
    wh.addItem(Item("B", 5, 6.0));
    // Item 3: 0 * 100.0 = 0.0
    wh.addItem(Item("C", 0, 100.0));
    // Total inventory value: 20.0 + 30.0 + 0.0 = 50.0
    // Total item kinds: 3
    // Expected average item value: 50.0 / 3 = 16.666667

    Report report(wh);
    assert(isClose(report.averageItemValue(), 50.0 / 3.0));

    std::cout << "Test test_averageItemValue_nonEmpty: PASS" << std::endl;
}

void test_outOfStockCount() {
    Warehouse wh("StockTest");
    // No items initially
    Report reportEmpty(wh);
    assert(reportEmpty.outOfStockCount() == 0);

    // Add in-stock items
    wh.addItem(Item("Item1", 10, 1.0));
    wh.addItem(Item("Item2", 1, 5.0));
    Report reportAllInStock(wh);
    assert(reportAllInStock.outOfStockCount() == 0);

    // Add out-of-stock items (quantity <= 0)
    wh.addItem(Item("Item3", 0, 10.0));
    wh.addItem(Item("Item4", 0, 20.0));
    Report reportMixed(wh);
    assert(reportMixed.outOfStockCount() == 2);

    // Withdraw all of Item2 to make it out of stock
    wh.withdraw("Item2", 1);
    Report reportAfterWithdraw(wh);
    assert(reportAfterWithdraw.outOfStockCount() == 3);

    std::cout << "Test test_outOfStockCount: PASS" << std::endl;
}

int main() {
    test_defaultConstructor();
    test_parameterizedConstructor();
    test_isEmpty();
    test_summaryLine();
    test_averageItemValue_empty();
    test_averageItemValue_nonEmpty();
    test_outOfStockCount();

    std::cout << "\nAll Report tests passed successfully!" << std::endl;
    return 0;
}