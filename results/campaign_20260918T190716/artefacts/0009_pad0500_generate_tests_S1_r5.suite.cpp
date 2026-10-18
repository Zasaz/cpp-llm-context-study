#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"
#include "Vector2.h"

// Helper function to compare floating point numbers
bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Custom Shape subclass for testing polymorphic behavior and destructor tracking
class MockShape : public Shape {
public:
    double customArea;
    bool* destroyedFlag;

    MockShape(double areaVal, bool* flag = nullptr)
        : customArea(areaVal), destroyedFlag(flag) {}

    ~MockShape() override {
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
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    std::cout << "Test test_Canvas_initialState: PASS" << std::endl;
}

void test_Canvas_addValidShapes() {
    Canvas canvas;
    
    Rectangle* rect = new Rectangle(4.0, 5.0); // Area = 20.0
    Circle* circle = new Circle(3.0);          // Area = PI * 9.0 ≈ 28.27433388

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

    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    canvas.add(nullptr);
    assert(canvas.count() == 1);

    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;

    Rectangle* rect1 = new Rectangle(2.0, 3.0); // Area = 6.0
    Rectangle* rect2 = new Rectangle(4.0, 5.0); // Area = 20.0
    Circle* circ = new Circle(1.0);             // Area = PI ≈ 3.14159265

    canvas.add(rect1);
    assert(approxEqual(canvas.totalArea(), 6.0));

    canvas.add(rect2);
    assert(approxEqual(canvas.totalArea(), 26.0));

    canvas.add(circ);
    const double expectedTotal = 6.0 + 20.0 + (3.14159265358979323846 * 1.0 * 1.0);
    assert(approxEqual(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;

    // Empty canvas
    assert(approxEqual(canvas.largestArea(), 0.0));

    Rectangle* rect1 = new Rectangle(2.0, 5.0);  // Area = 10.0
    Rectangle* rect2 = new Rectangle(10.0, 5.0); // Area = 50.0
    Circle* circ = new Circle(2.0);              // Area = PI * 4 ≈ 12.56637

    canvas.add(rect1);
    assert(approxEqual(canvas.largestArea(), 10.0));

    canvas.add(rect2);
    assert(approxEqual(canvas.largestArea(), 50.0));

    canvas.add(circ);
    assert(approxEqual(canvas.largestArea(), 50.0)); // 50.0 is still largest

    // Add an even larger circle
    Circle* bigCirc = new Circle(5.0);           // Area = PI * 25 ≈ 78.5398
    canvas.add(bigCirc);
    assert(approxEqual(canvas.largestArea(), 3.14159265358979323846 * 25.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    bool destroyed1 = false;
    bool destroyed2 = false;

    MockShape* s1 = new MockShape(15.0, &destroyed1);
    MockShape* s2 = new MockShape(25.0, &destroyed2);

    canvas.add(s1);
    canvas.add(s2);
    assert(canvas.count() == 2);

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(destroyed1 == true);
    assert(destroyed2 == true);
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Calling clear again on empty canvas should be safe
    canvas.clear();
    assert(canvas.count() == 0);

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

void test_Canvas_destructor() {
    bool destroyed1 = false;
    bool destroyed2 = false;

    {
        Canvas canvas;
        canvas.add(new MockShape(10.0, &destroyed1));
        canvas.add(new MockShape(20.0, &destroyed2));
        assert(canvas.count() == 2);
    } // canvas goes out of scope here, invoking ~Canvas()

    assert(destroyed1 == true);
    assert(destroyed2 == true);

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    test_Canvas_initialState();
    test_Canvas_addValidShapes();
    test_Canvas_addNullPointer();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "\nAll Canvas unit tests passed successfully!" << std::endl;
    return 0;
}