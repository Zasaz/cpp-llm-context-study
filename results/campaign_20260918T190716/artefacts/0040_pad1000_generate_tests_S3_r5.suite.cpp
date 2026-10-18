#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparison
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test initial state of a newly created Canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding null pointers to ensure they are ignored
void test_Canvas_addNull() {
    Canvas canvas;
    canvas.add(nullptr);
    canvas.add(0);

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_addNull: PASS" << std::endl;
}

// Test adding valid shapes and verifying count()
void test_Canvas_addAndCount() {
    Canvas canvas;

    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(1.0));
    assert(canvas.count() == 2);

    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addAndCount: PASS" << std::endl;
}

// Test totalArea calculation with various shapes
void test_Canvas_totalArea() {
    Canvas canvas;

    Rectangle* rect1 = new Rectangle(2.0, 3.0); // area = 6.0
    Rectangle* rect2 = new Rectangle(4.0, 5.0); // area = 20.0
    Circle* circ = new Circle(2.0);              // area = PI * 4.0 ≈ 12.566370614

    canvas.add(rect1);
    assert(approxEqual(canvas.totalArea(), 6.0));

    canvas.add(rect2);
    assert(approxEqual(canvas.totalArea(), 26.0));

    canvas.add(circ);
    double expectedTotal = 6.0 + 20.0 + (3.14159265358979323846 * 4.0);
    assert(approxEqual(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largestArea computation with multiple shapes added in varying orders
void test_Canvas_largestArea() {
    Canvas canvas;

    // Initially largest area is 0
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Add smaller shape
    canvas.add(new Rectangle(2.0, 2.0)); // area = 4.0
    assert(approxEqual(canvas.largestArea(), 4.0));

    // Add larger shape
    canvas.add(new Rectangle(5.0, 5.0)); // area = 25.0
    assert(approxEqual(canvas.largestArea(), 25.0));

    // Add intermediate shape
    canvas.add(new Circle(2.0)); // area = PI * 4 ≈ 12.566
    assert(approxEqual(canvas.largestArea(), 25.0));

    // Add even larger shape
    canvas.add(new Circle(10.0)); // area = PI * 100 ≈ 314.159
    assert(approxEqual(canvas.largestArea(), 3.14159265358979323846 * 100.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clear functionality
void test_Canvas_clear() {
    Canvas canvas;

    canvas.add(new Rectangle(10.0, 10.0));
    canvas.add(new Circle(5.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Verify canvas is still usable after clear
    canvas.add(new Rectangle(3.0, 4.0)); // area = 12.0
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 12.0));
    assert(approxEqual(canvas.largestArea(), 12.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test destructor cleanup (proper deallocation of managed shapes without crashing)
void test_Canvas_destructor() {
    {
        Canvas canvas;
        canvas.add(new Rectangle(1.0, 2.0));
        canvas.add(new Circle(3.0));
        canvas.add(new Shape());
    } // canvas goes out of scope here and destructor calls clear()

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Test Suite ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNull();
    test_Canvas_addAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}