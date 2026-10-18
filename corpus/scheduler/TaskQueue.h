#ifndef TASKQUEUE_H
#define TASKQUEUE_H

#include "Policy.h"
#include "Task.h"
#include <vector>

class TaskQueue {
public:
    std::vector<Task> tasks;
    Policy* policy;
    int capacity;
    int rejected;

    TaskQueue();
    TaskQueue(int limit);

    int enqueue(const Task& task);
    int usePolicy(Policy* replacement);
    int size() const;
    bool isEmpty() const;
    bool isFull() const;
    int bestIndex() const;
    Task best() const;
    Task dequeue();
    int totalRemaining() const;
    int pendingCount() const;
    int advanceAll(int units);
    int purgeComplete();
    int rejectedCount() const;
};

#endif
