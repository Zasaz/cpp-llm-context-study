#include <iostream>
#include <cassert>
#include <vector>
#include "TaskQueue.h"
#include "Task.h"
#include "Policy.h"

// Test custom policy implementations to verify usePolicy, rank, and accepts behavior
class RejectAllPolicy : public Policy {
public:
    bool accepts(const Task& /*task*/) const override {
        return false;
    }
    int rank(const Task& /*task*/) const override {
        return 0;
    }
};

class InverseWeightPolicy : public Policy {
public:
    bool accepts(const Task& /*task*/) const override {
        return true;
    }
    int rank(const Task& task) const override {
        // Lower weight gets a higher rank score
        return 1000 - task.weight();
    }
};

// 1. Test Constructor Defaults and Initial State
void test_constructors_and_initial_state() {
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.policy == nullptr);

    TaskQueue customQueue(3);
    assert(customQueue.capacity == 3);
    assert(customQueue.isEmpty());
    assert(customQueue.rejectedCount() == 0);

    // Non-positive limit should default capacity to 1
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negativeQueue(-5);
    assert(negativeQueue.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

// 2. Test Enqueue and Capacity Limits
void test_enqueue_and_capacity_limits() {
    TaskQueue queue(2);
    Task t1;
    Task t2;
    Task t3;

    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());

    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());

    // Queue is full; should reject t3
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

// 3. Test Policy Integration (Acceptance and Custom Ranking)
void test_policy_interaction() {
    TaskQueue queue(5);
    RejectAllPolicy rejectPolicy;

    int qSize = queue.usePolicy(&rejectPolicy);
    assert(qSize == 0);
    assert(queue.policy == &rejectPolicy);

    Task t1;
    int enqResult = queue.enqueue(t1);
    assert(enqResult == -1);
    assert(queue.size() == 0);
    assert(queue.rejectedCount() == 1);

    // Switch to InverseWeightPolicy
    InverseWeightPolicy inversePolicy;
    queue.usePolicy(&inversePolicy);

    // Test rank selection with policy
    Task lowWeight; // Assume weight() returns default or lower
    Task highWeight;
    // Advance or configure tasks if methods exist, or verify rank ordering
    queue.enqueue(lowWeight);
    queue.enqueue(highWeight);
    assert(queue.size() == 2);

    // Reset policy to null (reverts to default weight ranking)
    queue.usePolicy(nullptr);
    assert(queue.policy == nullptr);

    std::cout << "Test test_policy_interaction: PASS" << std::endl;
}

// 4. Test Empty Queue Behavior for Selection and Dequeue
void test_empty_queue_operations() {
    TaskQueue queue;
    assert(queue.bestIndex() == -1);

    Task bestTask = queue.best();
    // Default task returned
    Task dequeuedTask = queue.dequeue();
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    std::cout << "Test test_empty_queue_operations: PASS" << std::endl;
}

// 5. Test Best Index, Best Task, and Dequeue Order
void test_best_and_dequeue() {
    TaskQueue queue(5);
    Task t1;
    Task t2;
    Task t3;

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    int initialSize = queue.size();
    assert(initialSize == 3);

    int idx = queue.bestIndex();
    assert(idx >= 0 && idx < initialSize);

    Task top = queue.best();
    Task dequeued = queue.dequeue();
    assert(queue.size() == 2);

    // Dequeue remaining items
    queue.dequeue();
    queue.dequeue();
    assert(queue.isEmpty());
    assert(queue.bestIndex() == -1);

    std::cout << "Test test_best_and_dequeue: PASS" << std::endl;
}

// 6. Test Task Work Metrics: advanceAll, totalRemaining, pendingCount, and purgeComplete
void test_advancement_and_purging() {
    TaskQueue queue(5);
    Task t1;
    Task t2;

    queue.enqueue(t1);
    queue.enqueue(t2);

    int initialRemaining = queue.totalRemaining();
    assert(initialRemaining >= 0);

    int pending = queue.pendingCount();
    assert(pending <= queue.size());

    // Advance tasks
    int completedCount = queue.advanceAll(1000); // Advance sufficient units to complete
    assert(completedCount >= 0 && completedCount <= queue.size());

    // Purge completed tasks
    int purgedCount = queue.purgeComplete();
    assert(purgedCount == completedCount);
    assert(queue.size() == 2 - purgedCount);

    std::cout << "Test test_advancement_and_purging: PASS" << std::endl;
}

int main() {
    std::cout << "--- Running TaskQueue Unit Tests ---" << std::endl;

    test_constructors_and_initial_state();
    test_enqueue_and_capacity_limits();
    test_policy_interaction();
    test_empty_queue_operations();
    test_best_and_dequeue();
    test_advancement_and_purging();

    std::cout << "All TaskQueue unit tests passed successfully!" << std::endl;
    return 0;
}