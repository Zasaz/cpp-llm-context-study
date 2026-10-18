#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper constant for floating-point comparisons
static const double EPSILON = 1e-6;

static bool areClose(double a, double b) {
    return std::abs(a - b) < EPSILON;
}

// Mock Shape implementation for testing Canvas functionality
class MockShape : public Shape {
private:
    double m_area;
    bool* m_destructorFlag;

public:
    MockShape(double areaVal = 0.0, bool* destructorFlag = nullptr)
        : m_area(areaVal), m_destructorFlag(destructorFlag) {}

    virtual ~MockShape() {
        if (m_destructorFlag != nullptr) {
            *m_destructorFlag = true;
        }
    }

    // Support both const and non-const virtual area() signatures if present in Shape
    double area() const { return m_area; }
    double area() { return m_area; }
};

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(nullptr);
    canvas.add(0);
    assert(canvas.count() == 0);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_addSingleShape() {
    Canvas canvas;
    canvas.add(new MockShape(15.5));
    assert(canvas.count() == 1);
    assert(areClose(canvas.totalArea(), 15.5));
    assert(areClose(canvas.largestArea(), 15.5));
    std::cout << "Test test_Canvas_addSingleShape: PASS" << std::endl;
}

void test_Canvas_multipleShapesCalculations() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.add(new MockShape(45.5));
    canvas.add(new MockShape(5.25));
    canvas.add(new MockShape(30.0));

    assert(canvas.count() == 4);
    assert(areClose(canvas.totalArea(), 10.0 + 45.5 + 5.25 + 30.0));
    assert(areClose(canvas.largestArea(), 45.5));
    std::cout << "Test test_Canvas_multipleShapesCalculations: PASS" << std::endl;
}

void test_Canvas_clearAndMemoryDeallocation() {
    bool destroyed1 = false;
    bool destroyed2 = false;
    bool destroyed3 = false;

    Canvas canvas;
    canvas.add(new MockShape(12.0, &destroyed1));
    canvas.add(new MockShape(24.0, &destroyed2));
    canvas.add(new MockShape(36.0, &destroyed3));

    assert(canvas.count() == 3);
    assert(!destroyed1 && !destroyed2 && !destroyed3);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    assert(destroyed1 && destroyed2 && destroyed3);
    std::cout << "Test test_Canvas_clearAndMemoryDeallocation: PASS" << std::endl;
}

void test_Canvas_destructorDeallocation() {
    bool destroyed1 = false;
    bool destroyed2 = false;

    {
        Canvas canvas;
        canvas.add(new MockShape(7.0, &destroyed1));
        canvas.add(new MockShape(14.0, &destroyed2));
        assert(canvas.count() == 2);
        assert(!destroyed1 && !destroyed2);
    }

    assert(destroyed1);
    assert(destroyed2);
    std::cout << "Test test_Canvas_destructorDeallocation: PASS" << std::endl;
}

void test_Canvas_addAfterClear() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.add(new MockShape(20.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);

    canvas.add(new MockShape(50.0));
    assert(canvas.count() == 1);
    assert(areClose(canvas.totalArea(), 50.0));
    assert(areClose(canvas.largestArea(), 50.0));
    std::cout << "Test test_Canvas_addAfterClear: PASS" << std::endl;
}

void test_Canvas_zeroAreaShapes() {
    Canvas canvas;
    canvas.add(new MockShape(0.0));
    canvas.add(new MockShape(0.0));
    assert(canvas.count() == 2);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_zeroAreaShapes: PASS" << std::endl;
}

int main() {
    std::cout << "Running Canvas unit tests..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addSingleShape();
    test_Canvas_multipleShapesCalculations();
    test_Canvas_clearAndMemoryDeallocation();
    test_Canvas_destructorDeallocation();
    test_Canvas_addAfterClear();
    test_Canvas_zeroAreaShapes();

    std::cout << "All Canvas unit tests passed successfully!" << std::endl;
    return 0;
}