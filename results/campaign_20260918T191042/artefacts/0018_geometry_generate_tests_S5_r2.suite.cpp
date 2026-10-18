#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
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

void test_Canvas_addNullPointer() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    std::cout << "Test test_Canvas_addNullPointer: PASS" << std::endl;
}

void test_Canvas_addShapesAndCount() {
    Canvas canvas;
    
    Rectangle* rect = new Rectangle(4.0, 5.0);
    Circle* circ = new Circle(3.0);

    canvas.add(rect);
    assert(canvas.count() == 1);
    assert(canvas.shapes[0] == rect);

    canvas.add(circ);
    assert(canvas.count() == 2);
    assert(canvas.shapes[1] == circ);

    // canvas destructor will free allocated shapes
    std::cout << "Test test_Canvas_addShapesAndCount: PASS" << std::endl;
}

void test_Canvas_totalArea() {
    Canvas canvas;
    
    Rectangle* r1 = new Rectangle(2.0, 3.0); // Area = 6.0
    Rectangle* r2 = new Rectangle(5.0, 4.0); // Area = 20.0
    Circle* c1 = new Circle(2.0);            // Area = PI * 4 ≈ 12.566370614
    Shape* baseShape = new Shape();          // Area = 0.0

    canvas.add(r1);
    canvas.add(r2);
    canvas.add(c1);
    canvas.add(baseShape);

    double expectedTotal = 6.0 + 20.0 + (3.14159265358979323846 * 4.0) + 0.0;
    assert(isClose(canvas.totalArea(), expectedTotal));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

void test_Canvas_largestArea() {
    Canvas canvas;

    // Initially largest is 0.0
    assert(isClose(canvas.largestArea(), 0.0));

    Rectangle* smallRect = new Rectangle(2.0, 2.0); // Area = 4.0
    Circle* mediumCircle = new Circle(2.0);          // Area ≈ 12.56637
    Rectangle* largeRect = new Rectangle(10.0, 2.0); // Area = 20.0
    Rectangle* tinyRect = new Rectangle(1.0, 1.0);  // Area = 1.0

    canvas.add(smallRect);
    assert(isClose(canvas.largestArea(), 4.0));

    canvas.add(mediumCircle);
    assert(isClose(canvas.largestArea(), 3.14159265358979323846 * 4.0));

    canvas.add(largeRect);
    assert(isClose(canvas.largestArea(), 20.0));

    canvas.add(tinyRect);
    assert(isClose(canvas.largestArea(), 20.0));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

void test_Canvas_clear() {
    Canvas canvas;
    
    canvas.add(new Rectangle(3.0, 3.0));
    canvas.add(new Circle(1.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(isClose(canvas.totalArea(), 0.0));
    assert(isClose(canvas.largestArea(), 0.0));

    // Ensure we can still add shapes after clearing
    canvas.add(new Rectangle(2.0, 5.0));
    assert(canvas.count() == 1);
    assert(isClose(canvas.totalArea(), 10.0));
    assert(isClose(canvas.largestArea(), 10.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// Custom mock shape to verify that Canvas destructor properly calls Shape's destructor
class DestructorTrackerShape : public Shape {
public:
    bool* flag;
    DestructorTrackerShape(bool* destructionFlag) : flag(destructionFlag) {
        *flag = false;
    }
    ~DestructorTrackerShape() override {
        *flag = true;
    }
};

void test_Canvas_destructorCleansMemory() {
    bool shapeDestroyed = false;
    {
        Canvas canvas;
        canvas.add(new DestructorTrackerShape(&shapeDestroyed));
        assert(shapeDestroyed == false);
    }
    // Destructor should have been invoked when canvas goes out of scope
    assert(shapeDestroyed == true);

    std::cout << "Test test_Canvas_destructorCleansMemory: PASS" << std::endl;
}

int main() {
    std::cout << "================ Running Canvas Unit Tests ================" << std::endl;

    test_Canvas_initialState();
    test_Canvas_addNullPointer();
    test_Canvas_addShapesAndCount();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructorCleansMemory();

    std::cout << "================ All Tests Passed Successfully! ================" << std::endl;
    return 0;
}