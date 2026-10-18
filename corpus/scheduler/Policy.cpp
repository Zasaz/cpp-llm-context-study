#include "Policy.h"

Policy::Policy() {
    bias = 0;
}

Policy::~Policy() {
}

int Policy::rank(const Task& task) const {
    return task.priority + bias;
}

std::string Policy::name() const {
    return "policy";
}

bool Policy::accepts(const Task& task) const {
    return !task.isComplete();
}

int Policy::adjust(int amount) {
    bias = bias + amount;
    return bias;
}

FifoPolicy::FifoPolicy() {
    arrivalWeight = 1;
}

FifoPolicy::FifoPolicy(int weight) {
    arrivalWeight = weight;
}

int FifoPolicy::rank(const Task& task) const {
    return bias - task.id * arrivalWeight;
}

std::string FifoPolicy::name() const {
    return "fifo";
}

PriorityPolicy::PriorityPolicy() {
    floorPriority = 0;
}

PriorityPolicy::PriorityPolicy(int minimum) {
    floorPriority = minimum;
}

int PriorityPolicy::rank(const Task& task) const {
    int score = task.priority;
    if (score < floorPriority) {
        score = floorPriority;
    }
    return score * 100 - task.duration + bias;
}

std::string PriorityPolicy::name() const {
    return "priority";
}

bool PriorityPolicy::accepts(const Task& task) const {
    return task.priority >= floorPriority;
}

DeadlinePolicy::DeadlinePolicy() {
    horizon = 100;
}

DeadlinePolicy::DeadlinePolicy(int limit) {
    horizon = limit;
}

int DeadlinePolicy::rank(const Task& task) const {
    int urgency = horizon - task.deadline;
    if (urgency < 0) {
        urgency = 0;
    }
    return urgency * 5 + bias;
}

std::string DeadlinePolicy::name() const {
    return "deadline";
}

bool DeadlinePolicy::accepts(const Task& task) const {
    return task.deadline <= horizon;
}
