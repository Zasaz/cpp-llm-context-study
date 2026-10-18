#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_addValidShapes() {
    Canvas canvas;
    canvas.add(new Rectangle(4.0, 5.0)); // Area: 20.0
    assert(canvas.count() == 1);

    canvas.add(new Circle(2.0)); // Area: PI * 4.0 ≈ 12.56637
    assert(canvas.count() == 2);

    canvas.add(new Shape()); // Area: 0.0
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    // Expected: 0.0 initially
    assert(isClose(canvas.totalArea(), 0.0));

    // Add rectangle: 3 * 7 = 21.0
    canvas.add(new Rectangle(3.0, 7.0));
    assert(isClose(canvas.totalArea(), 21.0));

    // Add circle: PI * 1^2 = 3.141592653589793
    canvas.add(new Circle(1.0));
    double expectedArea = 21.0 + 3.14159265358979323846;
    assert(isClose(canvas.totalArea(), expectedArea));

    // Add generic base Shape: 0.0
    canvas.add(new Shape());
    assert(isClose(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    // Empty canvas largest area is 0.0
    assert(isClose(canvas.largestArea(), 0.0));

    // Add smaller shape first: Circle with radius 1 => area ~3.14159
    canvas.add(new Circle(1.0));
    assert(isClose(canvas.largestArea(), 3.14159265358979323846));

    // Add larger shape: Rectangle 5 * 10 => area 50.0
    canvas.add(new Rectangle(5.0, 10.0));
    assert(isClose(canvas.largestArea(), 50.0));

    // Add medium shape: Rectangle 4 * 4 => area 16.0
    canvas.add(new Rectangle(4.0, 4.0));
    assert(isClose(canvas.largestArea(), 50.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0));
    canvas.add(new Circle(5.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Should be safe to clear multiple times
    canvas.clear();
    assert(canvas.count() == 0);

    // Should be able to add new shapes after clear
    canvas.add(new Rectangle(2.0, 2.0));
    assert(canvas.count() == 1);
    assert(isClose(canvas.totalArea(), 4.0));
    assert(isClose(canvas.largestArea(), 4.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    // Test that dynamically allocated Canvas properly deletes its shapes without crashing
    Canvas* canvas = new Canvas();
    canvas->add(new Rectangle(10.0, 10.0));
    canvas->add(new Circle(3.0));
    delete canvas; // Invokes ~Canvas() -> clear()

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addValidShapes();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}