#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "Item.h"
#include "Warehouse.h"
#include "Report.h"

// Helper function for floating point comparisons
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

void test_defaultConstructor() {
    Report report;

    // A default report should have an empty warehouse
    assert(report.isEmpty() == true);
    assert(report.source.itemCount() == 0);
    assert(approxEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_parameterizedConstructor_emptyWarehouse() {
    Warehouse wh("North Depot");
    Report report(wh);

    assert(report.isEmpty() == true);
    assert(report.source.label == "North Depot");
    assert(approxEqual(report.averageItemValue(), 0.0));
    assert(report.outOfStockCount() == 0);
    // When warehouse is empty, summaryLine() returns "empty" regardless of label
    assert(report.summaryLine() == "empty");

    std::cout << "Test test_parameterizedConstructor_emptyWarehouse: PASS" << std::endl;
}

void test_averageItemValue_calculation() {
    Warehouse wh("Central Logistics");
    
    // Item 1: 10 units @ $5.00 -> totalValue = $50.00
    wh.addItem(Item("Widget", 10, 5.0));
    // Item 2: 5 units @ $20.00 -> totalValue = $100.00
    wh.addItem(Item("Gadget", 5, 20.0));
    // Item 3: 20 units @ $1.50 -> totalValue = $30.00
    wh.addItem(Item("Screw", 20, 1.5));

    // Total inventory value = 50 + 100 + 30 = 180.00
    // Total item types (itemCount) = 3
    // Average item value = 180.00 / 3 = 60.00
    Report report(wh);

    assert(!report.isEmpty());
    assert(report.source.itemCount() == 3);
    assert(approxEqual(report.source.inventoryValue(), 180.0));
    assert(approxEqual(report.averageItemValue(), 60.0));

    std::cout << "Test test_averageItemValue_calculation: PASS" << std::endl;
}

void test_averageItemValue_withZeroQuantitiesAndPrices() {
    Warehouse wh("Discount Warehouse");

    // Item 1: 0 units @ $10.00 -> totalValue = $0.00
    wh.addItem(Item("EmptyBox", 0, 10.0));
    // Item 2: 10 units @ $0.00 -> totalValue = $0.00
    wh.addItem(Item("FreeSample", 10, 0.0));

    Report report(wh);

    assert(report.source.itemCount() == 2);
    assert(approxEqual(report.source.inventoryValue(), 0.0));
    assert(approxEqual(report.averageItemValue(), 0.0));

    std::cout << "Test test_averageItemValue_withZeroQuantitiesAndPrices: PASS" << std::endl;
}

void test_outOfStockCount_mixedStock() {
    Warehouse wh("Main Store");
    
    wh.addItem(Item("Apples", 15, 0.5));    // In stock (15 > 0)
    wh.addItem(Item("Bananas", 0, 0.3));    // Out of stock
    wh.addItem(Item("Cherries", 50, 2.0));  // In stock (50 > 0)
    wh.addItem(Item("Dates", 0, 5.0));      // Out of stock
    wh.addItem(Item("Figs", 0, 3.0));       // Out of stock

    Report report(wh);

    assert(report.source.itemCount() == 5);
    assert(report.outOfStockCount() == 3);

    std::cout << "Test test_outOfStockCount_mixedStock: PASS" << std::endl;
}

void test_outOfStockCount_allInStock_and_allOutOfStock() {
    // Case 1: All items in stock
    Warehouse whAllInStock("Full Warehouse");
    whAllInStock.addItem(Item("ItemA", 10, 1.0));
    whAllInStock.addItem(Item("ItemB", 20, 2.0));
    
    Report reportAllInStock(whAllInStock);
    assert(reportAllInStock.outOfStockCount() == 0);

    // Case 2: All items out of stock
    Warehouse whAllOutOfStock("Empty Shelves");
    whAllOutOfStock.addItem(Item("ItemA", 0, 1.0));
    whAllOutOfStock.addItem(Item("ItemB", 0, 2.0));

    Report reportAllOutOfStock(whAllOutOfStock);
    assert(reportAllOutOfStock.outOfStockCount() == 2);

    std::cout << "Test test_outOfStockCount_allInStock_and_allOutOfStock: PASS" << std::endl;
}

void test_isEmpty_and_summaryLine() {
    Warehouse wh("East Wing Storage");
    Report report(wh);

    // Initially empty
    assert(report.isEmpty() == true);
    assert(report.summaryLine() == "empty");

    // Add an item to source
    report.source.addItem(Item("Tool", 1, 15.0));
    assert(report.isEmpty() == false);
    assert(report.summaryLine() == "East Wing Storage");

    std::cout << "Test test_isEmpty_and_summaryLine: PASS" << std::endl;
}

void test_sourceMutationReflectedInReport() {
    Warehouse wh("Fluctuating Hub");
    wh.addItem(Item("Laptop", 5, 800.0)); // total: 4000.0
    wh.addItem(Item("Mouse", 10, 25.0));  // total: 250.0

    Report report(wh);
    assert(report.source.itemCount() == 2);
    assert(approxEqual(report.averageItemValue(), 2125.0));
    assert(report.outOfStockCount() == 0);

    // Consume all laptops through source warehouse
    bool withdrawn = report.source.withdraw("Laptop", 5);
    assert(withdrawn == true);
    assert(report.outOfStockCount() == 1);
    // New total value = 0 + 250 = 250; itemCount = 2; average = 125.0
    assert(approxEqual(report.averageItemValue(), 125.0));

    // Restock the laptops
    int laptopIdx = report.source.findIndex("Laptop");
    assert(laptopIdx >= 0);
    report.source.items[laptopIdx].restock(2);
    assert(report.outOfStockCount() == 0);
    // New total value = (2 * 800) + 250 = 1850; average = 925.0
    assert(approxEqual(report.averageItemValue(), 925.0));

    std::cout << "Test test_sourceMutationReflectedInReport: PASS" << std::endl;
}

int main() {
    std::cout << "--- Running Report Unit Tests ---" << std::endl;

    test_defaultConstructor();
    test_parameterizedConstructor_emptyWarehouse();
    test_averageItemValue_calculation();
    test_averageItemValue_withZeroQuantitiesAndPrices();
    test_outOfStockCount_mixedStock();
    test_outOfStockCount_allInStock_and_allOutOfStock();
    test_isEmpty_and_summaryLine();
    test_sourceMutationReflectedInReport();

    std::cout << "--- All Report Unit Tests Passed Successfully! ---" << std::endl;
    return 0;
}