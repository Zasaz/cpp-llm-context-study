#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function to compare floating point numbers
bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor and initial state
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding null shapes
void test_Canvas_addNull() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_addNull: PASS" << std::endl;
}

// Test adding shapes and verifying count
void test_Canvas_addAndCount() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(1.0));
    assert(canvas.count() == 2);

    canvas.add(nullptr); // should not be added
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_addAndCount: PASS" << std::endl;
}

// Test totalArea calculation
void test_Canvas_totalArea() {
    Canvas canvas;
    canvas.add(new Rectangle(4.0, 5.0)); // area = 20.0
    canvas.add(new Circle(2.0));         // area = PI * 4.0 ≈ 12.5663706
    canvas.add(new Shape());              // base shape area = 0.0

    double expectedArea = 20.0 + (3.14159265358979323846 * 4.0) + 0.0;
    assert(approxEqual(canvas.totalArea(), expectedArea));
    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largestArea calculation
void test_Canvas_largestArea() {
    Canvas canvas;
    assert(approxEqual(canvas.largestArea(), 0.0));

    canvas.add(new Rectangle(2.0, 2.0)); // area = 4.0
    assert(approxEqual(canvas.largestArea(), 4.0));

    canvas.add(new Rectangle(5.0, 5.0)); // area = 25.0
    assert(approxEqual(canvas.largestArea(), 25.0));

    canvas.add(new Circle(2.0));         // area ≈ 12.566
    assert(approxEqual(canvas.largestArea(), 25.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clear() functionality
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0));
    canvas.add(new Circle(1.5));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Can still add shapes after clear
    canvas.add(new Rectangle(1.0, 1.0));
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 1.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test destructor cleans up memory properly when canvas goes out of scope
void test_Canvas_destructor() {
    {
        Canvas canvas;
        canvas.add(new Rectangle(10.0, 10.0));
        canvas.add(new Circle(5.0));
        // Destructor is implicitly called when canvas exits scope, calling clear() and deleting allocated shapes
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

    std::cout << "\nAll Canvas tests passed successfully!" << std::endl;
    return 0;
}