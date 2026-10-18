#include "Dashboard.h"

Dashboard::Dashboard() {
    title = "dashboard";
    checks = 0;
}

Dashboard::Dashboard(const std::string& heading) {
    title = heading;
    checks = 0;
}

int Dashboard::addPanel(const Channel& panel) {
    panels.push_back(panel);
    return panelCount();
}

int Dashboard::addRule(AlertRule* rule) {
    if (rule == 0) {
        return ruleCount();
    }
    rules.push_back(rule);
    return ruleCount();
}

int Dashboard::panelCount() const {
    return static_cast<int>(panels.size());
}

int Dashboard::ruleCount() const {
    return static_cast<int>(rules.size());
}

int Dashboard::checkCount() const {
    return checks;
}

int Dashboard::sweep() {
    int raised = 0;
    for (unsigned int i = 0; i < panels.size(); ++i) {
        double reading = panels[i].mean();
        for (unsigned int r = 0; r < rules.size(); ++r) {
            int before = rules[r]->firedCount();
            if (rules[r]->evaluate(reading) > before) {
                raised = raised + 1;
            }
        }
    }
    checks = checks + 1;
    return raised;
}

int Dashboard::alertsFor(double reading) const {
    int total = 0;
    for (unsigned int r = 0; r < rules.size(); ++r) {
        if (rules[r]->triggers(reading)) {
            total = total + 1;
        }
    }
    return total;
}

int Dashboard::worstSeverity() const {
    int worst = 0;
    for (unsigned int r = 0; r < rules.size(); ++r) {
        int current = rules[r]->severity();
        if (current > worst) {
            worst = current;
        }
    }
    return worst;
}

double Dashboard::panelMean(int index) const {
    if (index < 0 || index >= panelCount()) {
        return 0.0;
    }
    return panels[static_cast<unsigned int>(index)].mean();
}

double Dashboard::overallMean() const {
    if (panels.empty()) {
        return 0.0;
    }
    double total = 0.0;
    for (unsigned int i = 0; i < panels.size(); ++i) {
        total = total + panels[i].mean();
    }
    return total / static_cast<double>(panels.size());
}

std::string Dashboard::heading() const {
    return title;
}

std::string Dashboard::status() const {
    if (rules.empty()) {
        return "idle";
    }
    if (worstSeverity() >= 3) {
        return "critical";
    }
    return "nominal";
}

int Dashboard::reset() {
    int previous = checks;
    checks = 0;
    return previous;
}
