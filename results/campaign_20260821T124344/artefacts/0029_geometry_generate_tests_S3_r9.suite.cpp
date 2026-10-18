#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool isAlmostEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(isAlmostEqual(canvas.totalArea(), 0.0));
    assert(isAlmostEqual(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_addValidShapes() {
    Canvas canvas;
    Shape* rect = new Rectangle(4.0, 5.0);
    Shape* circle = new Circle(3.0);

    canvas.add(rect);
    assert(canvas.count() == 1);

    canvas.add(circle);
    assert(canvas.count() == 2);

    assert(canvas.shapes[0] == rect);
    assert(canvas.shapes[1] == circle);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    
    // Total area of empty canvas
    assert(isAlmostEqual(canvas.totalArea(), 0.0));

    // Rectangle of 4x5 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    assert(isAlmostEqual(canvas.totalArea(), 20.0));

    // Circle of radius 2: PI * 2^2 ≈ 12.566370614359172
    canvas.add(new Circle(2.0));
    double expectedArea = 20.0 + (3.14159265358979323846 * 4.0);
    assert(isAlmostEqual(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;

    // Empty canvas
    assert(isAlmostEqual(canvas.largestArea(), 0.0));

    // Add rectangle with area 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(isAlmostEqual(canvas.largestArea(), 10.0));

    // Add larger rectangle with area 50.0
    canvas.add(new Rectangle(10.0, 5.0));
    assert(isAlmostEqual(canvas.largestArea(), 50.0));

    // Add smaller circle: PI * 1^2 ≈ 3.14159
    canvas.add(new Circle(1.0));
    assert(isAlmostEqual(canvas.largestArea(), 50.0));

    // Add larger circle: PI * 5^2 ≈ 78.5398
    canvas.add(new Circle(5.0));
    double expectedLargest = 3.14159265358979323846 * 25.0;
    assert(isAlmostEqual(canvas.largestArea(), expectedLargest));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0));
    canvas.add(new Circle(1.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(isAlmostEqual(canvas.totalArea(), 0.0));
    assert(isAlmostEqual(canvas.largestArea(), 0.0));

    // Verify clear on empty canvas does not fail
    canvas.clear();
    assert(canvas.count() == 0);

    // Verify we can still add shapes after clearing
    canvas.add(new Rectangle(3.0, 3.0));
    assert(canvas.count() == 1);
    assert(isAlmostEqual(canvas.totalArea(), 9.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructorCleanup() {
    // Test that dynamically allocated Canvas cleans up without memory leaks or crash
    Canvas* canvas = new Canvas();
    canvas->add(new Rectangle(10.0, 10.0));
    canvas->add(new Circle(5.0));
    delete canvas; // Invokes ~Canvas() which calls clear()

    std::cout << "Test test_Canvas_destructorCleanup: PASS" << std::endl;
}

int main() {
    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addValidShapes();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructorCleanup();

    std::cout << "\nAll Canvas unit tests passed successfully!" << std::endl;
    return 0;
}