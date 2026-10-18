#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
static bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test initial state of an empty canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding valid shapes and handling nullptr / null pointers
void test_Canvas_addShapesAndCount() {
    Canvas canvas;

    canvas.add(nullptr);
    assert(canvas.count() == 0);

    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);

    canvas.add(nullptr);
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_addShapesAndCount: PASS" << std::endl;
}

// Test calculation of total area
void test_Canvas_totalArea() {
    Canvas canvas;

    // Rectangle: 4 * 5 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    // Circle: PI * 1^2 = PI (~3.14159265)
    canvas.add(new Circle(1.0));
    // Generic base Shape: 0.0
    canvas.add(new Shape());

    double expectedArea = 20.0 + (3.14159265358979323846 * 1.0 * 1.0) + 0.0;
    assert(isClose(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test finding the largest area among shapes
void test_Canvas_largestArea() {
    Canvas canvas;

    // Empty canvas largestArea should be 0.0
    assert(isClose(canvas.largestArea(), 0.0));

    // Add rectangle: area = 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(isClose(canvas.largestArea(), 10.0));

    // Add circle: area = PI * 3^2 = 9 * PI (~28.2743)
    canvas.add(new Circle(3.0));
    assert(isClose(canvas.largestArea(), 3.14159265358979323846 * 9.0));

    // Add smaller rectangle: area = 1.0
    canvas.add(new Rectangle(1.0, 1.0));
    assert(isClose(canvas.largestArea(), 3.14159265358979323846 * 9.0));

    // Add larger rectangle: area = 50.0
    canvas.add(new Rectangle(10.0, 5.0));
    assert(isClose(canvas.largestArea(), 50.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test explicitly calling clear() to empty canvas and deallocate memory
void test_Canvas_clear() {
    Canvas canvas;

    canvas.add(new Rectangle(3.0, 4.0));
    canvas.add(new Circle(5.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Calling clear again on an empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    // Canvas can be reused after clear
    canvas.add(new Rectangle(2.0, 2.0));
    assert(canvas.count() == 1);
    assert(isClose(canvas.totalArea(), 4.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test destructor cleanup (validated by scoping)
void test_Canvas_destructor() {
    {
        Canvas canvas;
        canvas.add(new Rectangle(10.0, 2.0));
        canvas.add(new Circle(2.5));
        canvas.add(new Shape());
        assert(canvas.count() == 3);
        // Destructor implicitly executes and frees heap memory allocated for shapes
    }

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Canvas Unit Tests..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addShapesAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "All Canvas Unit Tests Passed Successfully!" << std::endl;
    return 0;
}