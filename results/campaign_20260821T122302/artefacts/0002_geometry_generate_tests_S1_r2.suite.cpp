#include <cassert>
#include <cmath>
#include <iostream>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
static bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) <= epsilon;
}

// Test initial state of a freshly constructed Canvas
void test_canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    std::cout << "Test test_canvas_initialState: PASS" << std::endl;
}

// Test adding null pointers
void test_canvas_addNull() {
    Canvas canvas;
    canvas.add(0);
    canvas.add(nullptr);

    assert(canvas.count() == 0);
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    std::cout << "Test test_canvas_addNull: PASS" << std::endl;
}

// Test adding valid shapes and verifying count
void test_canvas_addValidShapes() {
    Canvas canvas;
    
    Rectangle* rect = new Rectangle(4.0, 5.0);
    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes[0] == rect);

    Circle* circ = new Circle(3.0);
    canvas.add(circ);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == circ);

    Shape* baseShape = new Shape();
    canvas.add(baseShape);
    assert(canvas.count() == 3);
    assert(canvas.shapes[2] == baseShape);

    std::cout << "Test test_canvas_addValidShapes: PASS" << std::endl;
}

// Test totalArea calculation with multiple shapes
void test_canvas_totalArea() {
    Canvas canvas;

    // Rectangle: 4.0 * 5.0 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    // Circle: PI * 2.0 * 2.0 = 4.0 * PI (~12.566370614)
    canvas.add(new Circle(2.0));
    // Base shape: area = 0.0
    canvas.add(new Shape());

    double expectedTotal = 20.0 + (3.14159265358979323846 * 4.0) + 0.0;
    assert(isClose(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_canvas_totalArea: PASS" << std::endl;
}

// Test largestArea with different configurations
void test_canvas_largestArea() {
    // Case 1: Empty canvas
    Canvas emptyCanvas;
    assert(isClose(emptyCanvas.largestArea(), 0.0));

    // Case 2: Shapes with distinct areas (largest in the middle)
    Canvas canvas1;
    canvas1.add(new Rectangle(2.0, 3.0));  // area = 6.0
    canvas1.add(new Rectangle(10.0, 5.0)); // area = 50.0 (largest)
    canvas1.add(new Circle(1.0));          // area ~ 3.14159
    assert(isClose(canvas1.largestArea(), 50.0));

    // Case 3: Largest at the beginning
    Canvas canvas2;
    canvas2.add(new Rectangle(10.0, 10.0)); // area = 100.0 (largest)
    canvas2.add(new Rectangle(2.0, 2.0));   // area = 4.0
    assert(isClose(canvas2.largestArea(), 100.0));

    // Case 4: Largest at the end
    Canvas canvas3;
    canvas3.add(new Rectangle(2.0, 2.0));   // area = 4.0
    canvas3.add(new Rectangle(8.0, 8.0));   // area = 64.0 (largest)
    assert(isClose(canvas3.largestArea(), 64.0));

    std::cout << "Test test_canvas_largestArea: PASS" << std::endl;
}

// Test clear() functionality
void test_canvas_clear() {
    Canvas canvas;

    canvas.add(new Rectangle(3.0, 4.0));
    canvas.add(new Circle(5.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Calling clear on an already empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    // Adding shapes again after clearing
    canvas.add(new Rectangle(2.0, 2.0));
    assert(canvas.count() == 1);
    assert(isClose(canvas.totalArea(), 4.0));
    assert(isClose(canvas.largestArea(), 4.0));

    std::cout << "Test test_canvas_clear: PASS" << std::endl;
}

// Test destructor memory management (destructor calls clear())
void test_canvas_destructor() {
    // Dynamically allocate Canvas and shapes, then delete to exercise destructor
    Canvas* canvas = new Canvas();
    canvas->add(new Rectangle(2.0, 5.0));
    canvas->add(new Circle(1.5));
    canvas->add(new Shape());

    assert(canvas->count() == 3);

    delete canvas; // Should safely deallocate all contained Shape objects

    std::cout << "Test test_canvas_destructor: PASS" << std::endl;
}

int main() {
    test_canvas_initialState();
    test_canvas_addNull();
    test_canvas_addValidShapes();
    test_canvas_totalArea();
    test_canvas_largestArea();
    test_canvas_clear();
    test_canvas_destructor();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}