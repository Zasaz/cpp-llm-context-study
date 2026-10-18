#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for comparing floating-point numbers
bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Custom mock shape to verify polymorphism and destructor execution in Canvas
class DestructorTrackerShape : public Shape {
public:
    bool* destroyedFlag;
    double shapeArea;

    DestructorTrackerShape(bool* flag, double a = 10.0) : destroyedFlag(flag), shapeArea(a) {
        if (destroyedFlag) {
            *destroyedFlag = false;
        }
    }

    virtual ~DestructorTrackerShape() {
        if (destroyedFlag) {
            *destroyedFlag = true;
        }
    }

    virtual double area() const {
        return shapeArea;
    }
};

void test_Canvas_constructor_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_constructor_initialState: PASS" << std::endl;
}

void test_Canvas_add_validShapes() {
    Canvas canvas;
    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_add_validShapes: PASS" << std::endl;
}

void test_Canvas_add_nullPointer() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);

    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_add_nullPointer: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    // Empty canvas
    assert(areClose(canvas.totalArea(), 0.0));

    // Rectangle of area 20.0 (4 * 5)
    canvas.add(new Rectangle(4.0, 5.0));
    assert(areClose(canvas.totalArea(), 20.0));

    // Circle of radius 3.0 (area = PI * 9)
    const double PI = 3.14159265358979323846;
    canvas.add(new Circle(3.0));
    double expectedArea = 20.0 + (PI * 9.0);
    assert(areClose(canvas.totalArea(), expectedArea));

    // Generic shape with area 0.0
    canvas.add(new Shape());
    assert(areClose(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    // Empty canvas
    assert(areClose(canvas.largestArea(), 0.0));

    // Add shapes with varying areas
    canvas.add(new Rectangle(2.0, 3.0)); // area = 6.0
    assert(areClose(canvas.largestArea(), 6.0));

    canvas.add(new Rectangle(5.0, 5.0)); // area = 25.0
    assert(areClose(canvas.largestArea(), 25.0));

    canvas.add(new Circle(2.0)); // area ~ 12.5664
    assert(areClose(canvas.largestArea(), 25.0));

    canvas.add(new Circle(4.0)); // area = PI * 16 ~ 50.2655
    double expectedMax = 3.14159265358979323846 * 16.0;
    assert(areClose(canvas.largestArea(), expectedMax));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    bool destroyed1 = false;
    bool destroyed2 = false;

    canvas.add(new DestructorTrackerShape(&destroyed1, 15.0));
    canvas.add(new DestructorTrackerShape(&destroyed2, 25.0));

    assert(canvas.count() == 2);
    assert(!destroyed1);
    assert(!destroyed2);

    canvas.clear();

    // Verify elements were deleted and vector was cleared
    assert(destroyed1);
    assert(destroyed2);
    assert(canvas.count() == 0);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    // Clearing an already empty canvas should not crash
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor_deallocatesShapes() {
    bool destroyed = false;
    {
        Canvas canvas;
        canvas.add(new DestructorTrackerShape(&destroyed, 50.0));
        assert(!destroyed);
    } // canvas goes out of scope here, invoking destructor

    assert(destroyed);

    std::cout << "Test test_Canvas_destructor_deallocatesShapes: PASS" << std::endl;
}

int main() {
    std::cout << "================ Running Canvas Unit Tests ================" << std::endl;

    test_Canvas_constructor_initialState();
    test_Canvas_add_validShapes();
    test_Canvas_add_nullPointer();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor_deallocatesShapes();

    std::cout << "================ All Tests Passed Successfully! ================" << std::endl;
    return 0;
}