#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for comparing floating-point numbers
static bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addValidShapes() {
    Canvas canvas;
    
    Rectangle* rect = new Rectangle(4.0, 5.0);
    Circle* circle = new Circle(3.0);

    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes[0] == rect);

    canvas.add(circle);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == circle);

    std::cout << "Test test_Canvas_addValidShapes: PASS" << std::endl;
}

void test_Canvas_addNullShape() {
    Canvas canvas;
    
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());

    Rectangle* rect = new Rectangle(2.0, 3.0);
    canvas.add(rect);
    assert(canvas.count() == 1);

    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    
    // Total area of empty canvas
    assert(isClose(canvas.totalArea(), 0.0));

    // Base shape has 0.0 area
    canvas.add(new Shape());
    assert(isClose(canvas.totalArea(), 0.0));

    // Rectangle area: 4.0 * 5.0 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    assert(isClose(canvas.totalArea(), 20.0));

    // Circle area: PI * 2.0 * 2.0 = 4.0 * PI
    const double PI = 3.14159265358979323846;
    canvas.add(new Circle(2.0));
    double expectedTotal = 0.0 + 20.0 + (PI * 4.0);
    assert(isClose(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;
    
    // Empty canvas largest area should be 0.0
    assert(isClose(canvas.largestArea(), 0.0));

    // Add shapes of varying areas
    canvas.add(new Rectangle(2.0, 3.0)); // area = 6.0
    assert(isClose(canvas.largestArea(), 6.0));

    canvas.add(new Rectangle(5.0, 10.0)); // area = 50.0
    assert(isClose(canvas.largestArea(), 50.0));

    canvas.add(new Circle(2.0)); // area = ~12.566
    assert(isClose(canvas.largestArea(), 50.0));

    canvas.add(new Circle(5.0)); // area = ~78.5398
    const double PI = 3.14159265358979323846;
    assert(isClose(canvas.largestArea(), PI * 25.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;

    canvas.add(new Rectangle(3.0, 4.0));
    canvas.add(new Circle(1.5));
    canvas.add(new Shape());
    assert(canvas.count() == 3);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Calling clear on an already empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    // Should be able to add new shapes after clear
    canvas.add(new Rectangle(2.0, 2.0));
    assert(canvas.count() == 1);
    assert(isClose(canvas.totalArea(), 4.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Track shape destruction to verify Canvas destructor and clear() clean up memory
class DestructionTrackerShape : public Shape {
public:
    bool* destroyedFlag;

    DestructionTrackerShape(bool* flag) : destroyedFlag(flag) {
        if (destroyedFlag) {
            *destroyedFlag = false;
        }
    }

    virtual ~DestructionTrackerShape() {
        if (destroyedFlag) {
            *destroyedFlag = true;
        }
    }

    double area() const {
        return 10.0;
    }
};

void test_Canvas_destructorCleansMemory() {
    bool shape1Destroyed = false;
    bool shape2Destroyed = false;

    {
        Canvas canvas;
        canvas.add(new DestructionTrackerShape(&shape1Destroyed));
        canvas.add(new DestructionTrackerShape(&shape2Destroyed));
        assert(canvas.count() == 2);
        assert(!shape1Destroyed);
        assert(!shape2Destroyed);
    } // canvas goes out of scope here; its destructor should call clear() and delete shapes

    assert(shape1Destroyed);
    assert(shape2Destroyed);

    std::cout << "Test test_Canvas_destructorCleansMemory: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullShape();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructorCleansMemory();

    std::cout << "=== All Canvas Tests Passed Successfully ===" << std::endl;
    return 0;
}