#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparison
bool areAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Concrete mock implementation of Shape for testing Canvas methods
class MockShape : public Shape {
private:
    double mockArea;
    bool* deletedFlag;

public:
    MockShape(double areaValue, bool* deleted = nullptr) 
        : mockArea(areaValue), deletedFlag(deleted) {}

    virtual ~MockShape() {
        if (deletedFlag != nullptr) {
            *deletedFlag = true;
        }
    }

    virtual double area() const {
        return mockArea;
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

void test_Canvas_addValidShapes() {
    Canvas canvas;
    canvas.add(new MockShape(10.5));
    assert(canvas.count() == 1);
    
    canvas.add(new MockShape(25.0));
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);

    canvas.add(new MockShape(5.0));
    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    assert(areAlmostEqual(canvas.totalArea(), 0.0));

    canvas.add(new MockShape(12.5));
    canvas.add(new MockShape(20.25));
    canvas.add(new MockShape(0.25));

    assert(canvas.count() == 3);
    assert(areAlmostEqual(canvas.totalArea(), 33.0));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    canvas.add(new MockShape(15.0));
    assert(areAlmostEqual(canvas.largestArea(), 15.0));

    canvas.add(new MockShape(45.5));
    assert(areAlmostEqual(canvas.largestArea(), 45.5));

    canvas.add(new MockShape(30.0));
    assert(areAlmostEqual(canvas.largestArea(), 45.5));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clearMemoryDeallocation() {
    Canvas canvas;
    bool deleted1 = false;
    bool deleted2 = false;

    canvas.add(new MockShape(10.0, &deleted1));
    canvas.add(new MockShape(20.0, &deleted2));

    assert(!deleted1);
    assert(!deleted2);
    assert(canvas.count() == 2);

    canvas.clear();

    assert(deleted1);
    assert(deleted2);
    assert(canvas.count() == 0);
    assert(areAlmostEqual(canvas.totalArea(), 0.0));
    assert(areAlmostEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_clearMemoryDeallocation: PASS" << std::endl;
}

void test_Canvas_destructor() {
    bool deleted = false;
    {
        Canvas canvas;
        canvas.add(new MockShape(5.0, &deleted));
        assert(!deleted);
    } // canvas goes out of scope here, triggering ~Canvas() which calls clear()

    assert(deleted);

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "Running Canvas unit tests..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullShape();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clearMemoryDeallocation();
    test_Canvas_destructor();

    std::cout << "All Canvas unit tests passed successfully!" << std::endl;
    return 0;
}