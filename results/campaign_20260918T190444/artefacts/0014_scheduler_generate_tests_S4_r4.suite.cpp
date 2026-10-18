#include <iostream>
#include <cassert>
#include <vector>
#include "TaskQueue.h"
#include "Policy.h"
#include "Task.h"

// Concrete implementation of Policy for testing purposes
class MockPolicy : public Policy {
public:
    bool acceptAll;
    int rankMultiplier;

    MockPolicy(bool accept = true, int multiplier = 1) 
        : acceptAll(accept), rankMultiplier(multiplier) {}

    bool accepts(const Task& task) const override {
        (void)task; // Suppress unused parameter warning
        return acceptAll;
    }

    int rank(const Task& task) const override {
        return task.weight() * rankMultiplier;
    }
};

// Test constructors and default states
void test_constructors_and_initial_state() {
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.policy == 0);

    TaskQueue customQueue(5);
    assert(customQueue.capacity == 5);
    assert(customQueue.isEmpty());
    assert(!customQueue.isFull());
    assert(customQueue.rejectedCount() == 0);

    // Test non-positive limit fallback to capacity = 1
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negQueue(-10);
    assert(negQueue.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

// Test basic enqueue and capacity bounds
void test_enqueue_and_capacity() {
    TaskQueue queue(2);
    Task t1;
    Task t2;
    Task t3;

    assert(queue.enqueue(t1) == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());

    assert(queue.enqueue(t2) == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());

    // Should reject because queue is full
    int result = queue.enqueue(t3);
    assert(result == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    // Another rejected enqueue
    queue.enqueue(t3);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity: PASS" << std::endl;
}

// Test policy integration on enqueue and ranking
void test_policy_behavior() {
    TaskQueue queue(5);
    MockPolicy rejectingPolicy(false, 1);
    
    int sizeAfterPolicy = queue.usePolicy(&rejectingPolicy);
    assert(sizeAfterPolicy == 0);
    assert(queue.policy == &rejectingPolicy);

    Task t1;
    // Should be rejected by policy
    assert(queue.enqueue(t1) == -1);
    assert(queue.size() == 0);
    assert(queue.rejectedCount() == 1);

    MockPolicy acceptingPolicy(true, 2);
    queue.usePolicy(&acceptingPolicy);
    assert(queue.enqueue(t1) == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1);

    // Reset policy to null
    queue.usePolicy(0);
    assert(queue.policy == 0);

    std::cout << "Test test_policy_behavior: PASS" << std::endl;
}

// Test bestIndex, best, and dequeue operations
void test_best_and_dequeue() {
    TaskQueue queue(5);

    // Empty queue edge cases
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    (void)emptyBest;
    Task emptyDequeued = queue.dequeue();
    (void)emptyDequeued;
    assert(queue.isEmpty());

    Task t1;
    Task t2;
    Task t3;

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.size() == 3);
    int chosenIdx = queue.bestIndex();
    assert(chosenIdx >= 0 && chosenIdx < queue.size());

    Task bestTask = queue.best();
    (void)bestTask;
    assert(queue.size() == 3); // best() should not remove element

    Task dequeuedTask = queue.dequeue();
    (void)dequeuedTask;
    assert(queue.size() == 2); // dequeue() removes the element

    queue.dequeue();
    queue.dequeue();
    assert(queue.isEmpty());
    assert(queue.bestIndex() == -1);

    std::cout << "Test test_best_and_dequeue: PASS" << std::endl;
}

// Test totalRemaining and pendingCount calculations
void test_metrics_remaining_and_pending() {
    TaskQueue queue(5);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);

    Task t1;
    Task t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    int remaining = queue.totalRemaining();
    assert(remaining >= 0);

    int pending = queue.pendingCount();
    assert(pending >= 0 && pending <= queue.size());

    std::cout << "Test test_metrics_remaining_and_pending: PASS" << std::endl;
}

// Test advanceAll and purgeComplete functionality
void test_advance_and_purge() {
    TaskQueue queue(5);
    Task t1;
    Task t2;

    queue.enqueue(t1);
    queue.enqueue(t2);

    int completed = queue.advanceAll(100);
    assert(completed >= 0);

    int initialSize = queue.size();
    int purged = queue.purgeComplete();
    assert(purged == completed);
    assert(queue.size() == initialSize - purged);

    // Purging again should remove 0
    assert(queue.purgeComplete() == 0);

    std::cout << "Test test_advance_and_purge: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_enqueue_and_capacity();
    test_policy_behavior();
    test_best_and_dequeue();
    test_metrics_remaining_and_pending();
    test_advance_and_purge();

    std::cout << "All TaskQueue unit tests passed successfully!" << std::endl;
    return 0;
}