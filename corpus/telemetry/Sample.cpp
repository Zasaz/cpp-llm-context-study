#include "Sample.h"

Sample::Sample() {
    sequence = 0;
    raw = 0.0;
    quality = 0;
    flagged = true;
}

Sample::Sample(int position, double reading, int qualityScore) {
    sequence = position;
    raw = reading;
    quality = qualityScore;
    if (quality < 0) {
        quality = 0;
    }
    if (quality > 100) {
        quality = 100;
    }
    flagged = false;
}

int Sample::position() const {
    return sequence;
}

double Sample::value() const {
    return raw;
}

int Sample::score() const {
    return quality;
}

bool Sample::isValid() const {
    return !flagged && quality > 0;
}

bool Sample::isDegraded() const {
    return quality < 50;
}

double Sample::weighted() const {
    if (!isValid()) {
        return 0.0;
    }
    return raw * static_cast<double>(quality) / 100.0;
}

double Sample::deviationFrom(double reference) const {
    double delta = raw - reference;
    return delta < 0.0 ? -delta : delta;
}

double Sample::bounded(double low, double high) const {
    if (low > high) {
        return raw;
    }
    if (raw < low) {
        return low;
    }
    if (raw > high) {
        return high;
    }
    return raw;
}

int Sample::flag() {
    int previous = quality;
    flagged = true;
    return previous;
}

int Sample::promote(int amount) {
    quality = quality + amount;
    if (quality < 0) {
        quality = 0;
    }
    if (quality > 100) {
        quality = 100;
    }
    return quality;
}
