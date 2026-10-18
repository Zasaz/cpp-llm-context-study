#include <iostream>
#include <cassert>
#include <vector>
#include "TaskQueue.h"
#include "Task.h"
#include "Policy.h"

// Concrete implementation of Policy for testing purposes
class MockPolicy : public Policy {
public:
    bool acceptAll;
    int rankMultiplier;

    MockPolicy(bool accept = true, int multiplier = 1) 
        : acceptAll(accept), rankMultiplier(multiplier) {}

    virtual bool accepts(const Task& task) const {
        return acceptAll;
    }

    virtual int rank(const Task& task) const {
        return task.weight() * rankMultiplier;
    }
};

// Custom policy to rank by remaining work instead of weight
class RemainingWorkPolicy : public Policy {
public:
    virtual bool accepts(const Task& task) const {
        return task.remaining() > 0;
    }

    virtual int rank(const Task& task) const {
        return task.remaining();
    }
};

void test_constructors_and_initial_state() {
    // Default constructor
    TaskQueue defaultQ;
    assert(defaultQ.size() == 0);
    assert(defaultQ.isEmpty());
    assert(!defaultQ.isFull());
    assert(defaultQ.capacity == 8);
    assert(defaultQ.rejectedCount() == 0);
    assert(defaultQ.policy == 0);
    assert(defaultQ.bestIndex() == -1);

    // Custom limit constructor (valid)
    TaskQueue customQ(5);
    assert(customQ.capacity == 5);
    assert(customQ.size() == 0);
    assert(customQ.isEmpty());
    assert(customQ.rejectedCount() == 0);

    // Custom limit constructor (non-positive edge cases)
    TaskQueue zeroQ(0);
    assert(zeroQ.capacity == 1);

    TaskQueue negQ(-10);
    assert(negQ.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue q(2);
    Task t1, t2, t3;

    // Enqueue within limit
    assert(q.enqueue(t1) == 1);
    assert(q.size() == 1);
    assert(!q.isEmpty());
    assert(!q.isFull());
    assert(q.rejectedCount() == 0);

    assert(q.enqueue(t2) == 2);
    assert(q.size() == 2);
    assert(q.isFull());
    assert(q.rejectedCount() == 0);

    // Enqueue beyond limit
    assert(q.enqueue(t3) == -1);
    assert(q.size() == 2);
    assert(q.rejectedCount() == 1);

    // Another rejected attempt
    assert(q.enqueue(t3) == -1);
    assert(q.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_usePolicy_and_policy_rejection() {
    TaskQueue q(5);
    MockPolicy rejectPolicy(false, 1);
    MockPolicy acceptPolicy(true, 1);

    int size = q.usePolicy(&rejectPolicy);
    assert(size == 0);
    assert(q.policy == &rejectPolicy);

    Task t1;
    // Policy rejects task
    assert(q.enqueue(t1) == -1);
    assert(q.size() == 0);
    assert(q.rejectedCount() == 1);

    // Change to accept policy
    q.usePolicy(&acceptPolicy);
    assert(q.enqueue(t1) == 1);
    assert(q.size() == 1);
    assert(q.rejectedCount() == 1); // Unchanged

    // Remove policy (set to null)
    q.usePolicy(0);
    assert(q.policy == 0);
    assert(q.enqueue(t1) == 2);

    std::cout << "Test test_usePolicy_and_policy_rejection: PASS" << std::endl;
}

void test_best_and_dequeue_without_policy() {
    TaskQueue q(5);

    // Empty queue checks
    assert(q.bestIndex() == -1);
    Task emptyBest = q.best();
    Task emptyDequeued = q.dequeue();
    assert(q.size() == 0);

    Task t1;
    Task t2;
    Task t3;

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    // bestIndex and best check
    int idx = q.bestIndex();
    assert(idx >= 0 && idx < 3);

    int initialSize = q.size();
    Task dequeuedTask = q.dequeue();
    assert(q.size() == initialSize - 1);

    // Dequeue all remaining
    q.dequeue();
    q.dequeue();
    assert(q.isEmpty());
    assert(q.bestIndex() == -1);

    std::cout << "Test test_best_and_dequeue_without_policy: PASS" << std::endl;
}

void test_best_and_dequeue_with_policy() {
    TaskQueue q(5);
    RemainingWorkPolicy policy;
    q.usePolicy(&policy);

    Task t1;
    Task t2;
    q.enqueue(t1);
    q.enqueue(t2);

    assert(q.bestIndex() != -1);
    Task bestTask = q.best();
    int initialSize = q.size();

    Task removedTask = q.dequeue();
    assert(q.size() == initialSize - 1);

    std::cout << "Test test_best_and_dequeue_with_policy: PASS" << std::endl;
}

void test_totalRemaining_and_pendingCount() {
    TaskQueue q(5);
    assert(q.totalRemaining() == 0);
    assert(q.pendingCount() == 0);

    Task t1;
    Task t2;
    q.enqueue(t1);
    q.enqueue(t2);

    int totalRem = q.totalRemaining();
    assert(totalRem == (t1.remaining() + t2.remaining()));

    int pending = q.pendingCount();
    int expectedPending = (t1.isPending() ? 1 : 0) + (t2.isPending() ? 1 : 0);
    assert(pending == expectedPending);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue q(5);
    Task t1;
    Task t2;
    q.enqueue(t1);
    q.enqueue(t2);

    // Advance tasks by a large number of units so they complete
    int completedCount = q.advanceAll(10000);
    assert(completedCount >= 0);

    // Purge completed tasks
    int purgedCount = q.purgeComplete();
    assert(purgedCount == completedCount);
    assert(q.size() == (2 - purgedCount));

    // Purging when no complete tasks exist
    int secondPurge = q.purgeComplete();
    assert(secondPurge == 0);

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

int main() {
    std::cout << "Starting TaskQueue test suite..." << std::endl;

    test_constructors_and_initial_state();
    test_enqueue_and_capacity_limits();
    test_usePolicy_and_policy_rejection();
    test_best_and_dequeue_without_policy();
    test_best_and_dequeue_with_policy();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();

    std::cout << "All TaskQueue tests passed successfully!" << std::endl;
    return 0;
}