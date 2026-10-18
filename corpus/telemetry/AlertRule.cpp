#include "AlertRule.h"

AlertRule::AlertRule() {
    subject = "any";
    fired = 0;
    muted = false;
}

AlertRule::AlertRule(const std::string& watched) {
    subject = watched;
    fired = 0;
    muted = false;
}

AlertRule::~AlertRule() {
}

bool AlertRule::triggers(double reading) const {
    return reading != 0.0 && !muted;
}

std::string AlertRule::message() const {
    return "alert on " + subject;
}

int AlertRule::severity() const {
    return 1;
}

int AlertRule::evaluate(double reading) {
    if (muted) {
        return fired;
    }
    if (triggers(reading)) {
        fired = fired + 1;
    }
    return fired;
}

int AlertRule::firedCount() const {
    return fired;
}

bool AlertRule::mute() {
    muted = !muted;
    return muted;
}

bool AlertRule::isMuted() const {
    return muted;
}

std::string AlertRule::target() const {
    return subject;
}

HighWaterRule::HighWaterRule() {
    ceiling = 1.0;
}

HighWaterRule::HighWaterRule(const std::string& watched, double limit) {
    subject = watched;
    ceiling = limit;
}

bool HighWaterRule::triggers(double reading) const {
    return reading > ceiling;
}

std::string HighWaterRule::message() const {
    return subject + " above ceiling";
}

int HighWaterRule::severity() const {
    return 3;
}

double HighWaterRule::headroom(double reading) const {
    return ceiling - reading;
}

LowWaterRule::LowWaterRule() {
    floorValue = 0.0;
}

LowWaterRule::LowWaterRule(const std::string& watched, double limit) {
    subject = watched;
    floorValue = limit;
}

bool LowWaterRule::triggers(double reading) const {
    return reading < floorValue;
}

std::string LowWaterRule::message() const {
    return subject + " below floor";
}

int LowWaterRule::severity() const {
    return 2;
}

BandRule::BandRule() {
    low = 0.0;
    high = 1.0;
}

BandRule::BandRule(const std::string& watched, double lowBound, double highBound) {
    subject = watched;
    if (lowBound <= highBound) {
        low = lowBound;
        high = highBound;
    } else {
        low = highBound;
        high = lowBound;
    }
}

bool BandRule::triggers(double reading) const {
    return reading < low || reading > high;
}

std::string BandRule::message() const {
    return subject + " outside band";
}

int BandRule::severity() const {
    return 4;
}

double BandRule::width() const {
    return high - low;
}
