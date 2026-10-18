#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
static bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test initial default state of a Canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding valid shapes to the Canvas
void test_Canvas_addShapes() {
    Canvas canvas;
    Rectangle* rect = new Rectangle(4.0, 5.0);
    Circle* circle = new Circle(3.0);

    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes[0] == rect);

    canvas.add(circle);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == circle);

    std::cout << "Test test_Canvas_addShapes: PASS" << std::endl;
}

// Test handling of null pointer when adding shapes
void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    // Add a valid shape, then null, ensuring count only increments for valid shapes
    canvas.add(new Rectangle(2.0, 2.0));
    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

// Test calculation of total area across multiple shapes
void test_Canvas_totalArea() {
    Canvas canvas;
    
    // Total area of empty canvas
    assert(isClose(canvas.totalArea(), 0.0));

    // Area: 10.0 * 5.0 = 50.0
    canvas.add(new Rectangle(10.0, 5.0));
    assert(isClose(canvas.totalArea(), 50.0));

    // Area: pi * 2^2 ≈ 12.566370614
    canvas.add(new Circle(2.0));
    double expectedArea = 50.0 + (3.14159265358979323846 * 4.0);
    assert(isClose(canvas.totalArea(), expectedArea));

    // Add base Shape (area is 0.0)
    canvas.add(new Shape());
    assert(isClose(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largest area computation among shapes
void test_Canvas_largestArea() {
    Canvas canvas;

    // Largest area of empty canvas
    assert(isClose(canvas.largestArea(), 0.0));

    // Largest with single shape
    canvas.add(new Rectangle(2.0, 3.0)); // area = 6.0
    assert(isClose(canvas.largestArea(), 6.0));

    // Add smaller shape
    canvas.add(new Rectangle(1.0, 2.0)); // area = 2.0
    assert(isClose(canvas.largestArea(), 6.0));

    // Add shape with larger area
    canvas.add(new Circle(2.0)); // area = pi * 4 ≈ 12.566
    double expectedLargest = 3.14159265358979323846 * 4.0;
    assert(isClose(canvas.largestArea(), expectedLargest));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clearing shapes and memory cleanup
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(3.0, 4.0));
    canvas.add(new Circle(1.5));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Verify clear on empty canvas is safe
    canvas.clear();
    assert(canvas.count() == 0);

    // Verify canvas can be reused after clear
    canvas.add(new Rectangle(2.0, 2.0));
    assert(canvas.count() == 1);
    assert(isClose(canvas.totalArea(), 4.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test dynamic allocation and destructor cleanup
void test_Canvas_destructor() {
    Canvas* dynamicCanvas = new Canvas();
    dynamicCanvas->add(new Rectangle(10.0, 10.0));
    dynamicCanvas->add(new Circle(5.0));
    
    // Deleting canvas invokes ~Canvas() which calls clear() and frees shapes
    delete dynamicCanvas;

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    test_Canvas_initialState();
    test_Canvas_addShapes();
    test_Canvas_addNullShape();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "\nAll Canvas tests passed successfully!" << std::endl;
    return 0;
}