#ifndef ALERTRULE_H
#define ALERTRULE_H

#include <string>

class AlertRule {
public:
    std::string subject;
    int fired;
    bool muted;

    AlertRule();
    AlertRule(const std::string& target);
    virtual ~AlertRule();

    virtual bool triggers(double reading) const;
    virtual std::string message() const;
    virtual int severity() const;
    int evaluate(double reading);
    int firedCount() const;
    bool mute();
    bool isMuted() const;
    std::string target() const;
};

class HighWaterRule : public AlertRule {
public:
    double ceiling;

    HighWaterRule();
    HighWaterRule(const std::string& target, double limit);
    bool triggers(double reading) const;
    std::string message() const;
    int severity() const;
    double headroom(double reading) const;
};

class LowWaterRule : public AlertRule {
public:
    double floorValue;

    LowWaterRule();
    LowWaterRule(const std::string& target, double limit);
    bool triggers(double reading) const;
    std::string message() const;
    int severity() const;
};

class BandRule : public AlertRule {
public:
    double low;
    double high;

    BandRule();
    BandRule(const std::string& target, double lowBound, double highBound);
    bool triggers(double reading) const;
    std::string message() const;
    int severity() const;
    double width() const;
};

#endif
