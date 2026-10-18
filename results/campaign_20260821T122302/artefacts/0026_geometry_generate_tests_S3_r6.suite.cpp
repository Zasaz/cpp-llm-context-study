#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparison
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

void test_canvas_default_constructor() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_canvas_default_constructor: PASS" << std::endl;
}

void test_canvas_add_null() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    // Add valid shape then null
    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_canvas_add_null: PASS" << std::endl;
}

void test_canvas_add_and_count() {
    Canvas canvas;
    assert(canvas.count() == 0);

    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);

    std::cout << "Test test_canvas_add_and_count: PASS" << std::endl;
}

void test_canvas_totalArea() {
    Canvas canvas;
    assert(approxEqual(canvas.totalArea(), 0.0));

    // Rectangle with area = 4.0 * 5.0 = 20.0
    Rectangle* rect = new Rectangle(4.0, 5.0);
    canvas.add(rect);
    assert(approxEqual(canvas.totalArea(), 20.0));

    // Circle with area = PI * 2.0 * 2.0 = 4.0 * PI
    Circle* circ = new Circle(2.0);
    double expectedCircleArea = circ->area();
    canvas.add(circ);
    assert(approxEqual(canvas.totalArea(), 20.0 + expectedCircleArea));

    // Base Shape with area = 0.0
    canvas.add(new Shape());
    assert(approxEqual(canvas.totalArea(), 20.0 + expectedCircleArea));

    std::cout << "Test test_canvas_totalArea: PASS" << std::endl;
}

void test_canvas_largestArea() {
    Canvas canvas;
    // Empty canvas largest area should be 0.0
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Add shapes with varying areas
    // Area: 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 10.0));

    // Area: 50.0
    canvas.add(new Rectangle(5.0, 10.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Area: ~28.27 (Circle with radius 3)
    canvas.add(new Circle(3.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Area: ~78.54 (Circle with radius 5)
    canvas.add(new Circle(5.0));
    double expectedLargest = 3.14159265358979323846 * 25.0;
    assert(approxEqual(canvas.largestArea(), expectedLargest));

    std::cout << "Test test_canvas_largestArea: PASS" << std::endl;
}

void test_canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(3.0, 4.0));
    canvas.add(new Circle(2.0));
    canvas.add(new Rectangle(10.0, 10.0));

    assert(canvas.count() == 3);
    assert(canvas.totalArea() > 0.0);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Ensure we can continue to use the canvas after clear()
    canvas.add(new Rectangle(2.0, 2.0));
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 4.0));
    assert(approxEqual(canvas.largestArea(), 4.0));

    std::cout << "Test test_canvas_clear: PASS" << std::endl;
}

void test_canvas_destructor_cleanup() {
    // Tests that a canvas going out of scope with heap shapes properly frees memory without errors
    {
        Canvas canvas;
        canvas.add(new Rectangle(1.0, 2.0));
        canvas.add(new Circle(3.0));
        canvas.add(new Shape());
    }
    // If destructor works properly and doesn't crash or double-free, execution reaches here
    std::cout << "Test test_canvas_destructor_cleanup: PASS" << std::endl;
}

int main() {
    std::cout << "Running Canvas unit tests..." << std::endl;

    test_canvas_default_constructor();
    test_canvas_add_null();
    test_canvas_add_and_count();
    test_canvas_totalArea();
    test_canvas_largestArea();
    test_canvas_clear();
    test_canvas_destructor_cleanup();

    std::cout << "All Canvas unit tests passed successfully!" << std::endl;
    return 0;
}