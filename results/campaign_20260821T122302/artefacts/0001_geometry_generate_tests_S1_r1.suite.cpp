#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Custom Shape to verify virtual deletion by Canvas::clear() / ~Canvas()
class DestructorTrackerShape : public Shape {
public:
    bool* destroyedFlag;

    DestructorTrackerShape(bool* flag) : destroyedFlag(flag) {
        if (destroyedFlag) {
            *destroyedFlag = false;
        }
    }

    virtual ~DestructorTrackerShape() {
        if (destroyedFlag) {
            *destroyedFlag = true;
        }
    }

    double area() const {
        return 42.0;
    }
};

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(nullptr);
    canvas.add(0);

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_addValidShapesAndCount() {
    Canvas canvas;

    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);

    // Adding null should not alter count
    canvas.add(nullptr);
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addValidShapesAndCount: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    assert(isClose(canvas.totalArea(), 0.0));

    // Rectangle: 4 * 5 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    assert(isClose(canvas.totalArea(), 20.0));

    // Circle: PI * 2^2 = 4 * PI ≈ 12.566370614
    canvas.add(new Circle(2.0));
    double expectedArea = 20.0 + (3.14159265358979323846 * 4.0);
    assert(isClose(canvas.totalArea(), expectedArea));

    // Generic Shape area: 0.0
    canvas.add(new Shape());
    assert(isClose(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    assert(isClose(canvas.largestArea(), 0.0));

    // Generic shape with 0 area
    canvas.add(new Shape());
    assert(isClose(canvas.largestArea(), 0.0));

    // Add rectangle with area 15.0
    canvas.add(new Rectangle(3.0, 5.0));
    assert(isClose(canvas.largestArea(), 15.0));

    // Add rectangle with area 50.0
    canvas.add(new Rectangle(10.0, 5.0));
    assert(isClose(canvas.largestArea(), 50.0));

    // Add circle with area PI * 3^2 ≈ 28.27 (smaller than 50.0)
    canvas.add(new Circle(3.0));
    assert(isClose(canvas.largestArea(), 50.0));

    // Add larger circle: radius 5 => PI * 25 ≈ 78.5398
    canvas.add(new Circle(5.0));
    double expectedLargest = 3.14159265358979323846 * 25.0;
    assert(isClose(canvas.largestArea(), expectedLargest));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    bool destroyed1 = false;
    bool destroyed2 = false;

    canvas.add(new DestructorTrackerShape(&destroyed1));
    canvas.add(new DestructorTrackerShape(&destroyed2));

    assert(canvas.count() == 2);
    assert(!destroyed1);
    assert(!destroyed2);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(destroyed1);
    assert(destroyed2);
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Clearing an already empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    bool destroyed = false;
    {
        Canvas canvas;
        canvas.add(new DestructorTrackerShape(&destroyed));
        assert(!destroyed);
    }
    // Destructor should have called clear(), deleting the tracked shape
    assert(destroyed);

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "Running Canvas test suite..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addValidShapesAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}