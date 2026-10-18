#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

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
    
    Rectangle* rect1 = new Rectangle(4.0, 5.0);
    canvas.add(rect1);
    assert(canvas.count() == 1);
    assert(canvas.shapes.size() == 1);
    assert(canvas.shapes[0] == rect1);

    Circle* circle1 = new Circle(3.0);
    canvas.add(circle1);
    assert(canvas.count() == 2);
    assert(canvas.shapes.size() == 2);
    assert(canvas.shapes[1] == circle1);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(nullptr);
    canvas.add(0);
    
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    // Add a valid shape, then null, ensuring only valid shapes are added
    canvas.add(new Rectangle(2.0, 3.0));
    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    
    // Empty canvas area
    assert(isClose(canvas.totalArea(), 0.0));

    // Add Rectangle (area = 20.0)
    canvas.add(new Rectangle(4.0, 5.0));
    assert(isClose(canvas.totalArea(), 20.0));

    // Add Circle with radius = 2.0 (area = PI * 4 = 12.566370614)
    canvas.add(new Circle(2.0));
    double expectedCircleArea = 3.14159265358979323846 * 4.0;
    assert(isClose(canvas.totalArea(), 20.0 + expectedCircleArea));

    // Add generic base Shape (area = 0.0)
    canvas.add(new Shape());
    assert(isClose(canvas.totalArea(), 20.0 + expectedCircleArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;

    // Empty canvas largestArea
    assert(isClose(canvas.largestArea(), 0.0));

    // Add shape with area = 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(isClose(canvas.largestArea(), 10.0));

    // Add shape with area = 50.0 (new largest)
    canvas.add(new Rectangle(5.0, 10.0));
    assert(isClose(canvas.largestArea(), 50.0));

    // Add shape with smaller area = 25.0
    canvas.add(new Rectangle(5.0, 5.0));
    assert(isClose(canvas.largestArea(), 50.0));

    // Add Circle with radius = 10.0 (area ≈ 314.159, new largest)
    canvas.add(new Circle(10.0));
    double expectedCircleArea = 3.14159265358979323846 * 100.0;
    assert(isClose(canvas.largestArea(), expectedCircleArea));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(3.0, 4.0));
    canvas.add(new Circle(5.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Clearing an already empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    // Can add new shapes after clearing
    canvas.add(new Rectangle(1.0, 2.0));
    assert(canvas.count() == 1);
    assert(isClose(canvas.totalArea(), 2.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructorCleanup() {
    // Tests that creating and destructing Canvas with dynamically allocated shapes works cleanly
    {
        Canvas canvas;
        canvas.add(new Rectangle(10.0, 20.0));
        canvas.add(new Circle(5.0));
        canvas.add(new Shape());
        assert(canvas.count() == 3);
    } // canvas goes out of scope here, triggering destructor and clear()

    std::cout << "Test test_Canvas_destructorCleanup: PASS" << std::endl;
}

int main() {
    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullShape();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructorCleanup();

    std::cout << "\nAll Canvas tests passed successfully!" << std::endl;
    return 0;
}