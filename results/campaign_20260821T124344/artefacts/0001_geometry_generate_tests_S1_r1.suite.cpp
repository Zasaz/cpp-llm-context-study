#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());
    
    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addValidShapes() {
    Canvas canvas;
    
    canvas.add(new Rectangle(4.0, 5.0)); // area: 20
    assert(canvas.count() == 1);
    
    canvas.add(new Circle(2.0)); // area: ~12.56637
    assert(canvas.count() == 2);
    
    canvas.add(new Rectangle(1.0, 2.0)); // area: 2
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_addNullptr() {
    Canvas canvas;
    
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullptr: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    
    assert(areAlmostEqual(canvas.totalArea(), 0.0));

    // Rectangle 3x4 -> area = 12
    canvas.add(new Rectangle(3.0, 4.0));
    assert(areAlmostEqual(canvas.totalArea(), 12.0));

    // Circle r=1 -> area = PI * 1^2 ≈ 3.14159265
    canvas.add(new Circle(1.0));
    double expectedArea = 12.0 + (3.14159265358979323846 * 1.0 * 1.0);
    assert(areAlmostEqual(canvas.totalArea(), expectedArea));

    // Base Shape -> area = 0
    canvas.add(new Shape());
    assert(areAlmostEqual(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    
    // Empty canvas
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    // Single shape
    canvas.add(new Rectangle(2.0, 5.0)); // area = 10.0
    assert(areAlmostEqual(canvas.largestArea(), 10.0));

    // Add smaller shape
    canvas.add(new Rectangle(1.0, 2.0)); // area = 2.0
    assert(areAlmostEqual(canvas.largestArea(), 10.0));

    // Add larger shape
    canvas.add(new Circle(3.0)); // area = PI * 9 ≈ 28.27433
    double expectedLargest = 3.14159265358979323846 * 9.0;
    assert(areAlmostEqual(canvas.largestArea(), expectedLargest));

    // Add another shape that is smaller than the largest
    canvas.add(new Rectangle(5.0, 5.0)); // area = 25.0
    assert(areAlmostEqual(canvas.largestArea(), expectedLargest));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;

    canvas.add(new Rectangle(2.0, 3.0));
    canvas.add(new Circle(1.0));
    canvas.add(new Rectangle(10.0, 10.0));
    assert(canvas.count() == 3);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    // Ensure we can add shapes again after clear
    canvas.add(new Rectangle(2.0, 2.0));
    assert(canvas.count() == 1);
    assert(areAlmostEqual(canvas.totalArea(), 4.0));
    assert(areAlmostEqual(canvas.largestArea(), 4.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    // Tests that a canvas going out of scope properly clears dynamic memory without error
    {
        Canvas canvas;
        canvas.add(new Rectangle(5.0, 5.0));
        canvas.add(new Circle(3.0));
        canvas.add(new Shape());
    }
    // Canvas destroyed here; if double-free or memory corruption occurs, assert/crash would happen
    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Canvas Test Suite..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullptr();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}