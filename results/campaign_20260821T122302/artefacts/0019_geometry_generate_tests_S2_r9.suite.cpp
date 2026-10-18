#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper constant for floating-point comparisons
static const double EPSILON = 1e-9;

static bool doubleEquals(double a, double b) {
    return std::fabs(a - b) < EPSILON;
}

// Concrete Shape class for testing Canvas functionality
class MockShape : public Shape {
public:
    static int s_destructorCount;
    double m_area;

    explicit MockShape(double a) : m_area(a) {}

    virtual ~MockShape() {
        ++s_destructorCount;
    }

    virtual double area() const {
        return m_area;
    }
};

int MockShape::s_destructorCount = 0;

void test_initial_state() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(doubleEquals(canvas.totalArea(), 0.0));
    assert(doubleEquals(canvas.largestArea(), 0.0));

    std::cout << "Test test_initial_state: PASS" << std::endl;
}

void test_add_null_shape() {
    Canvas canvas;
    canvas.add(0);
    assert(canvas.count() == 0);
    assert(doubleEquals(canvas.totalArea(), 0.0));
    assert(doubleEquals(canvas.largestArea(), 0.0));

    std::cout << "Test test_add_null_shape: PASS" << std::endl;
}

void test_add_single_shape() {
    Canvas canvas;
    canvas.add(new MockShape(42.5));

    assert(canvas.count() == 1);
    assert(doubleEquals(canvas.totalArea(), 42.5));
    assert(doubleEquals(canvas.largestArea(), 42.5));

    std::cout << "Test test_add_single_shape: PASS" << std::endl;
}

void test_add_multiple_shapes() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.add(new MockShape(25.5));
    canvas.add(new MockShape(5.25));

    assert(canvas.count() == 3);
    assert(doubleEquals(canvas.totalArea(), 40.75));
    assert(doubleEquals(canvas.largestArea(), 25.5));

    std::cout << "Test test_add_multiple_shapes: PASS" << std::endl;
}

void test_largest_area_ordering() {
    // Test largest shape at first position
    {
        Canvas canvas;
        canvas.add(new MockShape(100.0));
        canvas.add(new MockShape(50.0));
        canvas.add(new MockShape(25.0));
        assert(doubleEquals(canvas.largestArea(), 100.0));
    }

    // Test largest shape at middle position
    {
        Canvas canvas;
        canvas.add(new MockShape(50.0));
        canvas.add(new MockShape(100.0));
        canvas.add(new MockShape(25.0));
        assert(doubleEquals(canvas.largestArea(), 100.0));
    }

    // Test largest shape at last position
    {
        Canvas canvas;
        canvas.add(new MockShape(25.0));
        canvas.add(new MockShape(50.0));
        canvas.add(new MockShape(100.0));
        assert(doubleEquals(canvas.largestArea(), 100.0));
    }

    std::cout << "Test test_largest_area_ordering: PASS" << std::endl;
}

void test_clear() {
    MockShape::s_destructorCount = 0;
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.add(new MockShape(20.0));
    assert(canvas.count() == 2);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(doubleEquals(canvas.totalArea(), 0.0));
    assert(doubleEquals(canvas.largestArea(), 0.0));
    assert(MockShape::s_destructorCount == 2);

    // Ensure clear can be called on an already empty canvas without issue
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_clear: PASS" << std::endl;
}

void test_destructor_deletes_shapes() {
    MockShape::s_destructorCount = 0;
    {
        Canvas canvas;
        canvas.add(new MockShape(1.0));
        canvas.add(new MockShape(2.0));
        canvas.add(new MockShape(3.0));
        assert(canvas.count() == 3);
    }
    // After canvas goes out of scope, destructor should have deleted all 3 shapes
    assert(MockShape::s_destructorCount == 3);

    std::cout << "Test test_destructor_deletes_shapes: PASS" << std::endl;
}

void test_reuse_after_clear() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.clear();
    assert(canvas.count() == 0);

    canvas.add(new MockShape(15.0));
    canvas.add(new MockShape(30.0));
    assert(canvas.count() == 2);
    assert(doubleEquals(canvas.totalArea(), 45.0));
    assert(doubleEquals(canvas.largestArea(), 30.0));

    std::cout << "Test test_reuse_after_clear: PASS" << std::endl;
}

int main() {
    std::cout << "Running Canvas unit tests..." << std::endl;

    test_initial_state();
    test_add_null_shape();
    test_add_single_shape();
    test_add_multiple_shapes();
    test_largest_area_ordering();
    test_clear();
    test_destructor_deletes_shapes();
    test_reuse_after_clear();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}