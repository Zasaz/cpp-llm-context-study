#ifndef VECTOR2_H
#define VECTOR2_H

class Vector2 {
public:
    double x;
    double y;

    Vector2();
    Vector2(double px, double py);

    double length() const;
    double lengthSquared() const;
    Vector2 scaled(double factor) const;
    Vector2 added(const Vector2& other) const;
};

#endif
