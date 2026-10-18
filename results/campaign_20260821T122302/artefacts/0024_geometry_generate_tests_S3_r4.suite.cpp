#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper constant and function for floating point comparisons
static const double EPSILON = 1e-6;

static bool approx_equal(double a, double b) {
    return std::abs(a - b) < EPSILON;
}

// A test shape to verify polymorphic behavior and destructor tracking
class TrackingShape : public Shape {
public:
    static int destructor_count;
    double custom_area;

    TrackingShape(double a) : custom_area(a) {}
    ~TrackingShape() {
        destructor_count++;
    }
    double area() const {
        return custom_area;
    }
};
int TrackingShape::destructor_count = 0;

void test_canvas_default_constructor() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(canvas.totalArea() == 0.0);
    assert(canvas.largestArea() == 0.0);
    std::cout << "Test test_canvas_default_constructor: PASS" << std::endl;
}

void test_canvas_add_null() {
    Canvas canvas;
    canvas.add(0);
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(canvas.totalArea() == 0.0);
    assert(canvas.largestArea() == 0.0);
    std::cout << "Test test_canvas_add_null: PASS" << std::endl;
}

void test_canvas_add_and_count() {
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

    std::cout << "Test test_canvas_add_and_count: PASS" << std::endl;
}

void test_canvas_totalArea() {
    Canvas canvas;
    assert(canvas.totalArea() == 0.0);

    // Rectangle: 4 * 5 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    assert(approx_equal(canvas.totalArea(), 20.0));

    // Circle: PI * 3^2 = 28.2743338823
    canvas.add(new Circle(3.0));
    double expectedArea = 20.0 + (3.14159265358979323846 * 3.0 * 3.0);
    assert(approx_equal(canvas.totalArea(), expectedArea));

    // Another Rectangle: 2 * 3 = 6.0
    canvas.add(new Rectangle(2.0, 3.0));
    expectedArea += 6.0;
    assert(approx_equal(canvas.totalArea(), expectedArea));

    std::cout << "Test test_canvas_totalArea: PASS" << std::endl;
}

void test_canvas_largestArea() {
    Canvas canvas;
    // Empty canvas largestArea should be 0.0
    assert(canvas.largestArea() == 0.0);

    // Add first shape (area = 10.0)
    canvas.add(new Rectangle(2.0, 5.0));
    assert(approx_equal(canvas.largestArea(), 10.0));

    // Add smaller shape (area = 4.0)
    canvas.add(new Rectangle(2.0, 2.0));
    assert(approx_equal(canvas.largestArea(), 10.0));

    // Add larger shape: Circle radius 3 -> area ≈ 28.2743
    canvas.add(new Circle(3.0));
    double circleArea = 3.14159265358979323846 * 3.0 * 3.0;
    assert(approx_equal(canvas.largestArea(), circleArea));

    // Add even larger shape: Rectangle 10 * 10 = 100.0
    canvas.add(new Rectangle(10.0, 10.0));
    assert(approx_equal(canvas.largestArea(), 100.0));

    std::cout << "Test test_canvas_largestArea: PASS" << std::endl;
}

void test_canvas_clear() {
    TrackingShape::destructor_count = 0;
    Canvas canvas;

    canvas.add(new TrackingShape(10.0));
    canvas.add(new TrackingShape(20.0));
    canvas.add(new TrackingShape(30.0));
    assert(canvas.count() == 3);

    // clear() should delete all shapes and empty the vector
    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(canvas.totalArea() == 0.0);
    assert(canvas.largestArea() == 0.0);
    assert(TrackingShape::destructor_count == 3);

    // Adding shapes after clear should work normally
    canvas.add(new Rectangle(3.0, 4.0));
    assert(canvas.count() == 1);
    assert(approx_equal(canvas.totalArea(), 12.0));

    std::cout << "Test test_canvas_clear: PASS" << std::endl;
}

void test_canvas_destructor() {
    TrackingShape::destructor_count = 0;
    {
        Canvas localCanvas;
        localCanvas.add(new TrackingShape(5.0));
        localCanvas.add(new TrackingShape(15.0));
        assert(localCanvas.count() == 2);
    } // localCanvas goes out of scope here; its destructor should call clear()

    assert(TrackingShape::destructor_count == 2);
    std::cout << "Test test_canvas_destructor: PASS" << std::endl;
}

void test_canvas_mixed_shapes() {
    Canvas canvas;
    canvas.add(new Shape());              // base shape area = 0.0
    canvas.add(new Rectangle(0.0, 10.0)); // zero width rectangle area = 0.0
    canvas.add(new Circle(0.0));          // zero radius circle area = 0.0
    canvas.add(new Rectangle(5.0, 2.0));  // area = 10.0

    assert(canvas.count() == 4);
    assert(approx_equal(canvas.totalArea(), 10.0));
    assert(approx_equal(canvas.largestArea(), 10.0));

    std::cout << "Test test_canvas_mixed_shapes: PASS" << std::endl;
}

int main() {
    std::cout << "Running Canvas unit tests..." << std::endl;

    test_canvas_default_constructor();
    test_canvas_add_null();
    test_canvas_add_and_count();
    test_canvas_totalArea();
    test_canvas_largestArea();
    test_canvas_clear();
    test_canvas_destructor();
    test_canvas_mixed_shapes();

    std::cout << "All Canvas unit tests passed successfully!" << std::endl;
    return 0;
}