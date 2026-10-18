#ifndef DASHBOARD_H
#define DASHBOARD_H

#include "AlertRule.h"
#include "Channel.h"
#include <string>
#include <vector>

class Dashboard {
public:
    std::vector<Channel> panels;
    std::vector<AlertRule*> rules;
    std::string title;
    int checks;

    Dashboard();
    Dashboard(const std::string& heading);

    int addPanel(const Channel& panel);
    int addRule(AlertRule* rule);
    int panelCount() const;
    int ruleCount() const;
    int checkCount() const;
    int sweep();
    int alertsFor(double reading) const;
    int worstSeverity() const;
    double panelMean(int index) const;
    double overallMean() const;
    std::string heading() const;
    std::string status() const;
    int reset();
};

#endif
