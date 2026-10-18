#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test initial state of a newly created Canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding valid shapes to the Canvas
void test_Canvas_addValidShapes() {
    Canvas canvas;

    Shape* rect = new Rectangle(4.0, 5.0);
    Shape* circ = new Circle(3.0);

    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes.size() == 1);
    assert(canvas.shapes[0] == rect);

    canvas.add(circ);
    assert(canvas.count() == 2);
    assert(canvas.shapes.size() == 2);
    assert(canvas.shapes[1] == circ);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

// Test adding null pointers to the Canvas
void test_Canvas_addNullShape() {
    Canvas canvas;

    canvas.add(0);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    Shape* rect = new Rectangle(2.0, 3.0);
    canvas.add(rect);
    assert(canvas.count() == 1);

    canvas.add(0);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

// Test totalArea calculation
void test_Canvas_totalArea() {
    Canvas canvas;

    // Empty canvas total area is 0.0
    assert(approxEqual(canvas.totalArea(), 0.0));

    // Add Rectangle (area = 2 * 3 = 6.0)
    canvas.add(new Rectangle(2.0, 3.0));
    assert(approxEqual(canvas.totalArea(), 6.0));

    // Add another Rectangle (area = 4 * 5 = 20.0)
    canvas.add(new Rectangle(4.0, 5.0));
    assert(approxEqual(canvas.totalArea(), 26.0));

    // Add Circle (radius = 2.0, area = PI * 4.0 ~ 12.566370614)
    canvas.add(new Circle(2.0));
    double expectedArea = 6.0 + 20.0 + (3.14159265358979323846 * 4.0);
    assert(approxEqual(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largestArea calculation
void test_Canvas_largestArea() {
    Canvas canvas;

    // Empty canvas largest area is 0.0
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Add smallest shape first: Circle with radius 1 (area ~ 3.14159)
    canvas.add(new Circle(1.0));
    assert(approxEqual(canvas.largestArea(), 3.14159265358979323846));

    // Add larger shape: Rectangle 5x5 (area = 25.0)
    canvas.add(new Rectangle(5.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 25.0));

    // Add medium shape: Rectangle 2x4 (area = 8.0), largest should still be 25.0
    canvas.add(new Rectangle(2.0, 4.0));
    assert(approxEqual(canvas.largestArea(), 25.0));

    // Add shape with larger area: Circle with radius 5 (area ~ 78.5398)
    canvas.add(new Circle(5.0));
    double expectedLargest = 3.14159265358979323846 * 25.0;
    assert(approxEqual(canvas.largestArea(), expectedLargest));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test explicit clear() functionality and re-use
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

    // Calling clear on an already empty canvas should not cause issues
    canvas.clear();
    assert(canvas.count() == 0);

    // Canvas can be re-populated after clear
    canvas.add(new Rectangle(3.0, 4.0));
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 12.0));
    assert(approxEqual(canvas.largestArea(), 12.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test Canvas destructor cleans up shapes when canvas goes out of scope
void test_Canvas_destructor() {
    {
        Canvas canvas;
        canvas.add(new Rectangle(1.0, 2.0));
        canvas.add(new Circle(3.0));
        canvas.add(new Shape());
        assert(canvas.count() == 3);
    } // canvas goes out of scope here and destructor calls clear()

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "Running Canvas test suite..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullShape();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}