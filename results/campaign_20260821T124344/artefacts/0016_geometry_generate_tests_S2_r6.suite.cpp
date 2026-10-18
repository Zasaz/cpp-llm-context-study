#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Concrete derived Shape class for testing purposes if no other shapes exist
class MockShape : public Shape {
private:
    double mockArea;
    bool* destructionFlag;

public:
    explicit MockShape(double a, bool* destroyed = nullptr) 
        : mockArea(a), destructionFlag(destroyed) {}

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
    canvas.add(0);
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

void test_Canvas_addMultipleShapes() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.add(new MockShape(25.5));
    canvas.add(new MockShape(5.25));

    assert(canvas.count() == 3);
    assert(areAlmostEqual(canvas.totalArea(), 40.75));
    assert(areAlmostEqual(canvas.largestArea(), 25.5));
    std::cout << "Test test_Canvas_addMultipleShapes: PASS" << std::endl;
}

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

    // Verify properties after clear
    assert(canvas.count() == 0);
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));
    
    // Verify memory was deleted
    assert(shape1Destroyed);
    assert(shape2Destroyed);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructorCleansMemory() {
    bool shapeDestroyed = false;
    {
        Canvas canvas;
        canvas.add(new MockShape(50.0, &shapeDestroyed));
        assert(canvas.count() == 1);
        assert(!shapeDestroyed);
    } // canvas goes out of scope here; destructor should trigger clear()

    assert(shapeDestroyed);
    std::cout << "Test test_Canvas_destructorCleansMemory: PASS" << std::endl;
}

void test_Canvas_reuseAfterClear() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.clear();
    assert(canvas.count() == 0);

    // Add shapes again
    canvas.add(new MockShape(30.0));
    canvas.add(new MockShape(70.0));
    assert(canvas.count() == 2);
    assert(areAlmostEqual(canvas.totalArea(), 100.0));
    assert(areAlmostEqual(canvas.largestArea(), 70.0));

    std::cout << "Test test_Canvas_reuseAfterClear: PASS" << std::endl;
}

int main() {
    std::cout << "--- Starting Canvas Unit Tests ---" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addSingleShape();
    test_Canvas_addMultipleShapes();
    test_Canvas_clear();
    test_Canvas_destructorCleansMemory();
    test_Canvas_reuseAfterClear();

    std::cout << "--- All Canvas Unit Tests Passed Successfully ---" << std::endl;
    return 0;
}