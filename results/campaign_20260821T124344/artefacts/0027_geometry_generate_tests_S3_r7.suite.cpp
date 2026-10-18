#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function to compare floating point numbers with tolerance
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test initial state of an empty Canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding null pointers
void test_Canvas_addNull() {
    Canvas canvas;
    canvas.add(nullptr);
    canvas.add(0);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_addNull: PASS" << std::endl;
}

// Test adding valid shapes and verifying count
void test_Canvas_addShapesAndCount() {
    Canvas canvas;

    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addShapesAndCount: PASS" << std::endl;
}

// Test total area computation with different shapes
void test_Canvas_totalArea() {
    Canvas canvas;
    // Rectangle area = 4.0 * 5.0 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    // Circle area = PI * 2^2 = 4 * PI ≈ 12.566370614359172
    canvas.add(new Circle(2.0));
    // Base Shape area = 0.0
    canvas.add(new Shape());

    double expectedTotal = 20.0 + (3.14159265358979323846 * 4.0) + 0.0;
    assert(approxEqual(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test finding the largest area among shapes
void test_Canvas_largestArea() {
    Canvas canvas;

    // Empty canvas largestArea should be 0.0
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Add rectangle with area 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 10.0));

    // Add circle with radius 3.0, area = 9 * PI ≈ 28.27433388
    canvas.add(new Circle(3.0));
    double circleArea = 3.14159265358979323846 * 9.0;
    assert(approxEqual(canvas.largestArea(), circleArea));

    // Add rectangle with area 50.0
    canvas.add(new Rectangle(5.0, 10.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Add base shape with area 0.0
    canvas.add(new Shape());
    assert(approxEqual(canvas.largestArea(), 50.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clear() functionality and reusing canvas
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(10.0, 10.0));
    canvas.add(new Circle(5.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    // Verify canvas can be reused after clear
    canvas.add(new Rectangle(3.0, 4.0));
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 12.0));
    assert(approxEqual(canvas.largestArea(), 12.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test destructor cleanup (implicit execution upon leaving scope)
void test_Canvas_destructor() {
    {
        Canvas canvas;
        canvas.add(new Rectangle(2.0, 3.0));
        canvas.add(new Circle(1.0));
        canvas.add(new Shape());
    } // canvas destructor called here, freeing dynamically allocated shapes

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNull();
    test_Canvas_addShapesAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "=== All tests passed successfully! ===" << std::endl;
    return 0;
}