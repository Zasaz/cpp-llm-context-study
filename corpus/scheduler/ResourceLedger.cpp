#include "ResourceLedger.h"

ResourceLedger::ResourceLedger() {
    limit = 100;
}

ResourceLedger::ResourceLedger(int perResourceLimit) {
    limit = perResourceLimit > 0 ? perResourceLimit : 1;
}

int ResourceLedger::deposit(const std::string& resource, int amount) {
    if (amount <= 0) {
        return balanceOf(resource);
    }
    int current = balanceOf(resource);
    int updated = current + amount;
    if (updated > limit) {
        updated = limit;
    }
    balances[resource] = updated;
    movements.record(updated - current);
    return updated;
}

int ResourceLedger::withdraw(const std::string& resource, int amount) {
    int current = balanceOf(resource);
    if (amount <= 0 || amount > current) {
        return -1;
    }
    balances[resource] = current - amount;
    movements.record(amount);
    return current - amount;
}

int ResourceLedger::balanceOf(const std::string& resource) const {
    std::map<std::string, int>::const_iterator found = balances.find(resource);
    if (found == balances.end()) {
        return 0;
    }
    return found->second;
}

bool ResourceLedger::holds(const std::string& resource) const {
    return balances.find(resource) != balances.end();
}

int ResourceLedger::resourceCount() const {
    return static_cast<int>(balances.size());
}

int ResourceLedger::totalHeld() const {
    int sum = 0;
    std::map<std::string, int>::const_iterator it = balances.begin();
    while (it != balances.end()) {
        sum = sum + it->second;
        ++it;
    }
    return sum;
}

double ResourceLedger::averageMovement() const {
    return movements.mean();
}

int ResourceLedger::clearAll() {
    int previous = resourceCount();
    balances.clear();
    movements.discard();
    return previous;
}
