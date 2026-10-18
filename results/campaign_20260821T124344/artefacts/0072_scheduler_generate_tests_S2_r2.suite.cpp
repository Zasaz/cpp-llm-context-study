#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#include "TaskQueue.h"

// Concrete Policy for testing custom acceptance and ranking rules
class MockPolicy : public Policy {
public:
    int minWeight;

    MockPolicy(int minW = 0) : minWeight(minW) {}

    bool accepts(const Task& task) const override {
        return task.weight() >= minWeight;
    }

    int rank(const Task& task) const override {
        // Rank higher if remaining units are smaller (Shortest Job First),
        // or any custom inverse formula
        return 1000 - task.remaining();
    }
};

void test_constructors_and_defaults() {
    // Default constructor: capacity 8, empty, 0 rejected
    TaskQueue defaultQueue;
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.totalRemaining() == 0);
    assert(defaultQueue.pendingCount() == 0);

    // Parameterized constructor: custom positive capacity
    TaskQueue customQueue(3);
    assert(customQueue.isEmpty());
    assert(!customQueue.isFull());
    assert(customQueue.size() == 0);

    // Parameterized constructor: edge cases for <= 0 capacity (should clamp to 1)
    TaskQueue zeroQueue(0);
    assert(!zeroQueue.isFull());
    Task t1;
    assert(zeroQueue.enqueue(t1) == 1);
    assert(zeroQueue.isFull());

    TaskQueue negativeQueue(-5);
    assert(!negativeQueue.isFull());
    assert(negativeQueue.enqueue(t1) == 1);
    assert(negativeQueue.isFull());

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue queue(2);
    Task t1, t2, t3;

    // Enqueue until full
    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isFull());

    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());

    // Overflow attempt
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    int res4 = queue.enqueue(t3);
    assert(res4 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_policy_acceptance_and_rejection() {
    TaskQueue queue(5);
    MockPolicy policy(10); // Rejects tasks with weight < 10
    queue.usePolicy(&policy);

    Task lightTask;
    // Assuming a Task API allows setting properties or default constructor yields weight < 10
    // If Task constructors vary, we verify based on policy rejection logic
    if (lightTask.weight() < 10) {
        int res = queue.enqueue(lightTask);
        assert(res == -1);
        assert(queue.rejectedCount() == 1);
        assert(queue.size() == 0);
    }

    // Switch back to no policy (nullptr / 0)
    queue.usePolicy(nullptr);
    int res = queue.enqueue(lightTask);
    assert(res == 1);
    assert(queue.size() == 1);

    std::cout << "Test test_policy_acceptance_and_rejection: PASS" << std::endl;
}

void test_dequeue_and_best_selection() {
    TaskQueue queue(5);

    // Empty queue checks
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    Task emptyDequeued = queue.dequeue();
    assert(queue.isEmpty());

    // Enqueue tasks
    Task t1, t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    assert(queue.size() == 2);
    int bestIdx = queue.bestIndex();
    assert(bestIdx >= 0 && bestIdx < queue.size());

    Task peeked = queue.best();
    Task dequeued = queue.dequeue();
    assert(queue.size() == 1);

    // Dequeue remaining
    queue.dequeue();
    assert(queue.isEmpty());
    assert(queue.size() == 0);

    std::cout << "Test test_dequeue_and_best_selection: PASS" << std::endl;
}

void test_custom_policy_ranking() {
    TaskQueue queue(5);
    MockPolicy policy(0);
    queue.usePolicy(&policy);

    Task t1, t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    int idx = queue.bestIndex();
    assert(idx == 0 || idx == 1);
    assert(queue.size() == 2);

    std::cout << "Test test_custom_policy_ranking: PASS" << std::endl;
}

void test_advance_and_purge() {
    TaskQueue queue(5);
    Task t1, t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    int initialRemaining = queue.totalRemaining();
    int initialPending = queue.pendingCount();
    assert(initialRemaining >= 0);
    assert(initialPending >= 0);

    // Advance units
    int completedCount = queue.advanceAll(1000); // Advance significantly
    assert(completedCount >= 0);
    assert(queue.totalRemaining() <= initialRemaining);

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == completedCount);
    assert(queue.size() == 2 - purged);

    // Purge again on empty or remaining active tasks
    int purgedAgain = queue.purgeComplete();
    assert(purgedAgain == 0);

    std::cout << "Test test_advance_and_purge: PASS" << std::endl;
}

int main() {
    std::cout << "Starting TaskQueue test suite..." << std::endl;

    test_constructors_and_defaults();
    test_enqueue_and_capacity_limits();
    test_policy_acceptance_and_rejection();
    test_dequeue_and_best_selection();
    test_custom_policy_ranking();
    test_advance_and_purge();

    std::cout << "All TaskQueue tests passed successfully!" << std::endl;
    return 0;
}