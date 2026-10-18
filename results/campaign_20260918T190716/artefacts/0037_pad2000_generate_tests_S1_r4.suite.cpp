#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
static bool areClose(double a, double b, double epsilon = 1e-6) {
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

void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(0);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_addShapesAndCount() {
    Canvas canvas;
    canvas.add(new Rectangle(4.0, 5.0)); // Area = 20.0
    assert(canvas.count() == 1);

    canvas.add(new Circle(1.0)); // Area = PI * 1^2 ≈ 3.14159265
    assert(canvas.count() == 2);

    canvas.add(0); // Should not increase count
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_addShapesAndCount: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0)); // Area = 6.0
    canvas.add(new Rectangle(5.0, 2.0)); // Area = 10.0
    canvas.add(new Circle(2.0));         // Area = PI * 4.0 ≈ 12.5663706

    double expectedTotal = 6.0 + 10.0 + (3.14159265358979323846 * 4.0);
    assert(areClose(canvas.totalArea(), expectedTotal));
    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0)); // Area = 6.0
    canvas.add(new Circle(3.0));         // Area = PI * 9.0 ≈ 28.27433
    canvas.add(new Rectangle(4.0, 5.0)); // Area = 20.0

    double expectedLargest = 3.14159265358979323846 * 9.0;
    assert(areClose(canvas.largestArea(), expectedLargest));
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

    // Adding shapes after clearing should work normally
    canvas.add(new Rectangle(3.0, 3.0)); // Area = 9.0
    assert(canvas.count() == 1);
    assert(areClose(canvas.totalArea(), 9.0));
    assert(areClose(canvas.largestArea(), 9.0));
    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    // Tests that dynamically allocating and destroying a Canvas does not crash
    Canvas* dynamicCanvas = new Canvas();
    dynamicCanvas->add(new Rectangle(2.0, 2.0));
    dynamicCanvas->add(new Circle(1.0));
    assert(dynamicCanvas->count() == 2);
    delete dynamicCanvas; // ~Canvas() calls clear(), properly deallocating shapes
    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addShapesAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}