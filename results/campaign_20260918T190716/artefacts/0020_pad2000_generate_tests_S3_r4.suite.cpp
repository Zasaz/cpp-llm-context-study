#include <iostream>
#include <cassert>
#include <cmath>
#include "Canvas.h"
#include "Shape.h"

// Helper function for floating point comparison
bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test initial state of a newly created Canvas
void test_Canvas_constructor_initialState() {
    Canvas canvas;
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_constructor_initialState: PASS" << std::endl;
}

// Test adding null pointers does not increase count or modify shapes
void test_Canvas_add_nullPointer() {
    Canvas canvas;
    canvas.add(nullptr);
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));
    std::cout << "Test test_Canvas_add_nullPointer: PASS" << std::endl;
}

// Test adding valid shapes and verifying count
void test_Canvas_add_validShapes() {
    Canvas canvas;
    canvas.add(new Rectangle(2.0, 3.0));
    assert(canvas.count() == 1);

    canvas.add(new Circle(1.0));
    assert(canvas.count() == 2);

    canvas.add(new Shape());
    assert(canvas.count() == 3);

    std::cout << "Test test_Canvas_add_validShapes: PASS" << std::endl;
}

// Test totalArea calculation with multiple shapes
void test_Canvas_totalArea_multipleShapes() {
    Canvas canvas;
    // Rectangle area = 2.0 * 5.0 = 10.0
    canvas.add(new Rectangle(2.0, 5.0));
    // Circle area = PI * 2.0 * 2.0 ≈ 12.566370614
    canvas.add(new Circle(2.0));
    // Base Shape area = 0.0
    canvas.add(new Shape());

    double expectedTotal = 10.0 + (3.14159265358979323846 * 4.0) + 0.0;
    assert(approxEqual(canvas.totalArea(), expectedTotal));
    std::cout << "Test test_Canvas_totalArea_multipleShapes: PASS" << std::endl;
}

// Test largestArea computation among different shapes
void test_Canvas_largestArea_orderIndependence() {
    {
        Canvas canvas;
        canvas.add(new Rectangle(2.0, 2.0)); // area = 4.0
        canvas.add(new Rectangle(5.0, 5.0)); // area = 25.0
        canvas.add(new Circle(1.0));          // area ≈ 3.14159
        assert(approxEqual(canvas.largestArea(), 25.0));
    }
    {
        Canvas canvas;
        canvas.add(new Rectangle(10.0, 10.0)); // area = 100.0 (first is largest)
        canvas.add(new Rectangle(2.0, 2.0));   // area = 4.0
        canvas.add(new Circle(3.0));           // area ≈ 28.2743
        assert(approxEqual(canvas.largestArea(), 100.0));
    }
    std::cout << "Test test_Canvas_largestArea_orderIndependence: PASS" << std::endl;
}

// Test clearing a canvas and reusing it afterwards
void test_Canvas_clear_andReuse() {
    Canvas canvas;
    canvas.add(new Rectangle(3.0, 4.0));
    canvas.add(new Circle(2.0));
    assert(canvas.count() == 2);

    canvas.clear();
    assert(canvas.count() == 0);
    assert(canvas.shapes.empty());
    assert(approxEqual(canvas.totalArea(), 0.0));
    assert(approxEqual(canvas.largestArea(), 0.0));

    // Add new shapes after clear to verify it is still functional
    canvas.add(new Rectangle(1.0, 1.0));
    assert(canvas.count() == 1);
    assert(approxEqual(canvas.totalArea(), 1.0));
    assert(approxEqual(canvas.largestArea(), 1.0));

    std::cout << "Test test_Canvas_clear_andReuse: PASS" << std::endl;
}

// Test destructor memory cleanup by using custom shape subclass
class DestructionTrackerShape : public Shape {
public:
    static int deleteCount;
    DestructionTrackerShape() {}
    ~DestructionTrackerShape() override {
        deleteCount++;
    }
    double area() const override {
        return 5.0;
    }
};
int DestructionTrackerShape::deleteCount = 0;

void test_Canvas_destructor_freesMemory() {
    DestructionTrackerShape::deleteCount = 0;
    {
        Canvas canvas;
        canvas.add(new DestructionTrackerShape());
        canvas.add(new DestructionTrackerShape());
        assert(canvas.count() == 2);
    }
    // Destructor should have been invoked for both shapes upon scope exit
    assert(DestructionTrackerShape::deleteCount == 2);
    std::cout << "Test test_Canvas_destructor_freesMemory: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running Canvas Unit Tests ===" << std::endl;

    test_Canvas_constructor_initialState();
    test_Canvas_add_nullPointer();
    test_Canvas_add_validShapes();
    test_Canvas_totalArea_multipleShapes();
    test_Canvas_largestArea_orderIndependence();
    test_Canvas_clear_andReuse();
    test_Canvas_destructor_freesMemory();

    std::cout << "=== All Canvas tests PASSED successfully! ===" << std::endl;
    return 0;
}