#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function to compare floating point values
bool areClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Mock/Concrete Shape derived class for testing Canvas functionality
class MockShape : public Shape {
private:
    double shapeArea;

public:
    MockShape(double a) : shapeArea(a) {}
    virtual ~MockShape() {}

    virtual double area() const {
        return shapeArea;
    }
};

// 1. Test constructor initializes empty canvas
void test_Canvas_initialization() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));
    assert(canvas.shapes.empty());

    std::cout << "Test test_Canvas_initialization: PASS" << std::endl;
}

// 2. Test adding valid shapes and null pointers
void test_Canvas_add() {
    Canvas canvas;
    
    // Add null pointer - should be ignored
    canvas.add(nullptr);
    assert(canvas.count() == 0);

    // Add valid shapes
    canvas.add(new MockShape(10.0));
    assert(canvas.count() == 1);

    canvas.add(new MockShape(25.0));
    assert(canvas.count() == 2);

    // Add another null pointer - count shouldn't change
    canvas.add(nullptr);
    assert(canvas.count() == 2);

    std::cout << "Test test_Canvas_add: PASS" << std::endl;
}

// 3. Test totalArea calculation
void test_Canvas_totalArea() {
    Canvas canvas;
    assert(areClose(canvas.totalArea(), 0.0));

    canvas.add(new MockShape(12.5));
    assert(areClose(canvas.totalArea(), 12.5));

    canvas.add(new MockShape(27.5));
    assert(areClose(canvas.totalArea(), 40.0));

    canvas.add(new MockShape(0.0));
    assert(areClose(canvas.totalArea(), 40.0));

    std::cout << "Test test_Canvas_totalArea: PASS" << std::endl;
}

// 4. Test largestArea calculation
void test_Canvas_largestArea() {
    Canvas canvas;
    // Empty canvas returns 0.0
    assert(areClose(canvas.largestArea(), 0.0));

    canvas.add(new MockShape(15.0));
    assert(areClose(canvas.largestArea(), 15.0));

    canvas.add(new MockShape(50.0)); // Larger
    assert(areClose(canvas.largestArea(), 50.0));

    canvas.add(new MockShape(30.0)); // Smaller
    assert(areClose(canvas.largestArea(), 50.0));

    canvas.add(new MockShape(75.5)); // Larger
    assert(areClose(canvas.largestArea(), 75.5));

    std::cout << "Test test_Canvas_largestArea: PASS" << std::endl;
}

// 5. Test clear method resets state
void test_Canvas_clear() {
    Canvas canvas;
    canvas.add(new MockShape(10.0));
    canvas.add(new MockShape(20.0));
    canvas.add(new MockShape(30.0));

    assert(canvas.count() == 3);
    assert(areClose(canvas.totalArea(), 60.0));

    canvas.clear();

    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(areClose(canvas.totalArea(), 0.0));
    assert(areClose(canvas.largestArea(), 0.0));

    // Ensure adding shapes after clear works properly
    canvas.add(new MockShape(5.0));
    assert(canvas.count() == 1);
    assert(areClose(canvas.totalArea(), 5.0));

    std::cout << "Test test_Canvas_clear: PASS" << std::endl;
}

// 6. Test destructor cleanup via dynamic allocation/scoping
void test_Canvas_destructor() {
    Canvas* canvas = new Canvas();
    canvas->add(new MockShape(10.0));
    canvas->add(new MockShape(20.0));
    
    // Destructor is executed, properly freeing shapes and clearing internal vector
    delete canvas;

    std::cout << "Test test_Canvas_destructor: PASS" << std::endl;
}

int main() {
    std::cout << "Starting Canvas test suite..." << std::endl;

    test_Canvas_initialization();
    test_Canvas_add();
    test_Canvas_totalArea();
    test_Canvas_largestArea();
    test_Canvas_clear();
    test_Canvas_destructor();

    std::cout << "All Canvas tests passed successfully!" << std::endl;
    return 0;
}