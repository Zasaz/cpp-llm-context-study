#ifndef RESOURCELEDGER_H
#define RESOURCELEDGER_H

#include "RunStatistics.h"
#include <map>
#include <string>

class ResourceLedger {
public:
    std::map<std::string, int> balances;
    RunStatistics movements;
    int limit;

    ResourceLedger();
    ResourceLedger(int perResourceLimit);

    int deposit(const std::string& resource, int amount);
    int withdraw(const std::string& resource, int amount);
    int balanceOf(const std::string& resource) const;
    bool holds(const std::string& resource) const;
    int resourceCount() const;
    int totalHeld() const;
    double averageMovement() const;
    int clearAll();
};

#endif
