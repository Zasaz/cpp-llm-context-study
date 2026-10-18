#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
static bool approxEqual(double a, double b, double epsilon = 1e-5) {
    return std::abs(a - b) < epsilon;
}

// Test initial state of a newly created Canvas
void test_Canvas_defaultConstructor() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_defaultConstructor: PASS" << std::endl;
}

// Test adding valid (non-null) shapes to the Canvas
void test_Canvas_addValidShapes() {
    Canvas canvas;

    Shape* rect = new Rectangle(4.0, 5.0); // area = 20.0
    Shape* circ = new Circle(2.0);          // area ≈ 12.56637

    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes[0] == rect);

    canvas.add(circ);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == circ);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

// Test that null pointers are ignored and not added
void test_Canvas_addNullShape() {
    Canvas canvas;

    canvas.add(nullptr);
    canvas.add(0);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    // Add a valid shape, then null, then another valid shape
    canvas.add(new Rectangle(2.0, 3.0));
    canvas.add(nullptr);
    canvas.add(new Circle(1.0));

    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

// Test totalArea calculation with multiple shapes
void test_Canvas_totalArea() {
    Canvas canvas;

    // Empty canvas area should be 0.0
    assert(approxEqual(canvas.totalArea(), 0.0));

    // Rectangle: 3.0 * 4.0 = 12.0
    canvas.add(new Rectangle(3.0, 4.0));
    assert(approxEqual(canvas.totalArea(), 12.0));

    // Circle: PI * 1.0 * 1.0 ≈ 3.14159265
    canvas.add(new Circle(1.0));
    assert(approxEqual(canvas.totalArea(), 12.0 + 3.141592653589793));

    // Generic Shape: area = 0.0
    canvas.add(new Shape());
    assert(approxEqual(canvas.totalArea(), 12.0 + 3.141592653589793));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largestArea calculation across various shapes
void test_Canvas_largestArea() {
    Canvas canvas;

    // Empty canvas largest area should be 0.0
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Add smaller shape first: Rectangle area = 6.0
    canvas.add(new Rectangle(2.0, 3.0));
    assert(approxEqual(canvas.largestArea(), 6.0));

    // Add larger shape: Rectangle area = 50.0
    canvas.add(new Rectangle(5.0, 10.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Add medium shape: Circle(3.0) area ≈ 28.2743
    canvas.add(new Circle(3.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Add generic base Shape: area = 0.0
    canvas.add(new Shape());
    assert(approxEqual(canvas.largestArea(), 50.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clear() method resets the canvas and frees stored memory
void test_Canvas_clear() {
    Canvas canvas;

    canvas.add(new Rectangle(5.0, 5.0));
    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Verify canvas can be reused after clear()
    canvas.add(new Rectangle(2.0, 4.0));
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 8.0));
    assert(approxEqual(canvas.largestArea(), 8.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test Canvas destructor cleans up shapes correctly (no double frees or crashes)
void test_Canvas_destructor() {
    {
        Canvas canvas;
        canvas.add(new Rectangle(10.0, 10.0));
        canvas.add(new Circle(5.0));
        canvas.add(new Shape());
        assert(canvas.count() == 3);
        // Destructor invoked here as canvas goes out of scope
    }

    // Also test dynamic allocation and deallocation of Canvas
    Canvas* dynamicCanvas = new Canvas();
    dynamicCanvas->add(new Rectangle(1.0, 2.0));
    dynamicCanvas->add(new Circle(3.0));
    delete dynamicCanvas;

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_defaultConstructor();
    test_Canvas_addValidShapes();
    test_Canvas_addNullShape();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "=== All Canvas Unit Tests Passed Successfully ===" << std::endl;
    return 0;
}