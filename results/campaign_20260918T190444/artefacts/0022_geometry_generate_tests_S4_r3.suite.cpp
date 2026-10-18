#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Concrete mock implementation of Shape for testing Canvas logic
class MockShape : public Shape {
private:
    double mockAreaValue;
    bool* isDeletedFlag;

public:
    explicit MockShape(double areaVal, bool* deletedFlag = nullptr) 
        : mockAreaValue(areaVal), isDeletedFlag(deletedFlag) {}

    virtual ~MockShape() {
        if (isDeletedFlag != nullptr) {
            *isDeletedFlag = true;
        }
    }

    virtual double area() const {
        return mockAreaValue;
    }
};

// 1. Test initial state of Canvas upon construction
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// 2. Test adding valid shapes and verifying count
void test_Canvas_addValidShapes() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    assert(canvas.count() == 1);

    canvas.add(new MockShape(25.5));
    assert(canvas.count() == 2);

    canvas.add(new MockShape(5.0));
    assert(canvas.count() == 3);
    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

// 3. Test that adding a null pointer does not increment count or add to shapes vector
void test_Canvas_addNullPointer() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    canvas.add(new MockShape(12.0));
    assert(canvas.count() == 1);

    canvas.add(nullptr);
    assert(canvas.count() == 1);
    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

// 4. Test calculation of total area
void test_Canvas_totalArea() {
    Canvas canvas;
    assert(areAlmostEqual(canvas.totalArea(), 0.0));

    canvas.add(new MockShape(10.5));
    canvas.add(new MockShape(20.25));
    canvas.add(new MockShape(5.25));

    assert(areAlmostEqual(canvas.totalArea(), 36.0));
    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// 5. Test finding the largest area among shapes
void test_Canvas_largestArea() {
    Canvas canvas;
    // Empty canvas largest area should be 0.0
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    canvas.add(new MockShape(15.0));
    canvas.add(new MockShape(42.5));
    canvas.add(new MockShape(8.0));
    canvas.add(new MockShape(30.0));

    assert(areAlmostEqual(canvas.largestArea(), 42.5));
    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// 6. Test clear function and proper memory deallocation
void test_Canvas_clear() {
    bool shape1Deleted = false;
    bool shape2Deleted = false;

    Canvas canvas;
    canvas.add(new MockShape(10.0, &shape1Deleted));
    canvas.add(new MockShape(20.0, &shape2Deleted));

    assert(canvas.count() == 2);
    assert(!shape1Deleted);
    assert(!shape2Deleted);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(shape1Deleted);
    assert(shape2Deleted);
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    // Calling clear again on empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// 7. Test destructor memory deallocation
void test_Canvas_destructor() {
    bool shape1Deleted = false;
    bool shape2Deleted = false;

    {
        Canvas canvas;
        canvas.add(new MockShape(15.0, &shape1Deleted));
        canvas.add(new MockShape(35.0, &shape2Deleted));
        assert(!shape1Deleted);
        assert(!shape2Deleted);
    } // canvas goes out of scope here; destructor should be invoked

    assert(shape1Deleted);
    assert(shape2Deleted);
    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "Running Canvas Test Suite..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullPointer();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}