#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function to check floating point equality within a tolerance
static bool approxEqual(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) < eps;
}

// Test default constructor and initial state of empty Canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding null pointers
void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

// Test adding valid shapes and verifying count
void test_Canvas_addValidShapes() {
    Canvas canvas;
    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

// Test totalArea calculation
void test_Canvas_totalArea() {
    Canvas canvas;

    // Rectangle area = 4 * 5 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));

    // Circle area = PI * 2^2 = 4 * PI ≈ 12.566370614
    canvas.add(new Circle(2.0));

    // Base Shape area = 0.0
    canvas.add(new Shape());

    double expectedArea = 20.0 + (3.14159265358979323846 * 4.0) + 0.0;
    assert(approxEqual(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largestArea calculation
void test_Canvas_largestArea() {
    Canvas canvas;
    
    // Add shapes with different areas
    canvas.add(new Rectangle(2.0, 3.0)); // Area = 6.0
    canvas.add(new Circle(3.0));         // Area = PI * 9 ≈ 28.27433388
    canvas.add(new Rectangle(10.0, 2.0)); // Area = 20.0

    double expectedLargest = 3.14159265358979323846 * 9.0;
    assert(approxEqual(canvas.largestArea(), expectedLargest));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clear() method functionality
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0));
    canvas.add(new Circle(1.5));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Ensure adding shapes after clear works properly
    canvas.add(new Rectangle(5.0, 5.0));
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 25.0));
    assert(approxEqual(canvas.largestArea(), 25.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test Canvas destructor behavior by scoping
void test_Canvas_destructor() {
    {
        Canvas canvas;
        canvas.add(new Rectangle(1.0, 1.0));
        canvas.add(new Circle(2.0));
        // Destructor should clean up allocated shapes without leaks or errors
    }

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addValidShapes();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "\nAll Canvas tests passed successfully!" << std::endl;
    return 0;
}