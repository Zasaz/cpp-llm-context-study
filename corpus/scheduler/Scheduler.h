#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "ResourceLedger.h"
#include "RunStatistics.h"
#include "TaskQueue.h"
#include <string>

class Scheduler {
public:
    TaskQueue queue;
    RunStatistics stats;
    ResourceLedger ledger;
    int tick;
    int completed;

    Scheduler();
    Scheduler(int capacity);

    int submit(const Task& task);
    int step(int units);
    int runUntilIdle(int maxSteps);
    int currentTick() const;
    int completedCount() const;
    int backlog() const;
    bool idle() const;
    double averageStep() const;
    int reserve(const std::string& resource, int amount);
    int release(const std::string& resource, int amount);
};

#endif
