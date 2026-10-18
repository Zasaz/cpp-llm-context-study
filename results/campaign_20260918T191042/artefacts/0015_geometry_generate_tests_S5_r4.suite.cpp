#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Custom shape to test polymorphism and destructor invocation
class DestructorTrackerShape : public Shape {
public:
    bool* destroyedFlag;

    DestructorTrackerShape(bool* flag) : destroyedFlag(flag) {
        if (destroyedFlag) {
            *destroyedFlag = false;
        }
    }

    virtual ~DestructorTrackerShape() {
        if (destroyedFlag) {
            *destroyedFlag = true;
        }
    }

    virtual double area() const {
        return 42.0;
    }
};

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addValidShapes() {
    Canvas canvas;
    
    Rectangle* rect = new Rectangle(5.0, 4.0);
    Circle* circle = new Circle(3.0);

    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes[0] == rect);

    canvas.add(circle);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == circle);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_addNullPointer() {
    Canvas canvas;
    
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    Rectangle* rect = new Rectangle(2.0, 3.0);
    canvas.add(rect);
    assert(canvas.count() == 1);

    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    
    // Empty canvas area
    assert(approxEqual(canvas.totalArea(), 0.0));

    // Rectangle: 4 * 5 = 20
    canvas.add(new Rectangle(4.0, 5.0));
    assert(approxEqual(canvas.totalArea(), 20.0));

    // Circle: pi * 2^2 ≈ 12.566370614
    canvas.add(new Circle(2.0));
    double expectedTotal = 20.0 + (3.14159265358979323846 * 4.0);
    assert(approxEqual(canvas.totalArea(), expectedTotal));

    // Generic shape: area 0.0
    canvas.add(new Shape());
    assert(approxEqual(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;

    // Empty canvas largest area is 0.0
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Add shapes with varying areas
    canvas.add(new Rectangle(2.0, 3.0)); // area = 6.0
    assert(approxEqual(canvas.largestArea(), 6.0));

    canvas.add(new Rectangle(5.0, 5.0)); // area = 25.0
    assert(approxEqual(canvas.largestArea(), 25.0));

    canvas.add(new Circle(2.0));        // area ≈ 12.566
    assert(approxEqual(canvas.largestArea(), 25.0));

    canvas.add(new Rectangle(10.0, 10.0)); // area = 100.0
    assert(approxEqual(canvas.largestArea(), 100.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    bool destroyed1 = false;
    bool destroyed2 = false;

    canvas.add(new DestructorTrackerShape(&destroyed1));
    canvas.add(new DestructorTrackerShape(&destroyed2));

    assert(canvas.count() == 2);
    assert(!destroyed1);
    assert(!destroyed2);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(destroyed1);
    assert(destroyed2);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Calling clear again on empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructorCleansMemory() {
    bool destroyed1 = false;
    bool destroyed2 = false;

    {
        Canvas canvas;
        canvas.add(new DestructorTrackerShape(&destroyed1));
        canvas.add(new DestructorTrackerShape(&destroyed2));
        assert(!destroyed1);
        assert(!destroyed2);
    } // Canvas goes out of scope here, calling ~Canvas() which calls clear()

    assert(destroyed1);
    assert(destroyed2);

    std::cout << "Test test_Canvas_destructorCleansMemory: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullPointer();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructorCleansMemory();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}