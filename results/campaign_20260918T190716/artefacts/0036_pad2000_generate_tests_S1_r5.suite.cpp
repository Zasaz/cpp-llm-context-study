#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparison
static bool approx_equal(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approx_equal(canvas.totalArea(), 0.0));
    assert(approx_equal(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addNullPointer() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approx_equal(canvas.totalArea(), 0.0));
    assert(approx_equal(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

void test_Canvas_addShapesAndCount() {
    Canvas canvas;
    canvas.add(new Rectangle(4.0, 5.0)); // Area = 20.0
    assert(canvas.count() == 1);

    canvas.add(new Circle(1.0)); // Area = ~3.14159
    assert(canvas.count() == 2);

    canvas.add(nullptr); // Should be ignored
    assert(canvas.count() == 2);

    canvas.add(new Rectangle(2.0, 3.0)); // Area = 6.0
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addShapesAndCount: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    // Rectangle 1: 4 * 5 = 20
    canvas.add(new Rectangle(4.0, 5.0));
    // Circle 1: pi * 2^2 = 4 * pi (~12.566370614359172)
    canvas.add(new Circle(2.0));
    // Rectangle 2: 3 * 2 = 6
    canvas.add(new Rectangle(3.0, 2.0));

    double expectedArea = 20.0 + (3.14159265358979323846 * 4.0) + 6.0;
    assert(approx_equal(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    assert(approx_equal(canvas.largestArea(), 0.0));

    canvas.add(new Rectangle(2.0, 3.0)); // Area = 6.0
    assert(approx_equal(canvas.largestArea(), 6.0));

    canvas.add(new Rectangle(5.0, 5.0)); // Area = 25.0
    assert(approx_equal(canvas.largestArea(), 25.0));

    canvas.add(new Circle(2.0)); // Area = ~12.566
    assert(approx_equal(canvas.largestArea(), 25.0));

    canvas.add(new Circle(3.0)); // Area = 9 * pi (~28.27433388)
    assert(approx_equal(canvas.largestArea(), 3.14159265358979323846 * 9.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(10.0, 2.0));
    canvas.add(new Circle(5.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approx_equal(canvas.totalArea(), 0.0));
    assert(approx_equal(canvas.largestArea(), 0.0));

    // Ensure we can add again after clearing
    canvas.add(new Rectangle(1.0, 2.0));
    assert(canvas.count() == 1);
    assert(approx_equal(canvas.totalArea(), 2.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    // Test that scope exit properly cleans up without crashing / double freeing
    {
        Canvas canvas;
        canvas.add(new Rectangle(5.0, 5.0));
        canvas.add(new Circle(2.0));
        assert(canvas.count() == 2);
    }
    // If destructor executes without memory fault, test passes.
    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullPointer();
    test_Canvas_addShapesAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}