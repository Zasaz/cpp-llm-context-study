#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparison
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Concrete mock implementation of Shape for testing Canvas
class MockShape : public Shape {
private:
    double shapeArea;
    bool* destructorFlag;

public:
    MockShape(double areaVal, bool* flag = nullptr) 
        : shapeArea(areaVal), destructorFlag(flag) {}

    virtual ~MockShape() {
        if (destructorFlag != nullptr) {
            *destructorFlag = true;
        }
    }

    virtual double area() const {
        return shapeArea;
    }
};

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));
    
    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addValidShapes() {
    Canvas canvas;
    canvas.add(new MockShape(10.5));
    assert(canvas.count() == 1);
    
    canvas.add(new MockShape(20.25));
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    // Add a valid shape, then null, then valid
    canvas.add(new MockShape(5.0));
    canvas.add(nullptr);
    canvas.add(new MockShape(15.0));
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    assert(isClose(canvas.totalArea(), 0.0));

    canvas.add(new MockShape(12.5));
    assert(isClose(canvas.totalArea(), 12.5));

    canvas.add(new MockShape(7.5));
    assert(isClose(canvas.totalArea(), 20.0));

    canvas.add(new MockShape(0.0));
    assert(isClose(canvas.totalArea(), 20.0));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    assert(isClose(canvas.largestArea(), 0.0));

    canvas.add(new MockShape(15.0));
    assert(isClose(canvas.largestArea(), 15.0));

    canvas.add(new MockShape(45.5));
    assert(isClose(canvas.largestArea(), 45.5));

    canvas.add(new MockShape(30.0));
    assert(isClose(canvas.largestArea(), 45.5));

    canvas.add(new MockShape(100.0));
    assert(isClose(canvas.largestArea(), 100.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    bool deleted1 = false;
    bool deleted2 = false;

    Canvas canvas;
    canvas.add(new MockShape(10.0, &deleted1));
    canvas.add(new MockShape(20.0, &deleted2));

    assert(canvas.count() == 2);
    assert(!deleted1);
    assert(!deleted2);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(deleted1);
    assert(deleted2);
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Calling clear on an empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    bool deleted1 = false;
    bool deleted2 = false;

    {
        Canvas canvas;
        canvas.add(new MockShape(10.0, &deleted1));
        canvas.add(new MockShape(20.0, &deleted2));
        assert(!deleted1);
        assert(!deleted2);
    } // canvas goes out of scope here; destructor should execute clear()

    assert(deleted1);
    assert(deleted2);

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullShape();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "=== All Canvas Tests Passed Successfully! ===" << std::endl;
    return 0;
}