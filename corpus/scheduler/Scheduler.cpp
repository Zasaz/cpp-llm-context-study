#include "Scheduler.h"

Scheduler::Scheduler() {
    tick = 0;
    completed = 0;
}

Scheduler::Scheduler(int capacity) {
    queue = TaskQueue(capacity);
    tick = 0;
    completed = 0;
}

int Scheduler::submit(const Task& task) {
    return queue.enqueue(task);
}

int Scheduler::step(int units) {
    if (units <= 0) {
        return completed;
    }
    tick = tick + units;
    int finished = queue.advanceAll(units);
    completed = completed + finished;
    stats.record(finished);
    queue.purgeComplete();
    return completed;
}

int Scheduler::runUntilIdle(int maxSteps) {
    int taken = 0;
    while (taken < maxSteps && !idle()) {
        step(1);
        taken = taken + 1;
    }
    return taken;
}

int Scheduler::currentTick() const {
    return tick;
}

int Scheduler::completedCount() const {
    return completed;
}

int Scheduler::backlog() const {
    return queue.totalRemaining();
}

bool Scheduler::idle() const {
    return queue.isEmpty();
}

double Scheduler::averageStep() const {
    return stats.mean();
}

int Scheduler::reserve(const std::string& resource, int amount) {
    return ledger.deposit(resource, amount);
}

int Scheduler::release(const std::string& resource, int amount) {
    return ledger.withdraw(resource, amount);
}
