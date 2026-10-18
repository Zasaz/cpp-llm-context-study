#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"

// Tolerance for floating-point comparisons
static const double EPSILON = 1e-9;

static bool doubleEquals(double a, double b) {
    return std::abs(a - b) < EPSILON;
}

// Concrete test shape to be used across tests.
// Implements both const and non-const area() overloads to match Shape's interface.
class TestShape : public Shape {
private:
    double m_area;
    bool* m_destructedFlag;

public:
    explicit TestShape(double area, bool* destructedFlag = nullptr)
        : m_area(area), m_destructedFlag(destructedFlag) {}

    virtual ~TestShape() {
        if (m_destructedFlag != nullptr) {
            *m_destructedFlag = true;
        }
    }

    double area() const { return m_area; }
    double area() { return m_area; }
};

void test_Canvas_initialState() {
    Canvas canvas;

    assert(canvas.count() == 0);
    assert(doubleEquals(canvas.totalArea(), 0.0));
    assert(doubleEquals(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;

    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(doubleEquals(canvas.totalArea(), 0.0));
    assert(doubleEquals(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_addSingleShape() {
    Canvas canvas;

    canvas.add(new TestShape(25.5));

    assert(canvas.count() == 1);
    assert(doubleEquals(canvas.totalArea(), 25.5));
    assert(doubleEquals(canvas.largestArea(), 25.5));

    std::cout << "Test test_Canvas_addSingleShape: PASS" << std::endl;
}

void test_Canvas_multipleShapesCalculations() {
    Canvas canvas;

    canvas.add(new TestShape(10.0));
    canvas.add(new TestShape(45.5));
    canvas.add(new TestShape(5.25));
    canvas.add(new TestShape(30.0));

    assert(canvas.count() == 4);
    assert(doubleEquals(canvas.totalArea(), 10.0 + 45.5 + 5.25 + 30.0));
    assert(doubleEquals(canvas.largestArea(), 45.5));

    std::cout << "Test test_Canvas_multipleShapesCalculations: PASS" << std::endl;
}

void test_Canvas_clear() {
    bool shape1Deleted = false;
    bool shape2Deleted = false;

    Canvas canvas;
    canvas.add(new TestShape(15.0, &shape1Deleted));
    canvas.add(new TestShape(30.0, &shape2Deleted));

    assert(canvas.count() == 2);

    canvas.clear();

    // Verify shapes were properly deleted
    assert(shape1Deleted == true);
    assert(shape2Deleted == true);

    // Verify canvas is reset
    assert(canvas.count() == 0);
    assert(doubleEquals(canvas.totalArea(), 0.0));
    assert(doubleEquals(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructorCleansMemory() {
    bool shape1Deleted = false;
    bool shape2Deleted = false;

    {
        Canvas canvas;
        canvas.add(new TestShape(12.0, &shape1Deleted));
        canvas.add(new TestShape(18.0, &shape2Deleted));
        assert(canvas.count() == 2);
    } // canvas goes out of scope here, invoking ~Canvas()

    assert(shape1Deleted == true);
    assert(shape2Deleted == true);

    std::cout << "Test test_Canvas_destructorCleansMemory: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Canvas unit tests..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addSingleShape();
    test_Canvas_multipleShapesCalculations();
    test_Canvas_clear();
    test_Canvas_destructorCleansMemory();

    std::cout << "All Canvas unit tests PASSED!" << std::endl;
    return 0;
}