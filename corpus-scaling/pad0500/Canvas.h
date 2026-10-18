#ifndef CANVAS_H
#define CANVAS_H

#include "Shape.h"
#include <vector>

class Canvas {
public:
    std::vector<Shape*> shapes;

    Canvas();
    ~Canvas();

    void add(Shape* shape);
    int count() const;
    double totalArea() const;
    double largestArea() const;
    void clear();
};

#endif
