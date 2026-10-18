#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function to compare floating-point numbers
bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Concrete mock implementation of Shape for testing Canvas functionality
class MockShape : public Shape {
private:
    double shapeArea;

public:
    explicit MockShape(double a) : shapeArea(a) {}

    virtual ~MockShape() {}

    virtual double area() const {
        return shapeArea;
    }
};

// Test Canvas initialization state
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding null pointer
void test_Canvas_addNull() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_addNull: PASS" << std::endl;
}

// Test adding valid shapes
void test_Canvas_addValidShapes() {
    Canvas canvas;
    MockShape* s1 = new MockShape(10.0);
    MockShape* s2 = new MockShape(25.5);

    canvas.add(s1);
    assert(canvas.count() == 1);
    assert(canvas.shapes[0] == s1);

    canvas.add(s2);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == s2);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

// Test total area calculation
void test_Canvas_totalArea() {
    Canvas canvas;
    canvas.add(new MockShape(12.5));
    canvas.add(new MockShape(20.0));
    canvas.add(new MockShape(7.5));

    assert(areClose(canvas.totalArea(), 40.0));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largest area calculation
void test_Canvas_largestArea() {
    Canvas canvas;
    assert(areClose(canvas.largestArea(), 0.0));

    canvas.add(new MockShape(5.0));
    canvas.add(new MockShape(45.0));
    canvas.add(new MockShape(15.0));
    canvas.add(new MockShape(30.0));

    assert(areClose(canvas.largestArea(), 45.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clearing shapes from canvas
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.add(new MockShape(20.0));

    assert(canvas.count() == 2);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    // Calling clear again on an empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test destructor cleanup (proper deletion when Canvas goes out of scope)
void test_Canvas_destructor() {
    {
        Canvas canvas;
        canvas.add(new MockShape(15.0));
        canvas.add(new MockShape(25.0));
        assert(canvas.count() == 2);
    } // canvas destructor invoked here; should clean up dynamically allocated shapes without crashing

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNull();
    test_Canvas_addValidShapes();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "=== All Canvas Unit Tests Passed Successfully ===" << std::endl;
    return 0;
}