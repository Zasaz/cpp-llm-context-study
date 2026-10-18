#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function to compare floating point numbers
bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Concrete mock/implementation of Shape for testing Canvas logic
class MockShape : public Shape {
private:
    double mockArea;

public:
    explicit MockShape(double a) : mockArea(a) {}
    virtual ~MockShape() {}

    virtual double area() const {
        return mockArea;
    }
};

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addNullPointer() {
    Canvas canvas;
    canvas.add(0);
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

void test_Canvas_addSingleShape() {
    Canvas canvas;
    canvas.add(new MockShape(25.5));
    
    assert(canvas.count() == 1);
    assert(areClose(canvas.totalArea(), 25.5));
    assert(areClose(canvas.largestArea(), 25.5));
    std::cout << "Test test_Canvas_addSingleShape: PASS" << std::endl;
}

void test_Canvas_multipleShapes() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.add(new MockShape(50.0));
    canvas.add(new MockShape(25.0));

    assert(canvas.count() == 3);
    assert(areClose(canvas.totalArea(), 85.0));
    assert(areClose(canvas.largestArea(), 50.0));
    std::cout << "Test test_Canvas_multipleShapes: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new MockShape(15.0));
    canvas.add(new MockShape(30.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    // Ensure we can add again after clearing
    canvas.add(new MockShape(40.0));
    assert(canvas.count() == 1);
    assert(areClose(canvas.totalArea(), 40.0));
    assert(areClose(canvas.largestArea(), 40.0));
    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_zeroAndDuplicateAreas() {
    Canvas canvas;
    canvas.add(new MockShape(0.0));
    canvas.add(new MockShape(12.5));
    canvas.add(new MockShape(12.5));

    assert(canvas.count() == 3);
    assert(areClose(canvas.totalArea(), 25.0));
    assert(areClose(canvas.largestArea(), 12.5));
    std::cout << "Test test_Canvas_zeroAndDuplicateAreas: PASS" << std::endl;
}

void test_Canvas_destructorCleanup() {
    // Test that dynamically allocating and deleting Canvas properly runs destructor
    Canvas* canvas = new Canvas();
    canvas->add(new MockShape(10.0));
    canvas->add(new MockShape(20.0));
    delete canvas; // Should execute ~Canvas() -> clear() without leaks/crashes
    std::cout << "Test test_Canvas_destructorCleanup: PASS" << std::endl;
}

int main() {
    std::cout << "Running Canvas unit tests..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullPointer();
    test_Canvas_addSingleShape();
    test_Canvas_multipleShapes();
    test_Canvas_clear();
    test_Canvas_zeroAndDuplicateAreas();
    test_Canvas_destructorCleanup();

    std::cout << "All Canvas unit tests passed successfully!" << std::endl;
    return 0;
}