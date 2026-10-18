#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Tolerance for floating point comparisons
static const double EPSILON = 1e-9;

static bool doubleEquals(double a, double b) {
    return std::fabs(a - b) < EPSILON;
}

// Trackable Mock Shape to test calculations and memory management
class TestShape : public Shape {
private:
    double shapeArea;
    int* destructionCounter;

public:
    explicit TestShape(double a, int* counter = nullptr)
        : shapeArea(a), destructionCounter(counter) {}

    virtual ~TestShape() {
        if (destructionCounter != nullptr) {
            (*destructionCounter)++;
        }
    }

    virtual double area() const {
        return shapeArea;
    }
};

void test_Canvas_initialState() {
    Canvas canvas;

    assert(canvas.count() == 0);
    assert(doubleEquals(canvas.totalArea(), 0.0));
    assert(doubleEquals(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addNullPointer() {
    Canvas canvas;

    canvas.add(nullptr);
    canvas.add(0);

    assert(canvas.count() == 0);
    assert(doubleEquals(canvas.totalArea(), 0.0));
    assert(doubleEquals(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

void test_Canvas_addSingleShape() {
    Canvas canvas;
    canvas.add(new TestShape(15.5));

    assert(canvas.count() == 1);
    assert(doubleEquals(canvas.totalArea(), 15.5));
    assert(doubleEquals(canvas.largestArea(), 15.5));

    std::cout << "Test test_Canvas_addSingleShape: PASS" << std::endl;
}

void test_Canvas_multipleShapesCalculations() {
    Canvas canvas;

    canvas.add(new TestShape(10.0));
    canvas.add(new TestShape(25.5));
    canvas.add(new TestShape(5.25));
    canvas.add(new TestShape(18.0));

    assert(canvas.count() == 4);
    assert(doubleEquals(canvas.totalArea(), 10.0 + 25.5 + 5.25 + 18.0));
    assert(doubleEquals(canvas.largestArea(), 25.5));

    std::cout << "Test test_Canvas_multipleShapesCalculations: PASS" << std::endl;
}

void test_Canvas_clearDeletesShapesAndResets() {
    int deletedCount = 0;
    Canvas canvas;

    canvas.add(new TestShape(10.0, &deletedCount));
    canvas.add(new TestShape(20.0, &deletedCount));
    canvas.add(new TestShape(30.0, &deletedCount));

    assert(canvas.count() == 3);
    assert(deletedCount == 0);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(doubleEquals(canvas.totalArea(), 0.0));
    assert(doubleEquals(canvas.largestArea(), 0.0));
    assert(deletedCount == 3);

    // Clearing an already empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);
    assert(deletedCount == 3);

    std::cout << "Test test_Canvas_clearDeletesShapesAndResets: PASS" << std::endl;
}

void test_Canvas_destructorCleansUp() {
    int deletedCount = 0;

    {
        Canvas canvas;
        canvas.add(new TestShape(5.0, &deletedCount));
        canvas.add(new TestShape(15.0, &deletedCount));
        assert(canvas.count() == 2);
    } // Canvas goes out of scope and destructor should invoke clear()

    assert(deletedCount == 2);

    std::cout << "Test test_Canvas_destructorCleansUp: PASS" << std::endl;
}

void test_Canvas_reuseAfterClear() {
    Canvas canvas;

    canvas.add(new TestShape(100.0));
    assert(canvas.count() == 1);
    assert(doubleEquals(canvas.largestArea(), 100.0));

    canvas.clear();
    assert(canvas.count() == 0);

    canvas.add(new TestShape(20.0));
    canvas.add(new TestShape(50.0));
    assert(canvas.count() == 2);
    assert(doubleEquals(canvas.totalArea(), 70.0));
    assert(doubleEquals(canvas.largestArea(), 50.0));

    std::cout << "Test test_Canvas_reuseAfterClear: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullPointer();
    test_Canvas_addSingleShape();
    test_Canvas_multipleShapesCalculations();
    test_Canvas_clearDeletesShapesAndResets();
    test_Canvas_destructorCleansUp();
    test_Canvas_reuseAfterClear();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}