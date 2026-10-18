#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
static bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test 1: Initial state of an empty Canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());
    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test 2: Adding shapes to Canvas and count() verification
void test_Canvas_addShapesAndCount() {
    Canvas canvas;
    
    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);
    
    canvas.add(new Circle(1.0));
    assert(canvas.count() == 2);
    
    canvas.add(new Shape());
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addShapesAndCount: PASS" << std::endl;
}

// Test 3: Adding a null pointer should be safely ignored
void test_Canvas_addNullPointer() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

// Test 4: Calculation of totalArea with multiple shapes
void test_Canvas_totalArea() {
    Canvas canvas;
    Rectangle* rect1 = new Rectangle(4.0, 5.0); // Area = 20.0
    Rectangle* rect2 = new Rectangle(2.0, 3.0); // Area = 6.0
    Circle* circle = new Circle(2.0);           // Area = pi * 4 ≈ 12.5663706
    Shape* baseShape = new Shape();             // Area = 0.0

    double expectedTotal = 20.0 + 6.0 + (3.14159265358979323846 * 4.0) + 0.0;

    canvas.add(rect1);
    canvas.add(rect2);
    canvas.add(circle);
    canvas.add(baseShape);

    assert(canvas.count() == 4);
    assert(isClose(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test 5: Finding largestArea across diverse shapes
void test_Canvas_largestArea() {
    Canvas canvas;
    
    // Empty canvas largestArea should be 0.0
    assert(isClose(canvas.largestArea(), 0.0));

    Rectangle* rectSmall = new Rectangle(2.0, 3.0); // Area = 6.0
    Rectangle* rectLarge = new Rectangle(10.0, 5.0); // Area = 50.0
    Circle* circleMid = new Circle(3.0);             // Area = pi * 9 ≈ 28.27433

    canvas.add(rectSmall);
    assert(isClose(canvas.largestArea(), 6.0));

    canvas.add(circleMid);
    assert(isClose(canvas.largestArea(), 3.14159265358979323846 * 9.0));

    canvas.add(rectLarge);
    assert(isClose(canvas.largestArea(), 50.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test 6: Explicit clear() clears shapes and resets metrics
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(5.0, 5.0));
    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Canvas should remain usable after clear()
    canvas.add(new Rectangle(3.0, 4.0));
    assert(canvas.count() == 1);
    assert(isClose(canvas.totalArea(), 12.0));
    assert(isClose(canvas.largestArea(), 12.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test 7: Destructor proper cleanup (scope check)
void test_Canvas_destructor() {
    {
        Canvas canvas;
        canvas.add(new Rectangle(1.0, 2.0));
        canvas.add(new Circle(3.0));
        // Destructor called automatically on exit, clearing heap allocations
    }
    // If double deletion or memory corruption occurs, it would trigger errors here
    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "Running Canvas test suite..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addShapesAndCount();
    test_Canvas_addNullPointer();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}