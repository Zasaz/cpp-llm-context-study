#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"
#include "Vector2.h"

// Helper function for floating point comparisons
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test initial state of Canvas
void test_canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    std::cout << "Test test_canvas_initialState: PASS" << std::endl;
}

// Test adding null pointer
void test_canvas_addNull() {
    Canvas canvas;
    canvas.add(NULL);
    canvas.add(0);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    std::cout << "Test test_canvas_addNull: PASS" << std::endl;
}

// Test adding shapes and verifying count
void test_canvas_addAndCount() {
    Canvas canvas;
    
    Rectangle* rect1 = new Rectangle(4.0, 5.0);
    Circle* circle1 = new Circle(2.0);
    Shape* baseShape = new Shape();

    canvas.add(rect1);
    assert(canvas.count() == 1);
    assert(canvas.shapes.size() == 1);
    assert(canvas.shapes[0] == rect1);

    canvas.add(circle1);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == circle1);

    canvas.add(baseShape);
    assert(canvas.count() == 3);
    assert(canvas.shapes[2] == baseShape);

    std::cout << "Test test_canvas_addAndCount: PASS" << std::endl;
}

// Test total area computation
void test_canvas_totalArea() {
    Canvas canvas;
    
    // Rectangle 1: 4 * 5 = 20
    canvas.add(new Rectangle(4.0, 5.0));
    assert(approxEqual(canvas.totalArea(), 20.0));

    // Circle 1: PI * 3^2 ≈ 28.27433388
    double pi = 3.14159265358979323846;
    canvas.add(new Circle(3.0));
    double expectedTotal = 20.0 + (pi * 9.0);
    assert(approxEqual(canvas.totalArea(), expectedTotal));

    // Base shape has area 0.0
    canvas.add(new Shape());
    assert(approxEqual(canvas.totalArea(), expectedTotal));

    // Rectangle 2: 2.5 * 2.0 = 5.0
    canvas.add(new Rectangle(2.5, 2.0));
    expectedTotal += 5.0;
    assert(approxEqual(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_canvas_totalArea: PASS" << std::endl;
}

// Test largest area calculation
void test_canvas_largestArea() {
    Canvas canvas;

    // Initially 0.0
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Add shapes with varying areas in non-monotonic order
    // Shape 1: Area = 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 10.0));

    // Shape 2: Area = 50.0 (new max)
    canvas.add(new Rectangle(10.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Shape 3: Area ≈ 28.27 (less than current max 50.0)
    canvas.add(new Circle(3.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Shape 4: Area ≈ 314.159 (new max)
    canvas.add(new Circle(10.0));
    double expectedMax = 3.14159265358979323846 * 100.0;
    assert(approxEqual(canvas.largestArea(), expectedMax));

    std::cout << "Test test_canvas_largestArea: PASS" << std::endl;
}

// Test clear() method
void test_canvas_clear() {
    Canvas canvas;

    canvas.add(new Rectangle(3.0, 4.0));
    canvas.add(new Circle(1.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Calling clear again on empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    // Should be able to add new shapes after clear
    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 6.0));

    std::cout << "Test test_canvas_clear: PASS" << std::endl;
}

// Test destructor via dynamic allocation and scope exit
void test_canvas_destruction() {
    // Dynamic canvas destruction
    Canvas* dynamicCanvas = new Canvas();
    dynamicCanvas->add(new Rectangle(10.0, 10.0));
    dynamicCanvas->add(new Circle(5.0));
    delete dynamicCanvas; // Invokes ~Canvas() -> clear(), properly freeing shapes

    // Stack scope destruction
    {
        Canvas localCanvas;
        localCanvas.add(new Rectangle(1.0, 2.0));
        localCanvas.add(new Circle(3.0));
    }

    std::cout << "Test test_canvas_destruction: PASS" << std::endl;
}

int main() {
    std::cout << "--- Running Canvas Unit Tests ---" << std::endl;

    test_canvas_initialState();
    test_canvas_addNull();
    test_canvas_addAndCount();
    test_canvas_totalArea();
    test_canvas_largestArea();
    test_canvas_clear();
    test_canvas_destruction();

    std::cout << "--- All Canvas tests PASSED successfully! ---" << std::endl;
    return 0;
}