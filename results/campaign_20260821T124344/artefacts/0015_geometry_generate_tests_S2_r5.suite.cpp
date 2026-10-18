#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Concrete derived Shape class for testing purposes
class MockShape : public Shape {
private:
    double mockArea;
    bool* destructionFlag;

public:
    MockShape(double areaVal, bool* flag = nullptr) 
        : mockArea(areaVal), destructionFlag(flag) {}

    virtual ~MockShape() {
        if (destructionFlag != nullptr) {
            *destructionFlag = true;
        }
    }

    virtual double area() const override {
        return mockArea;
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
    canvas.add(new MockShape(15.5));

    assert(canvas.count() == 1);
    assert(areAlmostEqual(canvas.totalArea(), 15.5));
    assert(areAlmostEqual(canvas.largestArea(), 15.5));

    std::cout << "Test test_Canvas_addSingleShape: PASS" << std::endl;
}

void test_Canvas_addMultipleShapes_totalAndLargestArea() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.add(new MockShape(25.5));
    canvas.add(new MockShape(5.25));

    assert(canvas.count() == 3);
    assert(areAlmostEqual(canvas.totalArea(), 40.75));
    assert(areAlmostEqual(canvas.largestArea(), 25.5));

    std::cout << "Test test_Canvas_addMultipleShapes_totalAndLargestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    bool shape1Deleted = false;
    bool shape2Deleted = false;

    Canvas canvas;
    canvas.add(new MockShape(10.0, &shape1Deleted));
    canvas.add(new MockShape(20.0, &shape2Deleted));

    assert(canvas.count() == 2);

    canvas.clear();

    assert(shape1Deleted == true);
    assert(shape2Deleted == true);
    assert(canvas.count() == 0);
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    // Calling clear again on an empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    bool shape1Deleted = false;
    bool shape2Deleted = false;

    {
        Canvas canvas;
        canvas.add(new MockShape(12.0, &shape1Deleted));
        canvas.add(new MockShape(18.0, &shape2Deleted));
        assert(canvas.count() == 2);
    } // canvas goes out of scope and destructor should delete shapes

    assert(shape1Deleted == true);
    assert(shape2Deleted == true);

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "Running Canvas unit tests..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addSingleShape();
    test_Canvas_addMultipleShapes_totalAndLargestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "All Canvas unit tests passed successfully!" << std::endl;
    return 0;
}