#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function to compare floating point numbers
bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Concrete mock/implementation of Shape for testing Canvas independently
class MockShape : public Shape {
private:
    double mockArea;
    bool* destructionFlag;

public:
    MockShape(double areaVal, bool* flag = nullptr) 
        : mockArea(areaVal), destructionFlag(flag) {
        if (destructionFlag != nullptr) {
            *destructionFlag = false;
        }
    }

    virtual ~MockShape() {
        if (destructionFlag != nullptr) {
            *destructionFlag = true;
        }
    }

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
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

void test_Canvas_addAndCount() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    assert(canvas.count() == 1);

    canvas.add(new MockShape(20.0));
    assert(canvas.count() == 2);

    canvas.add(nullptr); // should not increase count
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_addAndCount: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    canvas.add(new MockShape(10.5));
    canvas.add(new MockShape(20.25));
    canvas.add(new MockShape(5.25));

    assert(canvas.count() == 3);
    assert(areClose(canvas.totalArea(), 36.0));
    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    canvas.add(new MockShape(15.0));
    canvas.add(new MockShape(42.5));
    canvas.add(new MockShape(30.0));
    canvas.add(new MockShape(5.0));

    assert(areClose(canvas.largestArea(), 42.5));
    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clearDeletesShapes() {
    bool destroyed1 = false;
    bool destroyed2 = false;

    Canvas canvas;
    canvas.add(new MockShape(10.0, &destroyed1));
    canvas.add(new MockShape(20.0, &destroyed2));

    assert(!destroyed1);
    assert(!destroyed2);
    assert(canvas.count() == 2);

    canvas.clear();

    assert(destroyed1);
    assert(destroyed2);
    assert(canvas.count() == 0);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_clearDeletesShapes: PASS" << std::endl;
}

void test_Canvas_destructorCleansMemory() {
    bool destroyed1 = false;
    bool destroyed2 = false;

    {
        Canvas canvas;
        canvas.add(new MockShape(12.0, &destroyed1));
        canvas.add(new MockShape(34.0, &destroyed2));
        assert(!destroyed1);
        assert(!destroyed2);
    } // canvas goes out of scope and its destructor is called

    assert(destroyed1);
    assert(destroyed2);
    std::cout << "Test test_Canvas_destructorCleansMemory: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullPointer();
    test_Canvas_addAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clearDeletesShapes();
    test_Canvas_destructorCleansMemory();

    std::cout << "=== All Canvas Tests Passed Successfully! ===" << std::endl;
    return 0;
}