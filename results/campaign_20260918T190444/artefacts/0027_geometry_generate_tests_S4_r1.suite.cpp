#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper constant and function for floating-point comparisons
const double EPSILON = 1e-6;

bool areAlmostEqual(double a, double b) {
    return std::fabs(a - b) < EPSILON;
}

// Mock/Concrete implementation of Shape to test Canvas functionality
class MockShape : public Shape {
private:
    double shapeArea;
    bool* destroyedFlag;

public:
    MockShape(double areaVal, bool* destroyed = 0) 
        : shapeArea(areaVal), destroyedFlag(destroyed) {
        if (destroyedFlag != 0) {
            *destroyedFlag = false;
        }
    }

    virtual ~MockShape() {
        if (destroyedFlag != 0) {
            *destroyedFlag = true;
        }
    }

    virtual double area() const {
        return shapeArea;
    }
};

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(0);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
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

void test_Canvas_addMultipleShapes() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.add(new MockShape(25.0));
    canvas.add(new MockShape(5.5));

    assert(canvas.count() == 3);
    assert(areAlmostEqual(canvas.totalArea(), 40.5));
    assert(areAlmostEqual(canvas.largestArea(), 25.0));

    std::cout << "Test test_Canvas_addMultipleShapes: PASS" << std::endl;
}

void test_Canvas_clearMemoryDeallocation() {
    bool destroyed1 = false;
    bool destroyed2 = false;

    Canvas canvas;
    canvas.add(new MockShape(15.0, &destroyed1));
    canvas.add(new MockShape(30.0, &destroyed2));

    assert(canvas.count() == 2);
    assert(!destroyed1);
    assert(!destroyed2);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(destroyed1);
    assert(destroyed2);
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_clearMemoryDeallocation: PASS" << std::endl;
}

void test_Canvas_destructorCleansMemory() {
    bool destroyed1 = false;
    bool destroyed2 = false;

    {
        Canvas canvas;
        canvas.add(new MockShape(100.0, &destroyed1));
        canvas.add(new MockShape(200.0, &destroyed2));
        assert(!destroyed1);
        assert(!destroyed2);
    } // canvas goes out of scope here and destructor is called

    assert(destroyed1);
    assert(destroyed2);

    std::cout << "Test test_Canvas_destructorCleansMemory: PASS" << std::endl;
}

void test_Canvas_multipleClears() {
    Canvas canvas;
    canvas.clear(); // Clear on empty canvas
    assert(canvas.count() == 0);

    canvas.add(new MockShape(12.0));
    canvas.clear();
    assert(canvas.count() == 0);

    canvas.clear(); // Clear again after already empty
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_multipleClears: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Canvas test suite..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addSingleShape();
    test_Canvas_addMultipleShapes();
    test_Canvas_clearMemoryDeallocation();
    test_Canvas_destructorCleansMemory();
    test_Canvas_multipleClears();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}