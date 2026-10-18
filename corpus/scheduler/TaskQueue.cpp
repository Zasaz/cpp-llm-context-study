#include "TaskQueue.h"

TaskQueue::TaskQueue() {
    policy = 0;
    capacity = 8;
    rejected = 0;
}

TaskQueue::TaskQueue(int limit) {
    policy = 0;
    capacity = limit > 0 ? limit : 1;
    rejected = 0;
}

int TaskQueue::size() const {
    return static_cast<int>(tasks.size());
}

bool TaskQueue::isEmpty() const {
    return tasks.empty();
}

bool TaskQueue::isFull() const {
    return size() >= capacity;
}

int TaskQueue::enqueue(const Task& task) {
    if (isFull()) {
        rejected = rejected + 1;
        return -1;
    }
    if (policy != 0 && !policy->accepts(task)) {
        rejected = rejected + 1;
        return -1;
    }
    tasks.push_back(task);
    return size();
}

int TaskQueue::usePolicy(Policy* replacement) {
    policy = replacement;
    return size();
}

int TaskQueue::bestIndex() const {
    if (tasks.empty()) {
        return -1;
    }
    int chosen = 0;
    int bestScore = 0;
    for (unsigned int i = 0; i < tasks.size(); ++i) {
        int score = 0;
        if (policy != 0) {
            score = policy->rank(tasks[i]);
        } else {
            score = tasks[i].weight();
        }
        if (i == 0 || score > bestScore) {
            chosen = static_cast<int>(i);
            bestScore = score;
        }
    }
    return chosen;
}

Task TaskQueue::best() const {
    int index = bestIndex();
    if (index < 0) {
        return Task();
    }
    return tasks[static_cast<unsigned int>(index)];
}

Task TaskQueue::dequeue() {
    int index = bestIndex();
    if (index < 0) {
        return Task();
    }
    Task chosen = tasks[static_cast<unsigned int>(index)];
    tasks.erase(tasks.begin() + index);
    return chosen;
}

int TaskQueue::totalRemaining() const {
    int total = 0;
    for (unsigned int i = 0; i < tasks.size(); ++i) {
        total = total + tasks[i].remaining();
    }
    return total;
}

int TaskQueue::pendingCount() const {
    int total = 0;
    for (unsigned int i = 0; i < tasks.size(); ++i) {
        if (tasks[i].isPending()) {
            total = total + 1;
        }
    }
    return total;
}

int TaskQueue::advanceAll(int units) {
    int completed = 0;
    for (unsigned int i = 0; i < tasks.size(); ++i) {
        tasks[i].advance(units);
        if (tasks[i].isComplete()) {
            completed = completed + 1;
        }
    }
    return completed;
}

int TaskQueue::purgeComplete() {
    std::vector<Task> kept;
    int removed = 0;
    for (unsigned int i = 0; i < tasks.size(); ++i) {
        if (tasks[i].isComplete()) {
            removed = removed + 1;
        } else {
            kept.push_back(tasks[i]);
        }
    }
    tasks = kept;
    return removed;
}

int TaskQueue::rejectedCount() const {
    return rejected;
}
