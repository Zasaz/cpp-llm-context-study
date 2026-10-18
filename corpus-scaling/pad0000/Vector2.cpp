#include "Vector2.h"
#include <cmath>

Vector2::Vector2() {
    x = 0.0;
    y = 0.0;
}

Vector2::Vector2(double px, double py) {
    x = px;
    y = py;
}

double Vector2::lengthSquared() const {
    return x * x + y * y;
}

double Vector2::length() const {
    return std::sqrt(lengthSquared());
}

Vector2 Vector2::scaled(double factor) const {
    return Vector2(x * factor, y * factor);
}

Vector2 Vector2::added(const Vector2& other) const {
    return Vector2(x + other.x, y + other.y);
}
