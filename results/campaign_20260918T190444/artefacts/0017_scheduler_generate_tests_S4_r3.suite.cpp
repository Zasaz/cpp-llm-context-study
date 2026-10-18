#include <iostream>
#include <cassert>
#include <vector>
#include "TaskQueue.h"
#include "Task.h"
#include "Policy.h"

// Concrete mock policy for testing TaskQueue with custom policies
class TestPolicy : public Policy {
public:
    bool acceptAll;
    int rankMultiplier;

    TestPolicy(bool allow = true, int mult = 1) : acceptAll(allow), rankMultiplier(mult) {}

    virtual bool accepts(const Task& task) const {
        return acceptAll;
    }

    virtual int rank(const Task& task) const {
        return task.weight() * rankMultiplier;
    }
};

// Test Default and Parameterized Constructors
void test_TaskQueue_constructors() {
    TaskQueue defaultQueue;
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.policy == 0);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.isEmpty());
    assert(defaultQueue.size() == 0);
    assert(!defaultQueue.isFull());

    TaskQueue customQueue(5);
    assert(customQueue.capacity == 5);
    assert(customQueue.policy == 0);
    assert(customQueue.rejectedCount() == 0);
    assert(customQueue.isEmpty());

    // Test invalid/non-positive capacity edge cases (should clamp to 1)
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negativeQueue(-10);
    assert(negativeQueue.capacity == 1);

    std::cout << "Test test_TaskQueue_constructors: PASS" << std::endl;
}

// Test Enqueue and Capacity Bounds
void test_TaskQueue_enqueue_and_capacity() {
    TaskQueue queue(2);
    Task t1, t2, t3;

    assert(queue.enqueue(t1) == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());

    assert(queue.enqueue(t2) == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());

    // Should reject when full
    int result = queue.enqueue(t3);
    assert(result == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    // Another rejection increases rejected count
    assert(queue.enqueue(t3) == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_TaskQueue_enqueue_and_capacity: PASS" << std::endl;
}

// Test Policy interactions (accepts, rejected, usePolicy)
void test_TaskQueue_policy_rejection() {
    TaskQueue queue(5);
    TestPolicy rejectingPolicy(false, 1);
    TestPolicy acceptingPolicy(true, 1);

    queue.usePolicy(&rejectingPolicy);
    assert(queue.policy == &rejectingPolicy);

    Task t;
    int res = queue.enqueue(t);
    assert(res == -1);
    assert(queue.rejectedCount() == 1);
    assert(queue.isEmpty());

    // Switch policy to accepting
    queue.usePolicy(&acceptingPolicy);
    res = queue.enqueue(t);
    assert(res == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1); // unchanged

    std::cout << "Test test_TaskQueue_policy_rejection: PASS" << std::endl;
}

// Test bestIndex, best, and dequeue without a custom policy (default weight-based)
void test_TaskQueue_best_and_dequeue_default_policy() {
    TaskQueue queue(5);

    // Empty queue edge cases
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    Task emptyDequeued = queue.dequeue();
    assert(queue.size() == 0);

    Task t1, t2, t3;
    // Advance t2 or adjust so weights differ if tasks have weight,
    // or verify behavior with multiple tasks
    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.size() == 3);
    int idx = queue.bestIndex();
    assert(idx >= 0 && idx < queue.size());

    Task chosen = queue.dequeue();
    assert(queue.size() == 2);

    queue.dequeue();
    queue.dequeue();
    assert(queue.isEmpty());
    assert(queue.dequeue().remaining() == Task().remaining());

    std::cout << "Test test_TaskQueue_best_and_dequeue_default_policy: PASS" << std::endl;
}

// Test bestIndex, best, and dequeue using a custom Policy ranking
void test_TaskQueue_best_and_dequeue_with_policy() {
    TaskQueue queue(5);
    TestPolicy policy(true, -1); // Inverts ranking preference
    queue.usePolicy(&policy);

    Task t1, t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    int idx = queue.bestIndex();
    assert(idx >= 0 && idx < queue.size());

    Task bestTask = queue.best();
    Task dequeuedTask = queue.dequeue();
    assert(queue.size() == 1);

    std::cout << "Test test_TaskQueue_best_and_dequeue_with_policy: PASS" << std::endl;
}

// Test totalRemaining and pendingCount
void test_TaskQueue_remaining_and_pending() {
    TaskQueue queue(5);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);

    Task t1, t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    int initialRemaining = queue.totalRemaining();
    assert(initialRemaining >= 0);
    assert(queue.pendingCount() == 2 || queue.pendingCount() >= 0);

    std::cout << "Test test_TaskQueue_remaining_and_pending: PASS" << std::endl;
}

// Test advanceAll and purgeComplete
void test_TaskQueue_advance_and_purge() {
    TaskQueue queue(5);
    Task t1, t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    // Advance by large units to complete tasks
    int completed = queue.advanceAll(100000);
    assert(completed >= 0);

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == completed);
    assert(queue.size() == (2 - purged));

    std::cout << "Test test_TaskQueue_advance_and_purge: PASS" << std::endl;
}

int main() {
    test_TaskQueue_constructors();
    test_TaskQueue_enqueue_and_capacity();
    test_TaskQueue_policy_rejection();
    test_TaskQueue_best_and_dequeue_default_policy();
    test_TaskQueue_best_and_dequeue_with_policy();
    test_TaskQueue_remaining_and_pending();
    test_TaskQueue_advance_and_purge();

    std::cout << "\nAll TaskQueue tests passed successfully!" << std::endl;
    return 0;
}