#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function to compare floating point numbers with a tolerance
bool is_close(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// Test initial state of Canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(is_close(canvas.totalArea(), 0.0));
    assert(is_close(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding null pointer
void test_Canvas_addNull() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    std::cout << "Test test_Canvas_addNull: PASS" << std::endl;
}

// Test adding valid shapes and verifying count
void test_Canvas_addValidShapes() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(1.0));
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

// Test totalArea calculation
void test_Canvas_totalArea() {
    Canvas canvas;
    
    // Rectangle 2x3 => area = 6.0
    canvas.add(new Rectangle(2.0, 3.0));
    assert(is_close(canvas.totalArea(), 6.0));

    // Circle r=2 => area = PI * 4.0 ≈ 12.5663706
    canvas.add(new Circle(2.0));
    double expectedArea = 6.0 + (3.14159265358979323846 * 4.0);
    assert(is_close(canvas.totalArea(), expectedArea));

    // Generic shape with area 0.0
    canvas.add(new Shape());
    assert(is_close(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largestArea determination
void test_Canvas_largestArea() {
    Canvas canvas;
    
    // Empty canvas largestArea should be 0.0
    assert(is_close(canvas.largestArea(), 0.0));

    // Add rectangle with area 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(is_close(canvas.largestArea(), 10.0));

    // Add rectangle with area 50.0 (new largest)
    canvas.add(new Rectangle(5.0, 10.0));
    assert(is_close(canvas.largestArea(), 50.0));

    // Add circle with area PI * 3^2 ≈ 28.27 (less than largest)
    canvas.add(new Circle(3.0));
    assert(is_close(canvas.largestArea(), 50.0));

    // Add circle with area PI * 5^2 ≈ 78.54 (new largest)
    canvas.add(new Circle(5.0));
    double expectedLargest = 3.14159265358979323846 * 25.0;
    assert(is_close(canvas.largestArea(), expectedLargest));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clear functionality
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(4.0, 5.0));
    canvas.add(new Circle(2.5));
    canvas.add(new Shape());

    assert(canvas.count() == 3);
    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(is_close(canvas.totalArea(), 0.0));
    assert(is_close(canvas.largestArea(), 0.0));

    // Calling clear again on an empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test destructor cleans up memory properly (executing clear)
void test_Canvas_destructor() {
    // Canvas destructor is tested via scoped allocation
    {
        Canvas canvas;
        canvas.add(new Rectangle(1.0, 2.0));
        canvas.add(new Circle(3.0));
    } // canvas goes out of scope here and destructor should free memory safely

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNull();
    test_Canvas_addValidShapes();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}