#include "Canvas.h"

Canvas::Canvas() {
}

Canvas::~Canvas() {
    clear();
}

void Canvas::add(Shape* shape) {
    if (shape != 0) {
        shapes.push_back(shape);
    }
}

int Canvas::count() const {
    return static_cast<int>(shapes.size());
}

double Canvas::totalArea() const {
    double sum = 0.0;
    for (unsigned int i = 0; i < shapes.size(); ++i) {
        sum = sum + shapes[i]->area();
    }
    return sum;
}

double Canvas::largestArea() const {
    double best = 0.0;
    for (unsigned int i = 0; i < shapes.size(); ++i) {
        double current = shapes[i]->area();
        if (current > best) {
            best = current;
        }
    }
    return best;
}

void Canvas::clear() {
    for (unsigned int i = 0; i < shapes.size(); ++i) {
        delete shapes[i];
    }
    shapes.clear();
}
