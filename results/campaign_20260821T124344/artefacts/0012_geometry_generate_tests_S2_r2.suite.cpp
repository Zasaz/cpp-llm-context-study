#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper comparison for floating-point values
static bool is_close(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Concrete mock Shape for testing Canvas functionality
class MockShape : public Shape {
private:
    double areaVal;
    bool* destroyedFlag;

public:
    MockShape(double area, bool* destroyed = 0)
        : areaVal(area), destroyedFlag(destroyed) {
        if (destroyedFlag) {
            *destroyedFlag = false;
        }
    }

    virtual ~MockShape() {
        if (destroyedFlag) {
            *destroyedFlag = true;
        }
    }

    virtual double area() const {
        return areaVal;
    }
};

// Test 1: Test empty Canvas initialization and defaults
void test_Canvas_empty() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(is_close(canvas.totalArea(), 0.0));
    assert(is_close(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_empty: PASS" << std::endl;
}

// Test 2: Adding valid shapes updates count correctly
void test_Canvas_add() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    assert(canvas.count() == 1);

    canvas.add(new MockShape(25.5));
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_add: PASS" << std::endl;
}

// Test 3: Adding null pointer should be ignored
void test_Canvas_add_null() {
    Canvas canvas;
    canvas.add(0);
    assert(canvas.count() == 0);
    assert(is_close(canvas.totalArea(), 0.0));

    canvas.add(new MockShape(5.0));
    canvas.add(0);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_add_null: PASS" << std::endl;
}

// Test 4: totalArea calculates the correct sum of areas
void test_Canvas_totalArea() {
    Canvas canvas;
    canvas.add(new MockShape(10.5));
    canvas.add(new MockShape(20.25));
    canvas.add(new MockShape(5.25));

    assert(is_close(canvas.totalArea(), 36.0));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test 5: largestArea identifies the maximum area correctly
void test_Canvas_largestArea() {
    Canvas canvas;
    canvas.add(new MockShape(15.0));
    canvas.add(new MockShape(42.5));
    canvas.add(new MockShape(7.0));
    canvas.add(new MockShape(30.0));

    assert(is_close(canvas.largestArea(), 42.5));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test 6: clear() properly deletes shapes and resets canvas
void test_Canvas_clear() {
    bool destroyed1 = false;
    bool destroyed2 = false;

    Canvas canvas;
    canvas.add(new MockShape(10.0, &destroyed1));
    canvas.add(new MockShape(20.0, &destroyed2));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(is_close(canvas.totalArea(), 0.0));
    assert(is_close(canvas.largestArea(), 0.0));
    assert(destroyed1 == true);
    assert(destroyed2 == true);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test 7: Destructor cleans up allocated shapes
void test_Canvas_destructor() {
    bool destroyed1 = false;
    bool destroyed2 = false;

    {
        Canvas canvas;
        canvas.add(new MockShape(12.0, &destroyed1));
        canvas.add(new MockShape(34.0, &destroyed2));
    } // canvas goes out of scope here

    assert(destroyed1 == true);
    assert(destroyed2 == true);

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "--- Starting Canvas Tests ---" << std::endl;

    test_Canvas_empty();
    test_Canvas_add();
    test_Canvas_add_null();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "--- All Canvas Tests PASSED ---" << std::endl;
    return 0;
}