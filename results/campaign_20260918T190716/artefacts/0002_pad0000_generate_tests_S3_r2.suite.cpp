#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function to compare floating-point numbers
static bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Custom Shape subclass for testing destructor behavior via Canvas::clear()
class DestructorTracker : public Shape {
public:
    static int deleteCount;
    double customArea;

    DestructorTracker(double a = 0.0) : customArea(a) {}
    virtual ~DestructorTracker() {
        deleteCount++;
    }
    virtual double area() const {
        return customArea;
    }
};
int DestructorTracker::deleteCount = 0;

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addNullPointer() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

void test_Canvas_addValidShapes() {
    Canvas canvas;

    Rectangle* rect = new Rectangle(4.0, 5.0); // Area = 20.0
    Circle* circle = new Circle(2.0);          // Area = PI * 4 = ~12.56637

    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes[0] == rect);

    canvas.add(circle);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == circle);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;

    canvas.add(new Rectangle(2.0, 3.0));  // 6.0
    canvas.add(new Rectangle(5.0, 4.0));  // 20.0
    canvas.add(new Circle(1.0));          // PI * 1^2 = ~3.14159265

    double expectedTotal = 6.0 + 20.0 + (3.14159265358979323846 * 1.0 * 1.0);
    assert(isClose(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;

    // Empty canvas largest area should be 0.0
    assert(isClose(canvas.largestArea(), 0.0));

    canvas.add(new Rectangle(2.0, 3.0)); // Area = 6.0
    assert(isClose(canvas.largestArea(), 6.0));

    canvas.add(new Rectangle(10.0, 5.0)); // Area = 50.0
    assert(isClose(canvas.largestArea(), 50.0));

    canvas.add(new Circle(2.0)); // Area = ~12.56637
    assert(isClose(canvas.largestArea(), 50.0));

    canvas.add(new Circle(5.0)); // Area = PI * 25 = ~78.5398
    assert(isClose(canvas.largestArea(), 3.14159265358979323846 * 25.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    DestructorTracker::deleteCount = 0;
    Canvas canvas;

    canvas.add(new DestructorTracker(10.0));
    canvas.add(new DestructorTracker(25.0));
    canvas.add(new DestructorTracker(5.0));

    assert(canvas.count() == 3);
    assert(DestructorTracker::deleteCount == 0);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(DestructorTracker::deleteCount == 3);
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Calling clear on an already empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);
    assert(DestructorTracker::deleteCount == 3);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    DestructorTracker::deleteCount = 0;
    {
        Canvas canvas;
        canvas.add(new DestructorTracker(12.0));
        canvas.add(new DestructorTracker(18.0));
        assert(canvas.count() == 2);
    } // canvas goes out of scope here; destructor should be invoked

    assert(DestructorTracker::deleteCount == 2);

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Canvas Unit Tests..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullPointer();
    test_Canvas_addValidShapes();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "All Canvas Unit Tests Passed Successfully!" << std::endl;
    return 0;
}