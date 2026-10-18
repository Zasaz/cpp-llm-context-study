#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_canvas_constructor_initial_state() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    std::cout << "Test test_canvas_constructor_initial_state: PASS" << std::endl;
}

void test_canvas_add_null() {
    Canvas canvas;
    canvas.add(nullptr);
    canvas.add(0);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    std::cout << "Test test_canvas_add_null: PASS" << std::endl;
}

void test_canvas_add_and_count() {
    Canvas canvas;
    
    Rectangle* r1 = new Rectangle(4.0, 5.0);
    Circle* c1 = new Circle(2.0);
    Shape* baseShape = new Shape();

    canvas.add(r1);
    assert(canvas.count() == 1);

    canvas.add(c1);
    assert(canvas.count() == 2);

    canvas.add(baseShape);
    assert(canvas.count() == 3);

    // Adding null should not change the count
    canvas.add(nullptr);
    assert(canvas.count() == 3);

    std::cout << "Test test_canvas_add_and_count: PASS" << std::endl;
}

void test_canvas_totalArea() {
    Canvas canvas;

    // Rectangle 1: area = 2.0 * 3.0 = 6.0
    canvas.add(new Rectangle(2.0, 3.0));
    assert(approxEqual(canvas.totalArea(), 6.0));

    // Rectangle 2: area = 4.0 * 5.0 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    assert(approxEqual(canvas.totalArea(), 26.0));

    // Circle 1: radius = 3.0, area = PI * 9 = ~28.27433388
    double pi = 3.14159265358979323846;
    canvas.add(new Circle(3.0));
    double expectedTotal = 6.0 + 20.0 + (pi * 9.0);
    assert(approxEqual(canvas.totalArea(), expectedTotal));

    // Base Shape has area = 0.0
    canvas.add(new Shape());
    assert(approxEqual(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_canvas_totalArea: PASS" << std::endl;
}

void test_canvas_largestArea() {
    Canvas canvas;

    // Empty canvas largestArea should be 0.0
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Add shapes with varying areas
    canvas.add(new Rectangle(2.0, 2.0));  // area = 4.0
    assert(approxEqual(canvas.largestArea(), 4.0));

    canvas.add(new Rectangle(5.0, 5.0));  // area = 25.0
    assert(approxEqual(canvas.largestArea(), 25.0));

    canvas.add(new Circle(2.0));          // area = PI * 4 = ~12.56637 (less than 25.0)
    assert(approxEqual(canvas.largestArea(), 25.0));

    canvas.add(new Circle(5.0));          // area = PI * 25 = ~78.5398 (new largest)
    double expectedLargest = 3.14159265358979323846 * 25.0;
    assert(approxEqual(canvas.largestArea(), expectedLargest));

    std::cout << "Test test_canvas_largestArea: PASS" << std::endl;
}

void test_canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(10.0, 10.0));
    canvas.add(new Circle(5.0));
    canvas.add(new Shape());

    assert(canvas.count() == 3);
    assert(canvas.totalArea() > 0.0);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Clearing an already empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_canvas_clear: PASS" << std::endl;
}

void test_canvas_reuse_after_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0));
    canvas.clear();

    assert(canvas.count() == 0);

    // Add new shapes after clear
    canvas.add(new Rectangle(10.0, 2.0)); // area = 20.0
    canvas.add(new Rectangle(1.0, 5.0));  // area = 5.0

    assert(canvas.count() == 2);
    assert(approxEqual(canvas.totalArea(), 25.0));
    assert(approxEqual(canvas.largestArea(), 20.0));

    std::cout << "Test test_canvas_reuse_after_clear: PASS" << std::endl;
}

void test_canvas_destructor_scope() {
    // Tests that canvas correctly deletes its allocated shapes when destructed out of scope
    {
        Canvas localCanvas;
        localCanvas.add(new Rectangle(3.0, 4.0));
        localCanvas.add(new Circle(1.5));
    }
    // If destructor executes without memory fault, test passes
    std::cout << "Test test_canvas_destructor_scope: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_canvas_constructor_initial_state();
    test_canvas_add_null();
    test_canvas_add_and_count();
    test_canvas_totalArea();
    test_canvas_largestArea();
    test_canvas_clear();
    test_canvas_reuse_after_clear();
    test_canvas_destructor_scope();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}