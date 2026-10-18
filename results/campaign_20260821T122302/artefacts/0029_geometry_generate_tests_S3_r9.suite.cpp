#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparison
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test initial state of Canvas after default construction
void test_canvas_default_constructor() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_canvas_default_constructor: PASS" << std::endl;
}

// Test adding a null pointer to the canvas
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

// Test adding shapes and checking count
void test_canvas_add_shapes() {
    Canvas canvas;
    
    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);
    assert(canvas.shapes.size() == 1);

    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);
    assert(canvas.shapes.size() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);
    assert(canvas.shapes.size() == 3);

    std::cout << "Test test_canvas_add_shapes: PASS" << std::endl;
}

// Test totalArea calculation with multiple shapes
void test_canvas_totalArea() {
    Canvas canvas;

    // Rectangle 1: 3 * 4 = 12.0
    canvas.add(new Rectangle(3.0, 4.0));
    // Rectangle 2: 2 * 5 = 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    // Circle: PI * 2^2 = 4 * PI
    canvas.add(new Circle(2.0));
    // Generic Shape: 0.0
    canvas.add(new Shape());

    const double PI = 3.14159265358979323846;
    double expectedTotal = 12.0 + 10.0 + (PI * 4.0) + 0.0;

    assert(approxEqual(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_canvas_totalArea: PASS" << std::endl;
}

// Test largestArea when canvas is empty
void test_canvas_largestArea_empty() {
    Canvas canvas;
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_canvas_largestArea_empty: PASS" << std::endl;
}

// Test largestArea with shapes of various sizes in different positions
void test_canvas_largestArea_multiple_shapes() {
    // Case 1: Largest shape added first
    {
        Canvas canvas;
        canvas.add(new Rectangle(10.0, 10.0)); // area = 100.0
        canvas.add(new Rectangle(2.0, 3.0));   // area = 6.0
        canvas.add(new Circle(1.0));           // area ~= 3.14159
        assert(approxEqual(canvas.largestArea(), 100.0));
    }

    // Case 2: Largest shape added in the middle
    {
        Canvas canvas;
        canvas.add(new Rectangle(2.0, 3.0));   // area = 6.0
        canvas.add(new Circle(5.0));           // area = PI * 25 ~= 78.5398
        canvas.add(new Rectangle(4.0, 5.0));   // area = 20.0
        const double PI = 3.14159265358979323846;
        assert(approxEqual(canvas.largestArea(), PI * 25.0));
    }

    // Case 3: Largest shape added last
    {
        Canvas canvas;
        canvas.add(new Rectangle(1.0, 1.0));   // area = 1.0
        canvas.add(new Circle(1.0));           // area ~= 3.14159
        canvas.add(new Rectangle(8.0, 8.0));   // area = 64.0
        assert(approxEqual(canvas.largestArea(), 64.0));
    }

    std::cout << "Test test_canvas_largestArea_multiple_shapes: PASS" << std::endl;
}

// Test clear() method and subsequent reuse of the canvas
void test_canvas_clear() {
    Canvas canvas;

    canvas.add(new Rectangle(5.0, 5.0));
    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Calling clear on an already empty canvas should not crash
    canvas.clear();
    assert(canvas.count() == 0);

    // Canvas should still be usable after clear()
    canvas.add(new Rectangle(3.0, 3.0)); // area = 9.0
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 9.0));
    assert(approxEqual(canvas.largestArea(), 9.0));

    std::cout << "Test test_canvas_clear: PASS" << std::endl;
}

// Test destructor cleanup behavior (allocation and destruction of Canvas on heap)
void test_canvas_destructor() {
    Canvas* canvas = new Canvas();
    canvas->add(new Rectangle(2.0, 2.0));
    canvas->add(new Circle(1.0));
    canvas->add(new Shape());
    assert(canvas->count() == 3);

    // Deleting canvas will invoke ~Canvas() which calls clear() and frees all owned shapes
    delete canvas;

    std::cout << "Test test_canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "--- Running Canvas Unit Tests ---" << std::endl;

    test_canvas_default_constructor();
    test_canvas_add_null();
    test_canvas_add_shapes();
    test_canvas_totalArea();
    test_canvas_largestArea_empty();
    test_canvas_largestArea_multiple_shapes();
    test_canvas_clear();
    test_canvas_destructor();

    std::cout << "--- All Canvas tests PASSED! ---" << std::endl;
    return 0;
}