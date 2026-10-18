#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparison
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Custom shape to test polymorphism and destructor behavior
class MockShape : public Shape {
public:
    double customArea;
    bool* destructionFlag;

    MockShape(double a, bool* flag = nullptr) : customArea(a), destructionFlag(flag) {}
    ~MockShape() override {
        if (destructionFlag) {
            *destructionFlag = true;
        }
    }
    double area() const override {
        return customArea;
    }
};

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addValidShapes() {
    Canvas canvas;
    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);

    canvas.add(new MockShape(15.0));
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_addNullPointer() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);

    canvas.add(new Rectangle(2.0, 3.0));
    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    // Empty canvas totalArea is 0.0
    assert(approxEqual(canvas.totalArea(), 0.0));

    // Rectangle area: 4.0 * 5.0 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    assert(approxEqual(canvas.totalArea(), 20.0));

    // Circle area: PI * 2^2 = ~12.566370614
    canvas.add(new Circle(2.0));
    double expectedTotal = 20.0 + (3.14159265358979323846 * 4.0);
    assert(approxEqual(canvas.totalArea(), expectedTotal));

    // Add another shape
    canvas.add(new MockShape(10.5));
    expectedTotal += 10.5;
    assert(approxEqual(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    // Empty canvas largestArea is 0.0
    assert(approxEqual(canvas.largestArea(), 0.0));

    canvas.add(new Rectangle(2.0, 3.0)); // Area = 6.0
    assert(approxEqual(canvas.largestArea(), 6.0));

    canvas.add(new Circle(3.0)); // Area = PI * 9 ~= 28.2743
    double circleArea = 3.14159265358979323846 * 9.0;
    assert(approxEqual(canvas.largestArea(), circleArea));

    canvas.add(new Rectangle(1.0, 2.0)); // Area = 2.0 (smaller, largest should remain unchanged)
    assert(approxEqual(canvas.largestArea(), circleArea));

    canvas.add(new MockShape(50.0)); // Area = 50.0 (new largest)
    assert(approxEqual(canvas.largestArea(), 50.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    bool shape1Deleted = false;
    bool shape2Deleted = false;

    Canvas canvas;
    canvas.add(new MockShape(10.0, &shape1Deleted));
    canvas.add(new MockShape(20.0, &shape2Deleted));

    assert(canvas.count() == 2);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    assert(shape1Deleted == true);
    assert(shape2Deleted == true);

    // Calling clear again on empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    bool shape1Deleted = false;
    bool shape2Deleted = false;

    {
        Canvas canvas;
        canvas.add(new MockShape(10.0, &shape1Deleted));
        canvas.add(new MockShape(20.0, &shape2Deleted));
        assert(!shape1Deleted);
        assert(!shape2Deleted);
    } // canvas goes out of scope, destructor should call clear()

    assert(shape1Deleted == true);
    assert(shape2Deleted == true);

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullPointer();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "=== All Canvas Tests Passed Successfully! ===" << std::endl;
    return 0;
}