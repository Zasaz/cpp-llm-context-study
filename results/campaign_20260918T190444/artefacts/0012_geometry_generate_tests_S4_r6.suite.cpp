#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Mock/Concrete Shape implementation to test polymorphic behavior with Canvas
class MockShape : public Shape {
private:
    double shapeArea;
    bool* destructionFlag;

public:
    explicit MockShape(double areaVal, bool* destroyed = 0)
        : shapeArea(areaVal), destructionFlag(destroyed) {}

    virtual ~MockShape() {
        if (destructionFlag != 0) {
            *destructionFlag = true;
        }
    }

    virtual double area() const {
        return shapeArea;
    }
};

// Test 1: Initial state of an empty canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test 2: Adding shapes and verifying count
void test_Canvas_addAndCount() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    assert(canvas.count() == 1);

    canvas.add(new MockShape(25.5));
    assert(canvas.count() == 2);

    // Adding null pointer should be ignored
    canvas.add(0);
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_addAndCount: PASS" << std::endl;
}

// Test 3: Total area calculation
void test_Canvas_totalArea() {
    Canvas canvas;
    assert(areAlmostEqual(canvas.totalArea(), 0.0));

    canvas.add(new MockShape(12.5));
    assert(areAlmostEqual(canvas.totalArea(), 12.5));

    canvas.add(new MockShape(7.5));
    canvas.add(new MockShape(20.0));
    assert(areAlmostEqual(canvas.totalArea(), 40.0));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test 4: Largest area calculation across multiple shapes
void test_Canvas_largestArea() {
    Canvas canvas;
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    canvas.add(new MockShape(15.0));
    assert(areAlmostEqual(canvas.largestArea(), 15.0));

    // Add smaller shape
    canvas.add(new MockShape(5.0));
    assert(areAlmostEqual(canvas.largestArea(), 15.0));

    // Add larger shape
    canvas.add(new MockShape(42.0));
    assert(areAlmostEqual(canvas.largestArea(), 42.0));

    // Add another shape in-between
    canvas.add(new MockShape(30.0));
    assert(areAlmostEqual(canvas.largestArea(), 42.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test 5: Manual clear() and proper deletion of heap-allocated shapes
void test_Canvas_clear() {
    bool shape1Destroyed = false;
    bool shape2Destroyed = false;

    Canvas canvas;
    canvas.add(new MockShape(10.0, &shape1Destroyed));
    canvas.add(new MockShape(20.0, &shape2Destroyed));

    assert(canvas.count() == 2);
    assert(!shape1Destroyed);
    assert(!shape2Destroyed);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(shape1Destroyed);
    assert(shape2Destroyed);
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    // Clearing an already empty canvas should not crash
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test 6: Destructor memory cleanup verification
void test_Canvas_destructorCleanup() {
    bool shape1Destroyed = false;
    bool shape2Destroyed = false;

    {
        Canvas canvas;
        canvas.add(new MockShape(5.0, &shape1Destroyed));
        canvas.add(new MockShape(15.0, &shape2Destroyed));
        assert(!shape1Destroyed);
        assert(!shape2Destroyed);
    } // canvas goes out of scope here; its destructor should be called

    assert(shape1Destroyed);
    assert(shape2Destroyed);

    std::cout << "Test test_Canvas_destructorCleanup: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructorCleanup();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}