#include <iostream>
#include <cassert>
#include <vector>
#include "TaskQueue.h"

// Concrete Policy for testing custom acceptance and ranking behaviors
class CustomPolicy : public Policy {
public:
    int minWorkAllowed;
    int multiplier;

    CustomPolicy(int minWork = 0, int mult = 1) 
        : minWorkAllowed(minWork), multiplier(mult) {}

    bool accepts(const Task& task) const override {
        return task.remaining() >= minWorkAllowed;
    }

    int rank(const Task& task) const override {
        return task.remaining() * multiplier;
    }
};

void test_constructors_and_initial_state() {
    // Default constructor
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty() == true);
    assert(defaultQueue.isFull() == false);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.totalRemaining() == 0);
    assert(defaultQueue.pendingCount() == 0);
    assert(defaultQueue.bestIndex() == -1);

    // Custom limit constructor
    TaskQueue customQueue(3);
    assert(customQueue.size() == 0);
    assert(customQueue.isEmpty() == true);
    assert(customQueue.isFull() == false);
    assert(customQueue.rejectedCount() == 0);

    // Boundary/invalid limit (< 1) should fallback to capacity = 1
    TaskQueue zeroLimitQueue(0);
    assert(zeroLimitQueue.size() == 0);
    assert(!zeroLimitQueue.isFull());

    TaskQueue negativeLimitQueue(-5);
    assert(negativeLimitQueue.size() == 0);
    assert(!negativeLimitQueue.isFull());

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue queue(2);

    Task t1;
    Task t2;
    Task t3;

    assert(queue.enqueue(t1) == 1);
    assert(queue.size() == 1);
    assert(!queue.isFull());
    assert(!queue.isEmpty());

    assert(queue.enqueue(t2) == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());

    // Queue is full; should reject subsequent enqueue
    assert(queue.enqueue(t3) == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    // Fallback limit capacity check (limit <= 0 => capacity 1)
    TaskQueue singleSlot(-10);
    assert(singleSlot.enqueue(t1) == 1);
    assert(singleSlot.isFull());
    assert(singleSlot.enqueue(t2) == -1);
    assert(singleSlot.rejectedCount() == 1);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_policy_filtering() {
    TaskQueue queue(5);
    CustomPolicy policy(5, 1); // Only accepts tasks with remaining() >= 5
    queue.usePolicy(&policy);

    Task smallTask; // Default Task typically has remaining < 5 or 0
    // Try to enqueue small task
    if (smallTask.remaining() < 5) {
        assert(queue.enqueue(smallTask) == -1);
        assert(queue.rejectedCount() == 1);
        assert(queue.isEmpty());
    }

    // Switch policy to allow everything
    CustomPolicy permissivePolicy(0, 1);
    queue.usePolicy(&permissivePolicy);
    assert(queue.enqueue(smallTask) == 1);
    assert(queue.size() == 1);

    std::cout << "Test test_policy_filtering: PASS" << std::endl;
}

void test_best_and_dequeue_default_weight() {
    TaskQueue queue(5);
    
    // Empty queue checks
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    Task emptyDequeued = queue.dequeue();
    assert(queue.isEmpty());

    Task t1;
    Task t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    assert(queue.size() == 2);
    int bestIdx = queue.bestIndex();
    assert(bestIdx >= 0 && bestIdx < 2);

    Task dequeued = queue.dequeue();
    assert(queue.size() == 1);

    queue.dequeue();
    assert(queue.isEmpty());
    assert(queue.bestIndex() == -1);

    std::cout << "Test test_best_and_dequeue_default_weight: PASS" << std::endl;
}

void test_advance_and_purge() {
    TaskQueue queue(5);

    Task t1;
    Task t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    int totalBefore = queue.totalRemaining();
    // Advance all tasks by large amount to complete them
    queue.advanceAll(1000);

    // After advancing a large amount, total remaining should decrease or be 0
    assert(queue.totalRemaining() <= totalBefore);

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged >= 0);
    assert(queue.size() == 2 - purged);

    std::cout << "Test test_advance_and_purge: PASS" << std::endl;
}

void test_policy_ranking() {
    TaskQueue queue(5);
    CustomPolicy policy(0, 2);
    queue.usePolicy(&policy);

    Task t1;
    Task t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    int idx = queue.bestIndex();
    assert(idx == 0 || idx == 1);

    Task bestTask = queue.best();
    Task dequeuedTask = queue.dequeue();
    assert(queue.size() == 1);

    std::cout << "Test test_policy_ranking: PASS" << std::endl;
}

int main() {
    std::cout << "Starting TaskQueue test suite..." << std::endl;

    test_constructors_and_initial_state();
    test_enqueue_and_capacity_limits();
    test_policy_filtering();
    test_best_and_dequeue_default_weight();
    test_advance_and_purge();
    test_policy_ranking();

    std::cout << "All TaskQueue tests passed successfully!" << std::endl;
    return 0;
}