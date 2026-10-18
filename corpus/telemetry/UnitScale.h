#ifndef UNITSCALE_H
#define UNITSCALE_H

#include <string>

class UnitScale {
public:
    double factor;
    double offset;
    std::string unit;

    UnitScale();
    UnitScale(double scaleFactor, double scaleOffset, const std::string& unitName);

    double toBase(double reading) const;
    double fromBase(double reading) const;
    double spanOf(double low, double high) const;
    std::string unitName() const;
    bool isIdentity() const;
    double quantised(double reading, int digits) const;
    UnitScale inverted() const;
};

#endif
