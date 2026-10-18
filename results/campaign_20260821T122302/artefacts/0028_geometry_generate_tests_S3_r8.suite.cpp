#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparison
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_canvas_initial_state() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    std::cout << "Test test_canvas_initial_state: PASS" << std::endl;
}

void test_canvas_add_and_count() {
    Canvas canvas;

    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);

    std::cout << "Test test_canvas_add_and_count: PASS" << std::endl;
}

void test_canvas_add_null_pointer() {
    Canvas canvas;

    canvas.add(nullptr);
    assert(canvas.count() == 0);

    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_canvas_add_null_pointer: PASS" << std::endl;
}

void test_canvas_totalArea() {
    Canvas canvas;

    // Empty canvas area should be 0.0
    assert(approxEqual(canvas.totalArea(), 0.0));

    // Rectangle: 3 * 4 = 12.0
    canvas.add(new Rectangle(3.0, 4.0));
    assert(approxEqual(canvas.totalArea(), 12.0));

    // Circle: PI * 2^2 = 4 * PI ≈ 12.5663706
    canvas.add(new Circle(2.0));
    double expectedArea = 12.0 + (3.14159265358979323846 * 4.0);
    assert(approxEqual(canvas.totalArea(), expectedArea));

    // Base Shape has area 0.0
    canvas.add(new Shape());
    assert(approxEqual(canvas.totalArea(), expectedArea));

    std::cout << "Test test_canvas_totalArea: PASS" << std::endl;
}

void test_canvas_largestArea() {
    Canvas canvas;

    // Empty canvas largest area should be 0.0
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Add shapes with known areas
    // rect1: area = 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 10.0));

    // rect2: area = 50.0 (new largest)
    canvas.add(new Rectangle(10.0, 5.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // circle1: area = PI * 3^2 ≈ 28.2743 (less than 50.0)
    canvas.add(new Circle(3.0));
    assert(approxEqual(canvas.largestArea(), 50.0));

    // circle2: area = PI * 5^2 ≈ 78.5398 (new largest)
    canvas.add(new Circle(5.0));
    double expectedLargest = 3.14159265358979323846 * 25.0;
    assert(approxEqual(canvas.largestArea(), expectedLargest));

    std::cout << "Test test_canvas_largestArea: PASS" << std::endl;
}

void test_canvas_clear_and_reuse() {
    Canvas canvas;

    canvas.add(new Rectangle(4.0, 5.0));
    canvas.add(new Circle(3.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    // Calling clear on already empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    // Reuse canvas after clear
    canvas.add(new Rectangle(2.0, 2.0));
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 4.0));
    assert(approxEqual(canvas.largestArea(), 4.0));

    std::cout << "Test test_canvas_clear_and_reuse: PASS" << std::endl;
}

void test_canvas_destructor_scope() {
    // Test that dynamically allocated shapes are properly destroyed via destructor
    {
        Canvas canvas;
        canvas.add(new Rectangle(10.0, 20.0));
        canvas.add(new Circle(5.0));
        canvas.add(new Shape());
    } // canvas goes out of scope and destructor calls clear()

    std::cout << "Test test_canvas_destructor_scope: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_canvas_initial_state();
    test_canvas_add_and_count();
    test_canvas_add_null_pointer();
    test_canvas_totalArea();
    test_canvas_largestArea();
    test_canvas_clear_and_reuse();
    test_canvas_destructor_scope();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}