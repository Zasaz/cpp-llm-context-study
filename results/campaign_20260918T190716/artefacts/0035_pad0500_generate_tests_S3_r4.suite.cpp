#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function to compare floating point numbers
bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addNull() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_addNull: PASS" << std::endl;
}

void test_Canvas_addShapesAndCount() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(1.0));
    assert(canvas.count() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);

    // Canvas destructor will free allocated memory
    std::cout << "Test test_Canvas_addShapesAndCount: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    // Rectangle: 4.0 * 5.0 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    // Circle: PI * 2.0^2 ≈ 12.566370614
    canvas.add(new Circle(2.0));
    // Base Shape: area = 0.0
    canvas.add(new Shape());

    double expectedTotal = (4.0 * 5.0) + (3.14159265358979323846 * 2.0 * 2.0) + 0.0;
    assert(areClose(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;

    // Empty canvas largestArea should be 0.0
    assert(areClose(canvas.largestArea(), 0.0));

    // Add rectangle with area 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(areClose(canvas.largestArea(), 10.0));

    // Add circle with area PI * 3^2 ≈ 28.27433
    canvas.add(new Circle(3.0));
    double expectedCircleArea = 3.14159265358979323846 * 9.0;
    assert(areClose(canvas.largestArea(), expectedCircleArea));

    // Add smaller rectangle with area 6.0
    canvas.add(new Rectangle(2.0, 3.0));
    assert(areClose(canvas.largestArea(), expectedCircleArea));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(10.0, 10.0));
    canvas.add(new Circle(5.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    // Calling clear multiple times should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    // Adding again after clear
    canvas.add(new Rectangle(2.0, 2.0));
    assert(canvas.count() == 1);
    assert(areClose(canvas.totalArea(), 4.0));
    assert(areClose(canvas.largestArea(), 4.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNull();
    test_Canvas_addShapesAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();

    std::cout << "=== All Canvas Unit Tests Passed! ===" << std::endl;
    return 0;
}