#ifndef TASK_H
#define TASK_H

#include <string>

class Task {
public:
    int id;
    std::string name;
    int priority;
    int duration;
    int deadline;
    int elapsed;
    bool started;
    bool finished;

    Task();
    Task(int taskId, const std::string& taskName, int taskPriority, int taskDuration);

    int identifier() const;
    std::string label() const;
    int weight() const;
    int remaining() const;
    bool isPending() const;
    bool isRunning() const;
    bool isComplete() const;
    bool isOverdue() const;
    int advance(int units);
    int restart();
    int setDeadline(int limit);
};

#endif
