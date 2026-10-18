#ifndef FILTER_H
#define FILTER_H

#include <string>

class Filter {
public:
    int applied;
    bool enabled;

    Filter();
    virtual ~Filter();

    virtual double apply(double reading);
    virtual std::string label() const;
    virtual bool rejects(double reading) const;
    int appliedCount() const;
    int resetCount();
    bool toggle();
};

class PassFilter : public Filter {
public:
    PassFilter();
    double apply(double reading);
    std::string label() const;
};

class ClampFilter : public Filter {
public:
    double low;
    double high;

    ClampFilter();
    ClampFilter(double lowBound, double highBound);
    double apply(double reading);
    std::string label() const;
    bool rejects(double reading) const;
    double width() const;
};

class SmoothFilter : public Filter {
public:
    double previous;
    double weight;
    bool primed;

    SmoothFilter();
    SmoothFilter(double smoothing);
    double apply(double reading);
    std::string label() const;
    double lastValue() const;
    bool isPrimed() const;
};

#endif
