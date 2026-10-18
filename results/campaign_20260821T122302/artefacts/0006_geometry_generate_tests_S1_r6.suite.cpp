#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper constant for floating-point comparisons
static const double EPSILON = 1e-6;

static bool approxEqual(double a, double b) {
    return std::fabs(a - b) < EPSILON;
}

// Custom Shape subclass to verify that Canvas::clear() properly invokes virtual destructors
static int g_customShapeDestructorCount = 0;

class TrackedShape : public Shape {
public:
    double customArea;

    TrackedShape(double a = 0.0) : customArea(a) {}

    virtual ~TrackedShape() {
        ++g_customShapeDestructorCount;
    }

    virtual double area() const {
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

void test_Canvas_addShapes() {
    Canvas canvas;

    Shape* rect = new Rectangle(4.0, 5.0);
    Shape* circ = new Circle(2.0);

    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes.size() == 1);
    assert(canvas.shapes[0] == rect);

    canvas.add(circ);
    assert(canvas.count() == 2);
    assert(canvas.shapes.size() == 2);
    assert(canvas.shapes[1] == circ);

    std::cout << "Test test_Canvas_addShapes: PASS" << std::endl;
}

void test_Canvas_addNull() {
    Canvas canvas;

    canvas.add(0);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    Shape* rect = new Rectangle(2.0, 3.0);
    canvas.add(rect);
    assert(canvas.count() == 1);

    canvas.add(0);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNull: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;

    // Rectangle: 4 * 5 = 20
    canvas.add(new Rectangle(4.0, 5.0));
    assert(approxEqual(canvas.totalArea(), 20.0));

    // Circle: PI * 3^2 = 9 * PI
    const double PI = 3.14159265358979323846;
    canvas.add(new Circle(3.0));
    assert(approxEqual(canvas.totalArea(), 20.0 + (PI * 9.0)));

    // Generic shape with 0 area
    canvas.add(new Shape());
    assert(approxEqual(canvas.totalArea(), 20.0 + (PI * 9.0)));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;

    // Empty canvas largestArea should be 0.0
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Add first shape: Area = 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 10.0));

    // Add second shape: Area = 50.0 (largest so far)
    canvas.add(new Rectangle(10.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Add third shape: Area = 12.0 (smaller than current max)
    canvas.add(new Rectangle(3.0, 4.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Add fourth shape: Circle of radius 5, Area = PI * 25 ~ 78.5398 (new largest)
    const double PI = 3.14159265358979323846;
    canvas.add(new Circle(5.0));
    assert(approxEqual(canvas.largestArea(), PI * 25.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    g_customShapeDestructorCount = 0;

    canvas.add(new TrackedShape(10.0));
    canvas.add(new TrackedShape(20.0));
    canvas.add(new TrackedShape(30.0));
    assert(canvas.count() == 3);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    // Verify destructors of contained shapes were called
    assert(g_customShapeDestructorCount == 3);

    // Multiple clear calls should be safe
    canvas.clear();
    assert(canvas.count() == 0);
    assert(g_customShapeDestructorCount == 3);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    g_customShapeDestructorCount = 0;
    {
        Canvas canvas;
        canvas.add(new TrackedShape(5.0));
        canvas.add(new TrackedShape(15.0));
        assert(canvas.count() == 2);
    }
    // canvas went out of scope, destructor should have called clear()
    assert(g_customShapeDestructorCount == 2);

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    test_Canvas_initialState();
    test_Canvas_addShapes();
    test_Canvas_addNull();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}