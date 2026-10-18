#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_Canvas_initialization() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialization: PASS" << std::endl;
}

void test_Canvas_addShapes() {
    Canvas canvas;

    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);
    assert(canvas.shapes.size() == 1);

    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);
    assert(canvas.shapes.size() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addShapes: PASS" << std::endl;
}

void test_Canvas_addNullPointer() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    // Adding null should not change the count
    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    assert(isClose(canvas.totalArea(), 0.0));

    // Rectangle: 3 * 4 = 12.0
    canvas.add(new Rectangle(3.0, 4.0));
    assert(isClose(canvas.totalArea(), 12.0));

    // Circle: PI * 2 * 2 = ~12.566370614359172
    canvas.add(new Circle(2.0));
    double expectedTotal = 12.0 + (3.14159265358979323846 * 4.0);
    assert(isClose(canvas.totalArea(), expectedTotal));

    // Base Shape area = 0.0
    canvas.add(new Shape());
    assert(isClose(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    assert(isClose(canvas.largestArea(), 0.0));

    // Add smaller shape (area = 10.0)
    canvas.add(new Rectangle(2.0, 5.0));
    assert(isClose(canvas.largestArea(), 10.0));

    // Add larger shape (area = 50.0)
    canvas.add(new Rectangle(10.0, 5.0));
    assert(isClose(canvas.largestArea(), 50.0));

    // Add intermediate shape (area = 25.0)
    canvas.add(new Rectangle(5.0, 5.0));
    assert(isClose(canvas.largestArea(), 50.0));

    // Add even larger shape: Circle radius 5 (area = PI * 25 ≈ 78.5398)
    canvas.add(new Circle(5.0));
    double expectedCircleArea = 3.14159265358979323846 * 25.0;
    assert(isClose(canvas.largestArea(), expectedCircleArea));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0));
    canvas.add(new Circle(1.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Calling clear multiple times should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    // Can add new shapes after clearing
    canvas.add(new Rectangle(5.0, 5.0));
    assert(canvas.count() == 1);
    assert(isClose(canvas.totalArea(), 25.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    // Tests that destruction cleans up properly without crashing / leaking memory
    {
        Canvas canvas;
        canvas.add(new Rectangle(1.0, 1.0));
        canvas.add(new Circle(2.0));
    } // canvas goes out of scope here; its destructor calls clear()

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    test_Canvas_initialization();
    test_Canvas_addShapes();
    test_Canvas_addNullPointer();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "\nAll Canvas tests passed successfully!" << std::endl;
    return 0;
}