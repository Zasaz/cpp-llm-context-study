#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for comparing floating point numbers
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Custom shape to test polymorphism and destructor invocation
class DestructorTrackerShape : public Shape {
public:
    bool* destroyedFlag;
    double customArea;

    DestructorTrackerShape(bool* flag, double a) : destroyedFlag(flag), customArea(a) {
        if (destroyedFlag) {
            *destroyedFlag = false;
        }
    }

    virtual ~DestructorTrackerShape() {
        if (destroyedFlag) {
            *destroyedFlag = true;
        }
    }

    double area() const override {
        return customArea;
    }
};

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
    
    Rectangle* rect = new Rectangle(4.0, 5.0); // area = 20.0
    Circle* circ = new Circle(3.0);            // area = PI * 9 ~ 28.27433

    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes[0] == rect);

    canvas.add(circ);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == circ);

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
    
    // Empty canvas total area
    assert(isClose(canvas.totalArea(), 0.0));

    // Add rectangle: area = 2.0 * 3.0 = 6.0
    canvas.add(new Rectangle(2.0, 3.0));
    assert(isClose(canvas.totalArea(), 6.0));

    // Add circle: area = PI * 2.0 * 2.0 = 4.0 * PI
    canvas.add(new Circle(2.0));
    double expectedArea = 6.0 + (3.14159265358979323846 * 4.0);
    assert(isClose(canvas.totalArea(), expectedArea));

    // Add generic Shape: area = 0.0
    canvas.add(new Shape());
    assert(isClose(canvas.totalArea(), expectedArea));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;

    // Empty canvas largest area
    assert(isClose(canvas.largestArea(), 0.0));

    // Add shapes with varying areas
    canvas.add(new Rectangle(2.0, 5.0)); // area = 10.0
    assert(isClose(canvas.largestArea(), 10.0));

    canvas.add(new Rectangle(10.0, 10.0)); // area = 100.0
    assert(isClose(canvas.largestArea(), 100.0));

    canvas.add(new Circle(1.0)); // area = PI ~ 3.14 (smaller)
    assert(isClose(canvas.largestArea(), 100.0));

    canvas.add(new Rectangle(20.0, 10.0)); // area = 200.0 (new largest)
    assert(isClose(canvas.largestArea(), 200.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    bool destroyed1 = false;
    bool destroyed2 = false;

    canvas.add(new DestructorTrackerShape(&destroyed1, 10.0));
    canvas.add(new DestructorTrackerShape(&destroyed2, 20.0));

    assert(canvas.count() == 2);
    assert(!destroyed1);
    assert(!destroyed2);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(destroyed1);
    assert(destroyed2);
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Calling clear on already empty canvas should not crash
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    bool destroyed1 = false;
    bool destroyed2 = false;

    {
        Canvas canvas;
        canvas.add(new DestructorTrackerShape(&destroyed1, 5.0));
        canvas.add(new DestructorTrackerShape(&destroyed2, 15.0));
        assert(!destroyed1);
        assert(!destroyed2);
    } // canvas goes out of scope here and destructor should be invoked

    assert(destroyed1);
    assert(destroyed2);

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

void test_Canvas_multipleCycles() {
    Canvas canvas;

    // Cycle 1
    canvas.add(new Rectangle(5.0, 5.0)); // area = 25.0
    assert(canvas.count() == 1);
    assert(isClose(canvas.totalArea(), 25.0));
    canvas.clear();
    assert(canvas.count() == 0);

    // Cycle 2
    canvas.add(new Circle(1.0)); // area = PI
    canvas.add(new Circle(2.0)); // area = 4 * PI
    assert(canvas.count() == 2);
    assert(isClose(canvas.largestArea(), 4.0 * 3.14159265358979323846));
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_multipleCycles: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Canvas Unit Tests..." << std::endl;

    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullPointer();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();
    test_Canvas_multipleCycles();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}