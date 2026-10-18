#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test initial empty state of the Canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding null pointers to the Canvas
void test_Canvas_addNull() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_addNull: PASS" << std::endl;
}

// Test adding valid shapes and verifying count()
void test_Canvas_addAndCount() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(1.0));
    assert(canvas.count() == 2);

    canvas.add(nullptr); // Null should not increase count
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_addAndCount: PASS" << std::endl;
}

// Test totalArea calculation across different shapes
void test_Canvas_totalArea() {
    Canvas canvas;
    // Rectangle: 4.0 * 5.0 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    // Circle: PI * 2.0 * 2.0 ≈ 12.566370614
    canvas.add(new Circle(2.0));
    // Base Shape: 0.0
    canvas.add(new Shape());

    double expectedArea = 20.0 + (3.14159265358979323846 * 4.0) + 0.0;
    assert(isClose(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largestArea calculation
void test_Canvas_largestArea() {
    Canvas canvas;
    // Empty canvas largest area should be 0.0
    assert(isClose(canvas.largestArea(), 0.0));

    // Add first shape (Area = 10.0)
    canvas.add(new Rectangle(2.0, 5.0));
    assert(isClose(canvas.largestArea(), 10.0));

    // Add larger shape (Area = 50.0)
    canvas.add(new Rectangle(10.0, 5.0));
    assert(isClose(canvas.largestArea(), 50.0));

    // Add smaller shape (Area = PI * 1^2 ≈ 3.14159)
    canvas.add(new Circle(1.0));
    assert(isClose(canvas.largestArea(), 50.0));

    // Add largest shape (Area = PI * 5^2 ≈ 78.5398)
    canvas.add(new Circle(5.0));
    double expectedLargest = 3.14159265358979323846 * 25.0;
    assert(isClose(canvas.largestArea(), expectedLargest));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clear() explicitly and re-usability of Canvas after clear()
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(3.0, 4.0));
    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Canvas should still accept new elements after clearing
    canvas.add(new Rectangle(2.0, 2.0));
    assert(canvas.count() == 1);
    assert(isClose(canvas.totalArea(), 4.0));
    assert(isClose(canvas.largestArea(), 4.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test destruction lifecycle (verifying destructor execution does not throw/crash)
void test_Canvas_destructor() {
    {
        Canvas canvas;
        canvas.add(new Rectangle(1.0, 1.0));
        canvas.add(new Circle(1.0));
        canvas.add(new Shape());
        // Destructor called automatically at scope exit, which calls clear() and deletes shapes.
    }
    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    test_Canvas_initialState();
    test_Canvas_addNull();
    test_Canvas_addAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}