#ifndef SHAPE_H
#define SHAPE_H

#include "Vector2.h"

class Shape {
public:
    Vector2 origin;

    Shape();
    virtual ~Shape();
    virtual double area() const;
    virtual double perimeter() const;
    Vector2 centre() const;
};

class Rectangle : public Shape {
public:
    double width;
    double height;

    Rectangle(double w, double h);
    double area() const;
    double perimeter() const;
};

class Circle : public Shape {
public:
    double radius;

    Circle(double r);
    double area() const;
    double perimeter() const;
};

#endif
