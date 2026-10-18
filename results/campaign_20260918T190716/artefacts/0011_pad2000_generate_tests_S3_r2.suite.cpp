#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for comparing floating-point numbers with tolerance
bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test 1: Verify default initialization of Canvas
void test_Canvas_constructor_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());
    
    std::cout << "Test test_Canvas_constructor_initialState: PASS" << std::endl;
}

// Test 2: Verify adding valid shapes and handling nullptr/null pointers
void test_Canvas_add_and_count() {
    Canvas canvas;
    
    // Add valid shape
    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);
    
    // Add null pointer (should not be added)
    canvas.add(nullptr);
    assert(canvas.count() == 1);
    
    // Add another shape
    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);
    
    std::cout << "Test test_Canvas_add_and_count: PASS" << std::endl;
}

// Test 3: Verify totalArea calculation across different shapes
void test_Canvas_totalArea_multipleShapes() {
    Canvas canvas;
    
    // Total area for empty canvas should be 0
    assert(areClose(canvas.totalArea(), 0.0));
    
    // Rectangle area = 3 * 4 = 12
    canvas.add(new Rectangle(3.0, 4.0));
    assert(areClose(canvas.totalArea(), 12.0));
    
    // Circle area = pi * 1^2 approx 3.14159265
    canvas.add(new Circle(1.0));
    double expectedArea = 12.0 + (3.14159265358979323846 * 1.0 * 1.0);
    assert(areClose(canvas.totalArea(), expectedArea));
    
    // Base generic shape area = 0
    canvas.add(new Shape());
    assert(areClose(canvas.totalArea(), expectedArea));
    
    std::cout << "Test test_Canvas_totalArea_multipleShapes: PASS" << std::endl;
}

// Test 4: Verify largestArea correctly identifies the maximum shape area
void test_Canvas_largestArea_variousOrders() {
    // Case 1: Empty canvas
    {
        Canvas canvas;
        assert(areClose(canvas.largestArea(), 0.0));
    }
    
    // Case 2: Shapes in increasing area order
    {
        Canvas canvas;
        canvas.add(new Rectangle(2.0, 2.0)); // area = 4
        canvas.add(new Rectangle(3.0, 3.0)); // area = 9
        canvas.add(new Rectangle(4.0, 4.0)); // area = 16
        assert(areClose(canvas.largestArea(), 16.0));
    }
    
    // Case 3: Shapes with the maximum in the middle
    {
        Canvas canvas;
        canvas.add(new Rectangle(2.0, 2.0));  // area = 4
        canvas.add(new Rectangle(10.0, 5.0)); // area = 50
        canvas.add(new Circle(2.0));          // area ≈ 12.566
        assert(areClose(canvas.largestArea(), 50.0));
    }
    
    std::cout << "Test test_Canvas_largestArea_variousOrders: PASS" << std::endl;
}

// Test 5: Verify clear function empties the collection and resets metrics
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(5.0, 5.0));
    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);
    
    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    
    // Verify that the canvas can still be used normally after clear
    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);
    assert(areClose(canvas.totalArea(), 6.0));
    
    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test 6: Verify destructor cleans up without crashing / double free
void test_Canvas_destructor_scope() {
    {
        Canvas canvas;
        canvas.add(new Rectangle(10.0, 2.0));
        canvas.add(new Circle(4.0));
        // Destructor is implicitly called when going out of scope
    }
    // If memory is properly freed and no segfault occurs, test passes
    std::cout << "Test test_Canvas_destructor_scope: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;
    
    test_Canvas_constructor_initialState();
    test_Canvas_add_and_count();
    test_Canvas_totalArea_multipleShapes();
    test_Canvas_largestArea_variousOrders();
    test_Canvas_clear();
    test_Canvas_destructor_scope();
    
    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}