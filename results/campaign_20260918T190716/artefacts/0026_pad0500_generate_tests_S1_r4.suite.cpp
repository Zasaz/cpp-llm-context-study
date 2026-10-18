#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
static bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_addShapesAndCount() {
    Canvas canvas;
    
    Rectangle* rect = new Rectangle(4.0, 5.0);
    Circle* circle = new Circle(3.0);

    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes.size() == 1);
    assert(canvas.shapes[0] == rect);

    canvas.add(circle);
    assert(canvas.count() == 2);
    assert(canvas.shapes.size() == 2);
    assert(canvas.shapes[1] == circle);

    // canvas destructor will free allocated shapes
    std::cout << "Test test_Canvas_addShapesAndCount: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    
    // Rectangle area = 4.0 * 5.0 = 20.0
    Rectangle* rect = new Rectangle(4.0, 5.0);
    // Circle area = PI * 2.0 * 2.0 ≈ 12.566370614359172
    Circle* circle = new Circle(2.0);
    // Generic Shape base area = 0.0
    Shape* baseShape = new Shape();

    canvas.add(rect);
    canvas.add(circle);
    canvas.add(baseShape);

    double expectedTotal = rect->area() + circle->area() + baseShape->area();
    assert(areClose(canvas.totalArea(), expectedTotal));
    
    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;

    // Initially 0 when empty
    assert(areClose(canvas.largestArea(), 0.0));

    Rectangle* rectSmall = new Rectangle(2.0, 3.0); // Area = 6.0
    Rectangle* rectLarge = new Rectangle(10.0, 10.0); // Area = 100.0
    Circle* circleMedium = new Circle(3.0); // Area = PI * 9 ≈ 28.2743

    // Add small first
    canvas.add(rectSmall);
    assert(areClose(canvas.largestArea(), 6.0));

    // Add large next
    canvas.add(rectLarge);
    assert(areClose(canvas.largestArea(), 100.0));

    // Add medium last, largest should still be 100.0
    canvas.add(circleMedium);
    assert(areClose(canvas.largestArea(), 100.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    
    canvas.add(new Rectangle(5.0, 5.0));
    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);

    // Explicit clear
    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    // Should be reusable after clear
    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);
    assert(areClose(canvas.totalArea(), 6.0));
    assert(areClose(canvas.largestArea(), 6.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructorCleanup() {
    // Test that dynamically allocating and deleting a Canvas properly cleans up its shapes without crash
    Canvas* canvas = new Canvas();
    canvas->add(new Rectangle(3.0, 3.0));
    canvas->add(new Circle(1.5));
    canvas->add(new Shape());
    
    delete canvas; // Calls destructor which calls clear()
    std::cout << "Test test_Canvas_destructorCleanup: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addShapesAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructorCleanup();

    std::cout << "=== All Canvas Tests Passed Successfully! ===" << std::endl;
    return 0;
}