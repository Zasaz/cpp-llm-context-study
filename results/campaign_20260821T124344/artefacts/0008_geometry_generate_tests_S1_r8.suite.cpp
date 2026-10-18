#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// Test initial/empty state of Canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding valid shapes and null pointers
void test_Canvas_add() {
    Canvas canvas;

    // Add valid shape
    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    // Add nullptr (should be ignored)
    canvas.add(nullptr);
    assert(canvas.count() == 1);

    // Add another valid shape
    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_add: PASS" << std::endl;
}

// Test totalArea calculation with multiple shapes
void test_Canvas_totalArea() {
    Canvas canvas;

    canvas.add(new Rectangle(2.0, 3.0)); // area = 6.0
    canvas.add(new Rectangle(4.0, 5.0)); // area = 20.0
    canvas.add(new Circle(1.0));          // area = PI * 1^2 ≈ 3.14159265

    double expectedTotal = 6.0 + 20.0 + (3.14159265358979323846 * 1.0 * 1.0);
    assert(areAlmostEqual(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largestArea calculation with varying shapes
void test_Canvas_largestArea() {
    Canvas canvas;

    // Empty canvas largest area is 0.0
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    // Add smaller shape
    canvas.add(new Rectangle(2.0, 2.0)); // area = 4.0
    assert(areAlmostEqual(canvas.largestArea(), 4.0));

    // Add larger shape
    canvas.add(new Rectangle(5.0, 5.0)); // area = 25.0
    assert(areAlmostEqual(canvas.largestArea(), 25.0));

    // Add intermediate shape
    canvas.add(new Circle(2.0));          // area ≈ 12.566
    assert(areAlmostEqual(canvas.largestArea(), 25.0));

    // Add even larger shape
    canvas.add(new Circle(4.0));          // area = PI * 16 ≈ 50.265
    double expectedLargest = 3.14159265358979323846 * 16.0;
    assert(areAlmostEqual(canvas.largestArea(), expectedLargest));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clear() method
void test_Canvas_clear() {
    Canvas canvas;

    canvas.add(new Rectangle(3.0, 4.0));
    canvas.add(new Circle(2.5));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    // Multiple calls to clear() should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    // Can add shapes again after clearing
    canvas.add(new Rectangle(10.0, 2.0));
    assert(canvas.count() == 1);
    assert(areAlmostEqual(canvas.totalArea(), 20.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test Canvas destructor and dynamic lifecycle
void test_Canvas_lifecycle() {
    // Test that dynamically allocated Canvas properly cleans up on deletion
    Canvas* dynamicCanvas = new Canvas();
    dynamicCanvas->add(new Rectangle(5.0, 5.0));
    dynamicCanvas->add(new Circle(2.0));
    assert(dynamicCanvas->count() == 2);
    delete dynamicCanvas; // Invokes ~Canvas() which calls clear()

    std::cout << "Test test_Canvas_lifecycle: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_add();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_lifecycle();

    std::cout << "=== All Canvas Tests Passed Successfully ===" << std::endl;
    return 0;
}