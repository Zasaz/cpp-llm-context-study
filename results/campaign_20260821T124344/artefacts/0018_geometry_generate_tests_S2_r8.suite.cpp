#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper constant for floating-point comparison
const double EPSILON = 1e-6;

bool areAlmostEqual(double a, double b) {
    return std::fabs(a - b) < EPSILON;
}

// Concrete mock implementation of Shape for testing Canvas functionality
class MockShape : public Shape {
private:
    double areaValue;
    bool* deletedFlag;

public:
    MockShape(double areaVal, bool* deleted = nullptr) 
        : areaValue(areaVal), deletedFlag(deleted) {}

    virtual ~MockShape() {
        if (deletedFlag != nullptr) {
            *deletedFlag = true;
        }
    }

    virtual double area() const {
        return areaValue;
    }
};

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_addSingleShape() {
    Canvas canvas;
    canvas.add(new MockShape(25.5));

    assert(canvas.count() == 1);
    assert(areAlmostEqual(canvas.totalArea(), 25.5));
    assert(areAlmostEqual(canvas.largestArea(), 25.5));

    std::cout << "Test test_Canvas_addSingleShape: PASS" << std::endl;
}

void test_Canvas_multipleShapes() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.add(new MockShape(50.5));
    canvas.add(new MockShape(25.0));

    assert(canvas.count() == 3);
    assert(areAlmostEqual(canvas.totalArea(), 85.5));
    assert(areAlmostEqual(canvas.largestArea(), 50.5));

    std::cout << "Test test_Canvas_multipleShapes: PASS" << std::endl;
}

void test_Canvas_clearMemoryManagement() {
    Canvas canvas;
    bool shape1Deleted = false;
    bool shape2Deleted = false;

    canvas.add(new MockShape(15.0, &shape1Deleted));
    canvas.add(new MockShape(30.0, &shape2Deleted));

    assert(canvas.count() == 2);
    assert(!shape1Deleted);
    assert(!shape2Deleted);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(shape1Deleted);
    assert(shape2Deleted);
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_clearMemoryManagement: PASS" << std::endl;
}

void test_Canvas_destructorCleansMemory() {
    bool shapeDeleted = false;
    {
        Canvas canvas;
        canvas.add(new MockShape(42.0, &shapeDeleted));
        assert(!shapeDeleted);
    }
    // After canvas goes out of scope, destructor should have called clear()
    assert(shapeDeleted);

    std::cout << "Test test_Canvas_destructorCleansMemory: PASS" << std::endl;
}

int main() {
    std::cout << "--- Starting Canvas Unit Tests ---" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addSingleShape();
    test_Canvas_multipleShapes();
    test_Canvas_clearMemoryManagement();
    test_Canvas_destructorCleansMemory();

    std::cout << "--- All Canvas Unit Tests Passed Successfully! ---" << std::endl;
    return 0;
}