#include "Filter.h"

Filter::Filter() {
    applied = 0;
    enabled = true;
}

Filter::~Filter() {
}

double Filter::apply(double reading) {
    applied = applied + 1;
    return reading;
}

std::string Filter::label() const {
    return "filter";
}

bool Filter::rejects(double reading) const {
    return !enabled && reading != 0.0;
}

int Filter::appliedCount() const {
    return applied;
}

int Filter::resetCount() {
    int previous = applied;
    applied = 0;
    return previous;
}

bool Filter::toggle() {
    enabled = !enabled;
    return enabled;
}

PassFilter::PassFilter() {
}

double PassFilter::apply(double reading) {
    applied = applied + 1;
    return reading;
}

std::string PassFilter::label() const {
    return "pass";
}

ClampFilter::ClampFilter() {
    low = 0.0;
    high = 1.0;
}

ClampFilter::ClampFilter(double lowBound, double highBound) {
    if (lowBound <= highBound) {
        low = lowBound;
        high = highBound;
    } else {
        low = highBound;
        high = lowBound;
    }
}

double ClampFilter::apply(double reading) {
    applied = applied + 1;
    if (reading < low) {
        return low;
    }
    if (reading > high) {
        return high;
    }
    return reading;
}

std::string ClampFilter::label() const {
    return "clamp";
}

bool ClampFilter::rejects(double reading) const {
    return reading < low || reading > high;
}

double ClampFilter::width() const {
    return high - low;
}

SmoothFilter::SmoothFilter() {
    previous = 0.0;
    weight = 0.5;
    primed = false;
}

SmoothFilter::SmoothFilter(double smoothing) {
    previous = 0.0;
    primed = false;
    weight = smoothing;
    if (weight < 0.0) {
        weight = 0.0;
    }
    if (weight > 1.0) {
        weight = 1.0;
    }
}

double SmoothFilter::apply(double reading) {
    applied = applied + 1;
    if (!primed) {
        previous = reading;
        primed = true;
        return reading;
    }
    previous = weight * reading + (1.0 - weight) * previous;
    return previous;
}

std::string SmoothFilter::label() const {
    return "smooth";
}

double SmoothFilter::lastValue() const {
    return previous;
}

bool SmoothFilter::isPrimed() const {
    return primed;
}
