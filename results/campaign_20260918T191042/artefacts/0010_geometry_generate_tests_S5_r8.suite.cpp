#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function to check approximate floating-point equality
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) <= epsilon;
}

// Test initial/empty state of Canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    
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
void test_Canvas_addShapesAndCount() {
    Canvas canvas;
    
    Shape* r1 = new Rectangle(4.0, 5.0);
    Shape* c1 = new Circle(3.0);
    
    canvas.add(r1);
    assert(canvas.count() == 1);
    assert(canvas.shapes[0] == r1);
    
    canvas.add(c1);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == c1);
    
    // Attempting to add null should not alter count or contents
    canvas.add(nullptr);
    assert(canvas.count() == 2);
    
    std::cout << "Test test_Canvas_addShapesAndCount: PASS" << std::endl;
}

// Test totalArea calculation across different shapes
void test_Canvas_totalArea() {
    Canvas canvas;
    
    Shape* r = new Rectangle(2.0, 3.0);   // area = 6.0
    Shape* c = new Circle(2.0);          // area = PI * 4.0 ≈ 12.566370614
    Shape* baseShape = new Shape();      // area = 0.0
    
    canvas.add(r);
    canvas.add(c);
    canvas.add(baseShape);
    
    double expectedTotalArea = 6.0 + (3.14159265358979323846 * 4.0) + 0.0;
    assert(approxEqual(canvas.totalArea(), expectedTotalArea));
    
    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largestArea calculation with multiple shapes
void test_Canvas_largestArea() {
    Canvas canvas;
    
    Shape* r1 = new Rectangle(2.0, 2.0);  // area = 4.0
    Shape* r2 = new Rectangle(5.0, 6.0);  // area = 30.0
    Shape* c1 = new Circle(1.0);          // area = PI * 1.0 ≈ 3.14159
    
    canvas.add(r1);
    assert(approxEqual(canvas.largestArea(), 4.0));
    
    canvas.add(r2);
    assert(approxEqual(canvas.largestArea(), 30.0));
    
    canvas.add(c1);
    assert(approxEqual(canvas.largestArea(), 30.0));
    
    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clear() function
void test_Canvas_clear() {
    Canvas canvas;
    
    canvas.add(new Rectangle(3.0, 4.0));
    canvas.add(new Circle(5.0));
    assert(canvas.count() == 2);
    
    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    
    // Ensure we can continue to use the Canvas after clear()
    canvas.add(new Rectangle(10.0, 10.0));
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 100.0));
    assert(approxEqual(canvas.largestArea(), 100.0));
    
    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Test destructor cleans up without issues (tested by dynamic allocation and destruction)
void test_Canvas_destructor() {
    Canvas* canvas = new Canvas();
    canvas->add(new Rectangle(5.0, 5.0));
    canvas->add(new Circle(2.5));
    
    // Deleting canvas invokes ~Canvas() which calls clear() and frees owned Shape pointers
    delete canvas;
    
    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "--- Running Canvas Unit Tests ---" << std::endl;
    
    test_Canvas_initialState();
    test_Canvas_addNull();
    test_Canvas_addShapesAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();
    
    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}