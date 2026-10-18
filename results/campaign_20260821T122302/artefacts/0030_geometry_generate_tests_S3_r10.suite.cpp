#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
static bool almost_equal(double a, double b, double epsilon = 1e-5) {
    return std::fabs(a - b) < epsilon;
}

// Test initial state of a newly created Canvas
void test_Canvas_defaultConstructor() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(almost_equal(canvas.totalArea(), 0.0));
    assert(almost_equal(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_defaultConstructor: PASS" << std::endl;
}

// Test adding null pointers
void test_Canvas_addNullPointer() {
    Canvas canvas;
    canvas.add(nullptr);
    canvas.add(0);

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(almost_equal(canvas.totalArea(), 0.0));
    assert(almost_equal(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

// Test adding valid shapes and tracking count
void test_Canvas_addAndCount() {
    Canvas canvas;

    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);

    // Attempting to add null should not increment count
    canvas.add(nullptr);
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addAndCount: PASS" << std::endl;
}

// Test calculating total area of diverse shapes
void test_Canvas_totalArea() {
    Canvas canvas;
    const double PI = 3.14159265358979323846;

    // Empty canvas total area
    assert(almost_equal(canvas.totalArea(), 0.0));

    // Add rectangle: area = 2.0 * 3.0 = 6.0
    canvas.add(new Rectangle(2.0, 3.0));
    assert(almost_equal(canvas.totalArea(), 6.0));

    // Add circle: area = PI * 2.0^2 = 4.0 * PI
    canvas.add(new Circle(2.0));
    double expectedTotal = 6.0 + (4.0 * PI);
    assert(almost_equal(canvas.totalArea(), expectedTotal));

    // Add base generic shape: area = 0.0
    canvas.add(new Shape());
    assert(almost_equal(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test finding the largest area among shapes
void test_Canvas_largestArea() {
    const double PI = 3.14159265358979323846;

    // 1. Empty canvas
    {
        Canvas canvas;
        assert(almost_equal(canvas.largestArea(), 0.0));
    }

    // 2. Largest is the first element
    {
        Canvas canvas;
        canvas.add(new Rectangle(10.0, 10.0)); // area = 100.0
        canvas.add(new Circle(2.0));           // area = ~12.566
        canvas.add(new Rectangle(2.0, 3.0));   // area = 6.0

        assert(almost_equal(canvas.largestArea(), 100.0));
    }

    // 3. Largest is the middle element
    {
        Canvas canvas;
        canvas.add(new Rectangle(2.0, 3.0));   // area = 6.0
        canvas.add(new Circle(5.0));           // area = 25 * PI = ~78.5398
        canvas.add(new Rectangle(5.0, 5.0));   // area = 25.0

        assert(almost_equal(canvas.largestArea(), 25.0 * PI));
    }

    // 4. Largest is the last element
    {
        Canvas canvas;
        canvas.add(new Rectangle(1.0, 1.0));   // area = 1.0
        canvas.add(new Circle(1.0));           // area = PI
        canvas.add(new Rectangle(8.0, 8.0));   // area = 64.0

        assert(almost_equal(canvas.largestArea(), 64.0));
    }

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clearing shapes from Canvas and reusability
void test_Canvas_clearAndReuse() {
    Canvas canvas;

    // Clearing an empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    // Add shapes
    canvas.add(new Rectangle(4.0, 5.0));
    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);

    // Clear populated canvas
    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(almost_equal(canvas.totalArea(), 0.0));
    assert(almost_equal(canvas.largestArea(), 0.0));

    // Re-add shapes after clear
    canvas.add(new Rectangle(3.0, 3.0));
    assert(canvas.count() == 1);
    assert(almost_equal(canvas.totalArea(), 9.0));
    assert(almost_equal(canvas.largestArea(), 9.0));

    std::cout << "Test test_Canvas_clearAndReuse: PASS" << std::endl;
}

// Test proper destruction without memory leaks / double frees
void test_Canvas_destructor() {
    // Dynamically allocated Canvas containing shapes
    Canvas* canvas = new Canvas();
    canvas->add(new Rectangle(10.0, 2.0));
    canvas->add(new Circle(4.0));
    canvas->add(new Shape());

    assert(canvas->count() == 3);

    // Destructor should invoke clear() and free all allocated shapes
    delete canvas;

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_defaultConstructor();
    test_Canvas_addNullPointer();
    test_Canvas_addAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clearAndReuse();
    test_Canvas_destructor();

    std::cout << "=== All Canvas Unit Tests Passed Successfully ===" << std::endl;
    return 0;
}