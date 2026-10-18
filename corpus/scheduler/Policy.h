#ifndef POLICY_H
#define POLICY_H

#include "Task.h"
#include <string>

class Policy {
public:
    int bias;

    Policy();
    virtual ~Policy();

    virtual int rank(const Task& task) const;
    virtual std::string name() const;
    virtual bool accepts(const Task& task) const;
    int adjust(int amount);
};

class FifoPolicy : public Policy {
public:
    int arrivalWeight;

    FifoPolicy();
    FifoPolicy(int weight);
    int rank(const Task& task) const;
    std::string name() const;
};

class PriorityPolicy : public Policy {
public:
    int floorPriority;

    PriorityPolicy();
    PriorityPolicy(int minimum);
    int rank(const Task& task) const;
    std::string name() const;
    bool accepts(const Task& task) const;
};

class DeadlinePolicy : public Policy {
public:
    int horizon;

    DeadlinePolicy();
    DeadlinePolicy(int limit);
    int rank(const Task& task) const;
    std::string name() const;
    bool accepts(const Task& task) const;
};

#endif
