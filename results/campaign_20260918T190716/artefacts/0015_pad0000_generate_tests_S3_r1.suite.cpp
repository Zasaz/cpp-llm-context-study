#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparison
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test initial state of a newly created Canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding null pointer
void test_Canvas_addNull() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_addNull: PASS" << std::endl;
}

// Test adding valid shapes and verifying count
void test_Canvas_addValidShapes() {
    Canvas canvas;
    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);

    canvas.add(new Shape()); // Base shape with area = 0
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

// Test calculating total area with multiple shapes
void test_Canvas_totalArea() {
    Canvas canvas;
    
    // Rect 1: 4 * 5 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    // Rect 2: 2 * 3 = 6.0
    canvas.add(new Rectangle(2.0, 3.0));
    // Circle: PI * 1^2 approx 3.14159265
    canvas.add(new Circle(1.0));
    // Base shape: 0.0
    canvas.add(new Shape());

    double expectedTotal = 20.0 + 6.0 + (3.14159265358979323846 * 1.0 * 1.0) + 0.0;
    assert(areAlmostEqual(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largestArea computation with various configurations
void test_Canvas_largestArea() {
    Canvas canvas;
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    // Add rectangle with area 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    assert(areAlmostEqual(canvas.largestArea(), 20.0));

    // Add circle with area PI * 3^2 = 28.2743...
    canvas.add(new Circle(3.0));
    double expectedCircleArea = 3.14159265358979323846 * 9.0;
    assert(areAlmostEqual(canvas.largestArea(), expectedCircleArea));

    // Add smaller rectangle (area 10.0), largest should remain unchanged
    canvas.add(new Rectangle(2.0, 5.0));
    assert(areAlmostEqual(canvas.largestArea(), expectedCircleArea));

    // Add larger rectangle (area 50.0), largest should update
    canvas.add(new Rectangle(10.0, 5.0));
    assert(areAlmostEqual(canvas.largestArea(), 50.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clearing shapes from Canvas
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0));
    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    // Verify clear on an already empty canvas doesn't crash
    canvas.clear();
    assert(canvas.count() == 0);

    // Verify we can continue adding shapes after clear
    canvas.add(new Rectangle(1.0, 1.0));
    assert(canvas.count() == 1);
    assert(areAlmostEqual(canvas.totalArea(), 1.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test destructor cleanup (instantiating and destroying on heap and scope)
void test_Canvas_destructor() {
    // Canvas destructor calls clear(), which deletes heap-allocated shapes
    Canvas* canvas = new Canvas();
    canvas->add(new Rectangle(5.0, 5.0));
    canvas->add(new Circle(2.0));
    delete canvas; // Should cleanly free all allocated shapes without crash/leak

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "Running Canvas test suite..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNull();
    test_Canvas_addValidShapes();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}