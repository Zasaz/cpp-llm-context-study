#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"
#include "Vector2.h"

// Helper function for floating point comparisons
static bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Custom shape for testing polymorphic behavior and custom area calculations
class Triangle : public Shape {
public:
    double base;
    double height;

    Triangle(double b, double h) : base(b), height(h) {}

    double area() const override {
        return 0.5 * base * height;
    }
};

// Test Canvas initial state
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding shapes and null checks
void test_Canvas_add() {
    Canvas canvas;
    
    // Test adding null pointer
    canvas.add(nullptr);
    assert(canvas.count() == 0);

    // Test adding valid shapes
    Shape* rect = new Rectangle(4.0, 5.0);
    Shape* circ = new Circle(3.0);

    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes[0] == rect);

    canvas.add(circ);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == circ);

    // Test adding another null pointer does not increase count
    canvas.add(nullptr);
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_add: PASS" << std::endl;
}

// Test totalArea calculation
void test_Canvas_totalArea() {
    Canvas canvas;
    assert(areClose(canvas.totalArea(), 0.0));

    // Base Shape: area = 0
    canvas.add(new Shape());
    assert(areClose(canvas.totalArea(), 0.0));

    // Rectangle: 3 * 4 = 12.0
    canvas.add(new Rectangle(3.0, 4.0));
    assert(areClose(canvas.totalArea(), 12.0));

    // Circle: PI * 2^2 = 4 * PI
    const double PI = 3.14159265358979323846;
    canvas.add(new Circle(2.0));
    assert(areClose(canvas.totalArea(), 12.0 + (4.0 * PI)));

    // Triangle: 0.5 * 6 * 4 = 12.0
    canvas.add(new Triangle(6.0, 4.0));
    assert(areClose(canvas.totalArea(), 24.0 + (4.0 * PI)));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largestArea computation
void test_Canvas_largestArea() {
    Canvas canvas;
    assert(areClose(canvas.largestArea(), 0.0));

    // Add shapes with varying areas
    canvas.add(new Rectangle(2.0, 3.0)); // Area = 6
    assert(areClose(canvas.largestArea(), 6.0));

    canvas.add(new Rectangle(5.0, 5.0)); // Area = 25
    assert(areClose(canvas.largestArea(), 25.0));

    canvas.add(new Circle(1.0)); // Area ~ 3.14159
    assert(areClose(canvas.largestArea(), 25.0));

    canvas.add(new Circle(5.0)); // Area = 25 * PI ~ 78.5398
    const double expectedMax = 3.14159265358979323846 * 25.0;
    assert(areClose(canvas.largestArea(), expectedMax));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test explicit clear() functionality
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 2.0));
    canvas.add(new Circle(1.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    // Multiple clears should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    // Can add new shapes after clearing
    canvas.add(new Rectangle(3.0, 3.0));
    assert(canvas.count() == 1);
    assert(areClose(canvas.totalArea(), 9.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test destruction cleanup (verifies that destructor calls clear without errors/leaks)
void test_Canvas_destructor() {
    Canvas* canvas = new Canvas();
    canvas->add(new Rectangle(10.0, 10.0));
    canvas->add(new Circle(5.0));
    canvas->add(new Triangle(4.0, 2.0));
    assert(canvas->count() == 3);

    // Destructor invocation
    delete canvas;

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "================ Running Canvas Tests ================" << std::endl;
    
    test_Canvas_initialState();
    test_Canvas_add();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "================ All Tests Passed! ================" << std::endl;
    return 0;
}