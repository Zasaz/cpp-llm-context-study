#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparison
static bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// Custom Shape subclass to verify polymorphic behavior and destruction
class CustomTestShape : public Shape {
public:
    double customArea;
    bool* destroyedFlag;

    CustomTestShape(double areaVal, bool* flag = nullptr)
        : customArea(areaVal), destroyedFlag(flag) {}

    ~CustomTestShape() override {
        if (destroyedFlag) {
            *destroyedFlag = true;
        }
    }

    double area() const override {
        return customArea;
    }
};

void test_initial_state() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_initial_state: PASS" << std::endl;
}

void test_add_null_pointer() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_add_null_pointer: PASS" << std::endl;
}

void test_add_and_count() {
    Canvas canvas;
    assert(canvas.count() == 0);

    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);
    assert(canvas.shapes.size() == 1);

    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);
    assert(canvas.shapes.size() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);
    assert(canvas.shapes.size() == 3);

    std::cout << "Test test_add_and_count: PASS" << std::endl;
}

void test_totalArea() {
    Canvas canvas;
    // Empty canvas total area is 0
    assert(areClose(canvas.totalArea(), 0.0));

    // Rectangle: 4 * 5 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    assert(areClose(canvas.totalArea(), 20.0));

    // Circle: PI * 3^2 = 9 * PI
    const double pi = 3.14159265358979323846;
    canvas.add(new Circle(3.0));
    assert(areClose(canvas.totalArea(), 20.0 + (pi * 9.0)));

    // Base Shape has area 0.0
    canvas.add(new Shape());
    assert(areClose(canvas.totalArea(), 20.0 + (pi * 9.0)));

    std::cout << "Test test_totalArea: PASS" << std::endl;
}

void test_largestArea() {
    Canvas canvas;
    // Empty canvas largest area is 0.0
    assert(areClose(canvas.largestArea(), 0.0));

    // First shape: area 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(areClose(canvas.largestArea(), 10.0));

    // Add larger shape: area 50.0
    canvas.add(new Rectangle(10.0, 5.0));
    assert(areClose(canvas.largestArea(), 50.0));

    // Add smaller shape: area ~12.566
    canvas.add(new Circle(2.0));
    assert(areClose(canvas.largestArea(), 50.0));

    // Add even larger shape: area 100.0
    canvas.add(new Rectangle(10.0, 10.0));
    assert(areClose(canvas.largestArea(), 100.0));

    std::cout << "Test test_largestArea: PASS" << std::endl;
}

void test_clear() {
    bool shape1Destroyed = false;
    bool shape2Destroyed = false;

    Canvas canvas;
    canvas.add(new CustomTestShape(15.0, &shape1Destroyed));
    canvas.add(new CustomTestShape(25.0, &shape2Destroyed));

    assert(canvas.count() == 2);
    assert(!shape1Destroyed);
    assert(!shape2Destroyed);

    canvas.clear();

    // Verify elements were deleted and list is empty
    assert(shape1Destroyed);
    assert(shape2Destroyed);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    // Verify canvas can be reused after clear
    canvas.add(new Rectangle(3.0, 4.0));
    assert(canvas.count() == 1);
    assert(areClose(canvas.totalArea(), 12.0));
    assert(areClose(canvas.largestArea(), 12.0));

    std::cout << "Test test_clear: PASS" << std::endl;
}

void test_destructor_deletes_shapes() {
    bool shapeDestroyed = false;

    {
        Canvas canvas;
        canvas.add(new CustomTestShape(42.0, &shapeDestroyed));
        assert(!shapeDestroyed);
    } // canvas goes out of scope here, invoking ~Canvas() -> clear()

    assert(shapeDestroyed);

    std::cout << "Test test_destructor_deletes_shapes: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_initial_state();
    test_add_null_pointer();
    test_add_and_count();
    test_totalArea();
    test_largestArea();
    test_clear();
    test_destructor_deletes_shapes();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}