#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Custom mock shape to test custom area and destructor behavior
class MockShape : public Shape {
public:
    double customArea;
    bool* destroyedFlag;

    MockShape(double a, bool* flag = nullptr) : customArea(a), destroyedFlag(flag) {}
    ~MockShape() override {
        if (destroyedFlag != nullptr) {
            *destroyedFlag = true;
        }
    }

    double area() const override {
        return customArea;
    }
};

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addValidShapes() {
    Canvas canvas;
    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);

    assert(canvas.shapes.size() == 2);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;

    // Rectangle of 4x5 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    // Rectangle of 2x3 = 6.0
    canvas.add(new Rectangle(2.0, 3.0));
    // Circle of radius 1 = PI * 1^2 ≈ 3.141592653589793
    canvas.add(new Circle(1.0));

    double expectedTotal = 20.0 + 6.0 + (3.14159265358979323846 * 1.0 * 1.0);
    assert(approxEqual(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;

    assert(approxEqual(canvas.largestArea(), 0.0));

    // Area: 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 10.0));

    // Area: 50.0
    canvas.add(new Rectangle(5.0, 10.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Area: PI * 3^2 ≈ 28.2743... (smaller than 50.0)
    canvas.add(new Circle(3.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Area: PI * 5^2 ≈ 78.5398... (larger than 50.0)
    canvas.add(new Circle(5.0));
    double expectedCircleArea = 3.14159265358979323846 * 25.0;
    assert(approxEqual(canvas.largestArea(), expectedCircleArea));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    bool destroyed1 = false;
    bool destroyed2 = false;

    canvas.add(new MockShape(15.0, &destroyed1));
    canvas.add(new MockShape(25.0, &destroyed2));
    assert(canvas.count() == 2);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(destroyed1 == true);
    assert(destroyed2 == true);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Clear on already empty canvas should not crash
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    bool destroyed1 = false;
    bool destroyed2 = false;

    {
        Canvas canvas;
        canvas.add(new MockShape(10.0, &destroyed1));
        canvas.add(new MockShape(20.0, &destroyed2));
        assert(canvas.count() == 2);
    } // canvas goes out of scope here, triggering ~Canvas()

    assert(destroyed1 == true);
    assert(destroyed2 == true);

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

void test_Canvas_multipleOperationsLifecycle() {
    Canvas canvas;

    // Cycle 1: Add and clear
    canvas.add(new Rectangle(10.0, 10.0)); // 100
    canvas.add(new Rectangle(2.0, 5.0));   // 10
    assert(canvas.count() == 2);
    assert(approxEqual(canvas.totalArea(), 110.0));
    assert(approxEqual(canvas.largestArea(), 100.0));

    canvas.clear();
    assert(canvas.count() == 0);

    // Cycle 2: Add again to verify reusable state
    canvas.add(new Circle(2.0)); // PI * 4
    assert(canvas.count() == 1);
    double expectedArea = 3.14159265358979323846 * 4.0;
    assert(approxEqual(canvas.totalArea(), expectedArea));
    assert(approxEqual(canvas.largestArea(), expectedArea));

    std::cout << "Test test_Canvas_multipleOperationsLifecycle: PASS" << std::endl;
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
    test_Canvas_multipleOperationsLifecycle();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}