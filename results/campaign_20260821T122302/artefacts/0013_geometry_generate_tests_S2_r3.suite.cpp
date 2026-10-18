#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>

#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparison
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// Concrete Shape class for testing Canvas functionality
class MockShape : public Shape {
private:
    double m_area;
    bool* m_destructorFlag;

public:
    MockShape(double areaValue, bool* destructorFlag = nullptr)
        : m_area(areaValue), m_destructorFlag(destructorFlag) {}

    virtual ~MockShape() {
        if (m_destructorFlag != nullptr) {
            *m_destructorFlag = true;
        }
    }

    virtual double area() const {
        return m_area;
    }
};

void test_empty_canvas() {
    Canvas canvas;

    assert(canvas.count() == 0);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_empty_canvas: PASS" << std::endl;
}

void test_add_null_pointer() {
    Canvas canvas;

    canvas.add(nullptr);
    canvas.add(0);

    assert(canvas.count() == 0);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_add_null_pointer: PASS" << std::endl;
}

void test_add_single_shape() {
    Canvas canvas;
    canvas.add(new MockShape(12.5));

    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 12.5));
    assert(approxEqual(canvas.largestArea(), 12.5));

    std::cout << "Test test_add_single_shape: PASS" << std::endl;
}

void test_multiple_shapes_areas() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.add(new MockShape(25.5));
    canvas.add(new MockShape(5.25));
    canvas.add(new MockShape(15.0));

    assert(canvas.count() == 4);
    assert(approxEqual(canvas.totalArea(), 55.75));
    assert(approxEqual(canvas.largestArea(), 25.5));

    std::cout << "Test test_multiple_shapes_areas: PASS" << std::endl;
}

void test_clear_and_memory_deallocation() {
    bool shape1Deleted = false;
    bool shape2Deleted = false;

    Canvas canvas;
    canvas.add(new MockShape(10.0, &shape1Deleted));
    canvas.add(new MockShape(20.0, &shape2Deleted));

    assert(canvas.count() == 2);
    assert(!shape1Deleted);
    assert(!shape2Deleted);

    canvas.clear();

    assert(shape1Deleted);
    assert(shape2Deleted);
    assert(canvas.count() == 0);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_clear_and_memory_deallocation: PASS" << std::endl;
}

void test_destructor_deallocation() {
    bool shape1Deleted = false;
    bool shape2Deleted = false;

    {
        Canvas canvas;
        canvas.add(new MockShape(14.0, &shape1Deleted));
        canvas.add(new MockShape(7.0, &shape2Deleted));
        assert(canvas.count() == 2);
        assert(!shape1Deleted);
        assert(!shape2Deleted);
    } // Canvas goes out of scope here and destructor should be invoked

    assert(shape1Deleted);
    assert(shape2Deleted);

    std::cout << "Test test_destructor_deallocation: PASS" << std::endl;
}

void test_add_after_clear() {
    Canvas canvas;
    canvas.add(new MockShape(5.0));
    canvas.add(new MockShape(10.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);

    canvas.add(new MockShape(30.0));
    canvas.add(new MockShape(40.0));
    assert(canvas.count() == 2);
    assert(approxEqual(canvas.totalArea(), 70.0));
    assert(approxEqual(canvas.largestArea(), 40.0));

    std::cout << "Test test_add_after_clear: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Canvas test suite..." << std::endl;

    test_empty_canvas();
    test_add_null_pointer();
    test_add_single_shape();
    test_multiple_shapes_areas();
    test_clear_and_memory_deallocation();
    test_destructor_deallocation();
    test_add_after_clear();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}