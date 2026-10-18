#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test initial state of Canvas upon creation
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());
    
    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding null pointer
void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    
    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

// Test adding valid shapes and verifying count
void test_Canvas_addShapesAndCount() {
    Canvas canvas;
    
    Rectangle* r1 = new Rectangle(4.0, 5.0);
    Circle* c1 = new Circle(3.0);
    
    canvas.add(r1);
    assert(canvas.count() == 1);
    
    canvas.add(c1);
    assert(canvas.count() == 2);
    
    assert(canvas.shapes[0] == r1);
    assert(canvas.shapes[1] == c1);
    
    std::cout << "Test test_Canvas_addShapesAndCount: PASS" << std::endl;
}

// Test totalArea calculation
void test_Canvas_totalArea() {
    Canvas canvas;
    
    // Rectangle: 4 * 5 = 20
    canvas.add(new Rectangle(4.0, 5.0));
    assert(approxEqual(canvas.totalArea(), 20.0));
    
    // Circle: PI * 2^2 = 4 * PI ≈ 12.566370614
    canvas.add(new Circle(2.0));
    double expectedArea = 20.0 + (3.14159265358979323846 * 4.0);
    assert(approxEqual(canvas.totalArea(), expectedArea));
    
    // Generic Shape with 0 area
    canvas.add(new Shape());
    assert(approxEqual(canvas.totalArea(), expectedArea));
    
    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largestArea computation with various configurations
void test_Canvas_largestArea() {
    Canvas canvas;
    assert(approxEqual(canvas.largestArea(), 0.0));
    
    // Add Rectangle: area = 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 10.0));
    
    // Add Circle: area = PI * 3^2 ≈ 28.27433
    canvas.add(new Circle(3.0));
    double circleArea = 3.14159265358979323846 * 9.0;
    assert(approxEqual(canvas.largestArea(), circleArea));
    
    // Add smaller Rectangle: area = 6.0
    canvas.add(new Rectangle(2.0, 3.0));
    assert(approxEqual(canvas.largestArea(), circleArea));
    
    // Add larger Rectangle: area = 50.0
    canvas.add(new Rectangle(10.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 50.0));
    
    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clear method
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0));
    canvas.add(new Circle(1.5));
    canvas.add(new Shape());
    
    assert(canvas.count() == 3);
    
    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    
    // Ensure we can add again after clearing
    canvas.add(new Rectangle(3.0, 3.0));
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 9.0));
    assert(approxEqual(canvas.largestArea(), 9.0));
    
    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test destructor cleanup (proper destruction without memory leaks / errors)
void test_Canvas_destructor() {
    {
        Canvas canvas;
        canvas.add(new Rectangle(1.0, 2.0));
        canvas.add(new Circle(2.5));
        canvas.add(new Shape());
    } // canvas goes out of scope here and destructor calls clear()
    
    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;
    
    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addShapesAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();
    
    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}