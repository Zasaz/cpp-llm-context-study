#include "Shape.h"

static const double PI = 3.14159265358979323846;

Shape::Shape() {
    origin = Vector2(0.0, 0.0);
}

Shape::~Shape() {
}

double Shape::area() const {
    return 0.0;
}

double Shape::perimeter() const {
    return 0.0;
}

Vector2 Shape::centre() const {
    return origin;
}

Rectangle::Rectangle(double w, double h) {
    width = w;
    height = h;
}

double Rectangle::area() const {
    return width * height;
}

double Rectangle::perimeter() const {
    return 2.0 * (width + height);
}

Circle::Circle(double r) {
    radius = r;
}

double Circle::area() const {
    return PI * radius * radius;
}

double Circle::perimeter() const {
    return 2.0 * PI * radius;
}
