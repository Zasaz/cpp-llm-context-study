#include "Task.h"

Task::Task() {
    id = 0;
    name = "unnamed";
    priority = 0;
    duration = 0;
    deadline = 0;
    elapsed = 0;
    started = false;
    finished = false;
}

Task::Task(int taskId, const std::string& taskName, int taskPriority, int taskDuration) {
    id = taskId;
    name = taskName;
    priority = taskPriority;
    duration = taskDuration > 0 ? taskDuration : 0;
    deadline = duration;
    elapsed = 0;
    started = false;
    finished = false;
}

int Task::identifier() const {
    return id;
}

std::string Task::label() const {
    return name;
}

int Task::weight() const {
    int base = priority * 10;
    if (isOverdue()) {
        base = base + 25;
    }
    return base;
}

int Task::remaining() const {
    int left = duration - elapsed;
    return left > 0 ? left : 0;
}

bool Task::isPending() const {
    return !started && !finished;
}

bool Task::isRunning() const {
    return started && !finished;
}

bool Task::isComplete() const {
    return finished;
}

bool Task::isOverdue() const {
    return elapsed > deadline;
}

int Task::advance(int units) {
    if (units <= 0 || finished) {
        return remaining();
    }
    started = true;
    elapsed = elapsed + units;
    if (elapsed >= duration) {
        elapsed = duration;
        finished = true;
    }
    return remaining();
}

int Task::restart() {
    int previous = elapsed;
    elapsed = 0;
    started = false;
    finished = false;
    return previous;
}

int Task::setDeadline(int limit) {
    int previous = deadline;
    if (limit >= 0) {
        deadline = limit;
    }
    return previous;
}
