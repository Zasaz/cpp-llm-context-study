#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"
#include "Vector2.h"

// Helper function for floating-point comparison
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) <= epsilon;
}

// Test initial state of a newly constructed Canvas
void test_Canvas_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

// Test adding null pointers to the Canvas
void test_Canvas_addNullShape() {
    Canvas canvas;
    canvas.add(0);
    canvas.add(nullptr);

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_addNullShape: PASS" << std::endl;
}

// Test adding valid shapes and verifying count
void test_Canvas_addAndCount() {
    Canvas canvas;

    canvas.add(new Rectangle(4.0, 5.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);

    // Adding null should not change count
    canvas.add(nullptr);
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_addAndCount: PASS" << std::endl;
}

// Test totalArea calculation across different combinations of shapes
void test_Canvas_totalArea() {
    Canvas canvas;
    // Empty canvas total area is 0
    assert(approxEqual(canvas.totalArea(), 0.0));

    // Rectangle: 4.0 * 5.0 = 20.0
    canvas.add(new Rectangle(4.0, 5.0));
    assert(approxEqual(canvas.totalArea(), 20.0));

    // Circle: PI * 3.0^2 = 9 * PI ≈ 28.2743338823
    const double PI = 3.14159265358979323846;
    canvas.add(new Circle(3.0));
    double expectedArea = 20.0 + (PI * 9.0);
    assert(approxEqual(canvas.totalArea(), expectedArea));

    // Base Shape has area 0.0
    canvas.add(new Shape());
    assert(approxEqual(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// Test largestArea calculation with various configurations
void test_Canvas_largestArea() {
    // Case 1: Empty canvas
    Canvas canvas1;
    assert(approxEqual(canvas1.largestArea(), 0.0));

    // Case 2: Only shapes with 0 area
    canvas1.add(new Shape());
    assert(approxEqual(canvas1.largestArea(), 0.0));

    // Case 3: Multiple shapes where the largest is in the middle
    Canvas canvas2;
    canvas2.add(new Rectangle(2.0, 2.0));    // Area = 4.0
    canvas2.add(new Rectangle(10.0, 10.0));  // Area = 100.0 (Largest)
    canvas2.add(new Circle(1.0));            // Area ≈ 3.14159
    assert(approxEqual(canvas2.largestArea(), 100.0));

    // Case 4: Largest shape is the first one
    Canvas canvas3;
    canvas3.add(new Rectangle(20.0, 10.0));  // Area = 200.0 (Largest)
    canvas3.add(new Rectangle(5.0, 5.0));    // Area = 25.0
    canvas3.add(new Circle(2.0));            // Area ≈ 12.566
    assert(approxEqual(canvas3.largestArea(), 200.0));

    // Case 5: Largest shape is the last one
    Canvas canvas4;
    canvas4.add(new Rectangle(2.0, 3.0));    // Area = 6.0
    canvas4.add(new Circle(2.0));            // Area ≈ 12.566
    canvas4.add(new Circle(5.0));            // Area ≈ 78.5398 (Largest)
    const double PI = 3.14159265358979323846;
    assert(approxEqual(canvas4.largestArea(), PI * 25.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// Test clear() method resets the canvas and allows reuse
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new Rectangle(10.0, 5.0));
    canvas.add(new Circle(4.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Calling clear on an empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    // Reuse canvas after clear
    canvas.add(new Rectangle(3.0, 3.0));
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 9.0));
    assert(approxEqual(canvas.largestArea(), 9.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Custom mock shape to verify destructor invocation during clear / destruction
class DestructorTrackerShape : public Shape {
public:
    bool* destroyedFlag;

    DestructorTrackerShape(bool* flag) : destroyedFlag(flag) {
        if (destroyedFlag) {
            *destroyedFlag = false;
        }
    }

    ~DestructorTrackerShape() {
        if (destroyedFlag) {
            *destroyedFlag = true;
        }
    }

    double area() const {
        return 42.0;
    }
};

// Test that Canvas destructor properly frees memory of owned shapes
void test_Canvas_destructorCleansMemory() {
    bool shape1Destroyed = false;
    bool shape2Destroyed = false;

    {
        Canvas canvas;
        canvas.add(new DestructorTrackerShape(&shape1Destroyed));
        canvas.add(new DestructorTrackerShape(&shape2Destroyed));
        assert(!shape1Destroyed);
        assert(!shape2Destroyed);
    } // canvas goes out of scope here; destructor should be invoked

    assert(shape1Destroyed);
    assert(shape2Destroyed);

    std::cout << "Test test_Canvas_destructorCleansMemory: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Test Suite ===" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullShape();
    test_Canvas_addAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructorCleansMemory();

    std::cout << "=== All Canvas tests passed successfully! ===" << std::endl;
    return 0;
}