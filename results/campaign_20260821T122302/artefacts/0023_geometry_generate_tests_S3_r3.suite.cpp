#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparison
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) <= epsilon;
}

void test_canvas_default_constructor() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.totalArea() == 0.0);
    assert(canvas.largestArea() == 0.0);
    assert(canvas.shapes.empty());

    std::cout << "Test test_canvas_default_constructor: PASS" << std::endl;
}

void test_canvas_add_null_pointer() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(canvas.totalArea() == 0.0);
    assert(canvas.largestArea() == 0.0);

    std::cout << "Test test_canvas_add_null_pointer: PASS" << std::endl;
}

void test_canvas_add_and_count() {
    Canvas canvas;

    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);
    assert(canvas.shapes.size() == 1);

    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);
    assert(canvas.shapes.size() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);
    assert(canvas.shapes.size() == 3);

    // Adding null should not increase the count
    canvas.add(nullptr);
    assert(canvas.count() == 3);

    std::cout << "Test test_canvas_add_and_count: PASS" << std::endl;
}

void test_canvas_total_area() {
    Canvas canvas;

    // Rectangle: 4.0 * 5.0 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    assert(approxEqual(canvas.totalArea(), 20.0));

    // Base Shape: area = 0.0
    canvas.add(new Shape());
    assert(approxEqual(canvas.totalArea(), 20.0));

    // Circle: PI * 2^2 = ~12.566370614
    const double expectedCircleArea = 3.14159265358979323846 * 4.0;
    canvas.add(new Circle(2.0));
    assert(approxEqual(canvas.totalArea(), 20.0 + expectedCircleArea));

    // Rectangle: 2.5 * 2.0 = 5.0
    canvas.add(new Rectangle(2.5, 2.0));
    assert(approxEqual(canvas.totalArea(), 20.0 + expectedCircleArea + 5.0));

    std::cout << "Test test_canvas_total_area: PASS" << std::endl;
}

void test_canvas_largest_area() {
    Canvas canvas;

    // Empty canvas largestArea should be 0.0
    assert(canvas.largestArea() == 0.0);

    // Add generic shape with 0.0 area
    canvas.add(new Shape());
    assert(canvas.largestArea() == 0.0);

    // Add Rectangle (area = 10.0) -> largest should be 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 10.0));

    // Add smaller Circle (radius = 1.0, area = ~3.14159) -> largest should still be 10.0
    canvas.add(new Circle(1.0));
    assert(approxEqual(canvas.largestArea(), 10.0));

    // Add larger Rectangle (area = 50.0) -> largest should update to 50.0
    canvas.add(new Rectangle(10.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // Add even larger Circle (radius = 5.0, area = ~78.5398) -> largest should update
    const double largeCircleArea = 3.14159265358979323846 * 25.0;
    canvas.add(new Circle(5.0));
    assert(approxEqual(canvas.largestArea(), largeCircleArea));

    std::cout << "Test test_canvas_largest_area: PASS" << std::endl;
}

void test_canvas_clear_and_reuse() {
    Canvas canvas;

    canvas.add(new Rectangle(3.0, 4.0));
    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);

    // Clear should delete elements and empty vector
    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.totalArea() == 0.0);
    assert(canvas.largestArea() == 0.0);
    assert(canvas.shapes.empty());

    // Canvas should remain usable after clear
    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 6.0));
    assert(approxEqual(canvas.largestArea(), 6.0));

    // Clearing an already empty canvas should not crash
    canvas.clear();
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_canvas_clear_and_reuse: PASS" << std::endl;
}

void test_canvas_destructor_lifecycle() {
    // Test that a Canvas object can properly destruct shapes on scope exit
    {
        Canvas localCanvas;
        localCanvas.add(new Rectangle(1.0, 1.0));
        localCanvas.add(new Circle(1.0));
        localCanvas.add(new Shape());
        assert(localCanvas.count() == 3);
    }
    // Destructor runs here without memory leaks or double free errors

    std::cout << "Test test_canvas_destructor_lifecycle: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_canvas_default_constructor();
    test_canvas_add_null_pointer();
    test_canvas_add_and_count();
    test_canvas_total_area();
    test_canvas_largest_area();
    test_canvas_clear_and_reuse();
    test_canvas_destructor_lifecycle();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}