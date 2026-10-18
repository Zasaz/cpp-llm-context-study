#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Custom shape to verify polymorphism and destructor invocations
class TestTrackedShape : public Shape {
public:
    int* destructionCounter;
    double customArea;

    TestTrackedShape(double a, int* counter) : destructionCounter(counter), customArea(a) {}
    virtual ~TestTrackedShape() {
        if (destructionCounter != nullptr) {
            (*destructionCounter)++;
        }
    }
    virtual double area() const override {
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
    
    Rectangle* rect = new Rectangle(4.0, 5.0);
    Circle* circle = new Circle(3.0);

    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes[0] == rect);

    canvas.add(circle);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == circle);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_addNullPointer() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    // Add a valid shape, then null, ensuring only valid shape is added
    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);
    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    // Rectangle 4x5 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    // Circle radius 2 = PI * 2 * 2 ≈ 12.5663706144
    canvas.add(new Circle(2.0));
    // Generic base shape = 0.0
    canvas.add(new Shape());

    double expectedArea = 20.0 + (3.14159265358979323846 * 4.0) + 0.0;
    assert(approxEqual(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Area: 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 10.0));

    // Area: 50.0 (New largest)
    canvas.add(new Rectangle(10.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Area: ~28.274 (Smaller than largest)
    canvas.add(new Circle(3.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Area: ~78.5398 (New largest)
    canvas.add(new Circle(5.0));
    double expectedLargest = 3.14159265358979323846 * 25.0;
    assert(approxEqual(canvas.largestArea(), expectedLargest));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    int destructCount = 0;

    canvas.add(new TestTrackedShape(15.0, &destructCount));
    canvas.add(new TestTrackedShape(25.0, &destructCount));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(destructCount == 2);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Calling clear again on an empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);
    assert(destructCount == 2);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructorCleansMemory() {
    int destructCount = 0;
    {
        Canvas canvas;
        canvas.add(new TestTrackedShape(10.0, &destructCount));
        canvas.add(new TestTrackedShape(20.0, &destructCount));
        canvas.add(new TestTrackedShape(30.0, &destructCount));
        assert(canvas.count() == 3);
    }
    // All 3 shapes should have been deleted when canvas went out of scope
    assert(destructCount == 3);

    std::cout << "Test test_Canvas_destructorCleansMemory: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullPointer();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructorCleansMemory();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}