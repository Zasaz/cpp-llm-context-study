#include "UnitScale.h"
#include <cmath>

UnitScale::UnitScale() {
    factor = 1.0;
    offset = 0.0;
    unit = "raw";
}

UnitScale::UnitScale(double scaleFactor, double scaleOffset, const std::string& unitName) {
    factor = scaleFactor != 0.0 ? scaleFactor : 1.0;
    offset = scaleOffset;
    unit = unitName;
}

double UnitScale::toBase(double reading) const {
    return reading * factor + offset;
}

double UnitScale::fromBase(double reading) const {
    return (reading - offset) / factor;
}

double UnitScale::spanOf(double low, double high) const {
    double a = toBase(low);
    double b = toBase(high);
    double delta = b - a;
    return delta < 0.0 ? -delta : delta;
}

std::string UnitScale::unitName() const {
    return unit;
}

bool UnitScale::isIdentity() const {
    return factor == 1.0 && offset == 0.0;
}

double UnitScale::quantised(double reading, int digits) const {
    if (digits < 0) {
        return reading;
    }
    double step = std::pow(10.0, static_cast<double>(digits));
    double shifted = reading * step;
    if (shifted < 0.0) {
        return -std::floor(-shifted + 0.5) / step;
    }
    return std::floor(shifted + 0.5) / step;
}

UnitScale UnitScale::inverted() const {
    return UnitScale(1.0 / factor, -offset / factor, unit);
}
