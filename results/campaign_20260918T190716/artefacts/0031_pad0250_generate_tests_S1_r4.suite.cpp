#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function to compare floating point values with an epsilon tolerance
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Custom mock shape to test custom shapes and proper destruction
class TrackedShape : public Shape {
public:
    static int deleteCount;
    double customArea;

    TrackedShape(double a = 0.0) : customArea(a) {}
    ~TrackedShape() override {
        ++deleteCount;
    }

    double area() const override {
        return customArea;
    }
};
int TrackedShape::deleteCount = 0;

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addValidShapes() {
    Canvas canvas;
    
    Rectangle* rect = new Rectangle(4.0, 5.0);
    Circle* circle = new Circle(2.0);

    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes[0] == rect);

    canvas.add(circle);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == circle);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;
    
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    // Add a valid shape, then null, ensuring count remains unchanged
    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);
    
    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    
    // Rectangle area: 3 * 4 = 12.0
    canvas.add(new Rectangle(3.0, 4.0));
    // Circle radius 1.0 area: PI * 1.0 * 1.0 ~= 3.141592653589793
    canvas.add(new Circle(1.0));
    // Generic shape base area: 0.0
    canvas.add(new Shape());

    double expectedArea = 12.0 + (3.14159265358979323846 * 1.0 * 1.0) + 0.0;
    assert(isClose(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    
    // Empty canvas
    assert(isClose(canvas.largestArea(), 0.0));

    // Add smaller shape (area = 10.0)
    canvas.add(new Rectangle(2.0, 5.0));
    assert(isClose(canvas.largestArea(), 10.0));

    // Add larger shape (area = 50.0)
    canvas.add(new Rectangle(5.0, 10.0));
    assert(isClose(canvas.largestArea(), 50.0));

    // Add intermediate shape (area ~= 28.2743)
    canvas.add(new Circle(3.0));
    assert(isClose(canvas.largestArea(), 50.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    TrackedShape::deleteCount = 0;
    Canvas canvas;

    canvas.add(new TrackedShape(5.0));
    canvas.add(new TrackedShape(15.0));
    assert(canvas.count() == 2);

    canvas.clear();

    // Verify properties after explicit clear()
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));
    assert(TrackedShape::deleteCount == 2);

    // Ensure clear on empty canvas does nothing harmful
    canvas.clear();
    assert(canvas.count() == 0);
    assert(TrackedShape::deleteCount == 2);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    TrackedShape::deleteCount = 0;
    {
        Canvas canvas;
        canvas.add(new TrackedShape(10.0));
        canvas.add(new TrackedShape(20.0));
        canvas.add(new TrackedShape(30.0));
        assert(canvas.count() == 3);
    } // canvas goes out of scope here; destructor should call clear()

    assert(TrackedShape::deleteCount == 3);

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "--- Running Canvas Test Suite ---" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullShape();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "--- All Canvas tests passed successfully! ---" << std::endl;
    return 0;
}