#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for comparing floating-point numbers
static bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Custom Shape subclass for testing specific behaviors/destructions
class TrackingShape : public Shape {
public:
    static int deleteCount;
    double customArea;

    TrackingShape(double a = 0.0) : customArea(a) {}
    virtual ~TrackingShape() {
        ++deleteCount;
    }

    double area() const {
        return customArea;
    }
};
int TrackingShape::deleteCount = 0;

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addValidShapes() {
    Canvas canvas;

    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_addNullPointer() {
    Canvas canvas;
    
    // Adding nullptr / 0 should be safely ignored
    canvas.add(0);
    assert(canvas.count() == 0);

    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    canvas.add(0);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;

    // Rectangle: 4 * 5 = 20
    canvas.add(new Rectangle(4.0, 5.0));
    // Circle: PI * 3^2 = 9 * PI ≈ 28.27433388
    canvas.add(new Circle(3.0));
    // Base Shape: 0.0
    canvas.add(new Shape());

    double expectedTotal = 20.0 + (3.14159265358979323846 * 9.0) + 0.0;
    assert(areClose(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;

    // Case: Empty canvas
    assert(areClose(canvas.largestArea(), 0.0));

    // Case: Single shape
    canvas.add(new Rectangle(2.0, 3.0)); // area = 6
    assert(areClose(canvas.largestArea(), 6.0));

    // Case: Larger shape added
    canvas.add(new Rectangle(5.0, 5.0)); // area = 25
    assert(areClose(canvas.largestArea(), 25.0));

    // Case: Smaller shape added
    canvas.add(new Circle(1.0)); // area = PI ≈ 3.14159
    assert(areClose(canvas.largestArea(), 25.0));

    // Case: Even larger shape added
    canvas.add(new Circle(5.0)); // area = 25 * PI ≈ 78.5398
    assert(areClose(canvas.largestArea(), 3.14159265358979323846 * 25.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    TrackingShape::deleteCount = 0;
    Canvas canvas;

    canvas.add(new TrackingShape(10.0));
    canvas.add(new TrackingShape(20.0));
    canvas.add(new TrackingShape(30.0));

    assert(canvas.count() == 3);
    assert(TrackingShape::deleteCount == 0);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(TrackingShape::deleteCount == 3);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    // Clearing an already empty canvas should not crash
    canvas.clear();
    assert(canvas.count() == 0);
    assert(TrackingShape::deleteCount == 3);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    TrackingShape::deleteCount = 0;
    {
        Canvas canvas;
        canvas.add(new TrackingShape(5.0));
        canvas.add(new TrackingShape(15.0));
        assert(canvas.count() == 2);
    } // canvas goes out of scope here; destructor must invoke clear() and free shapes

    assert(TrackingShape::deleteCount == 2);

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

void test_Canvas_reuseAfterClear() {
    Canvas canvas;

    canvas.add(new Rectangle(2.0, 2.0)); // area = 4
    assert(canvas.count() == 1);
    assert(areClose(canvas.totalArea(), 4.0));

    canvas.clear();
    assert(canvas.count() == 0);

    canvas.add(new Rectangle(3.0, 3.0)); // area = 9
    canvas.add(new Rectangle(1.0, 1.0)); // area = 1
    assert(canvas.count() == 2);
    assert(areClose(canvas.totalArea(), 10.0));
    assert(areClose(canvas.largestArea(), 9.0));

    std::cout << "Test test_Canvas_reuseAfterClear: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Canvas test suite..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullPointer();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();
    test_Canvas_reuseAfterClear();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}