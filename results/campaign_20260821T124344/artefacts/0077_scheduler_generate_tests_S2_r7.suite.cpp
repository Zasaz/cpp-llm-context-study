#include <cassert>
#include <iostream>
#include <vector>
#include <string>

#include "TaskQueue.h"

// Mock policy for testing policy-related functionalities in TaskQueue
class TestPolicy : public Policy {
public:
    bool allowAll;
    int rankMultiplier;

    TestPolicy(bool allow = true, int multiplier = 1)
        : allowAll(allow), rankMultiplier(multiplier) {}

    bool accepts(const Task& task) const {
        return allowAll;
    }

    int rank(const Task& task) const {
        return task.weight() * rankMultiplier;
    }
};

void test_constructors_and_initial_state() {
    // Default constructor
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.totalRemaining() == 0);
    assert(defaultQueue.pendingCount() == 0);
    assert(defaultQueue.bestIndex() == -1);

    // Custom limit constructor
    TaskQueue customQueue(3);
    assert(customQueue.size() == 0);
    assert(customQueue.isEmpty());
    assert(!customQueue.isFull());
    assert(customQueue.rejectedCount() == 0);

    // Non-positive limit constructor (should default capacity to at least 1)
    TaskQueue zeroQueue(0);
    assert(zeroQueue.size() == 0);
    assert(!zeroQueue.isFull());

    TaskQueue negativeQueue(-5);
    assert(negativeQueue.size() == 0);
    assert(!negativeQueue.isFull());

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity() {
    TaskQueue queue(2);

    Task t1;
    Task t2;
    Task t3;

    // Enqueue within capacity
    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());

    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());

    // Enqueue exceeding capacity
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    int res4 = queue.enqueue(t3);
    assert(res4 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity: PASS" << std::endl;
}

void test_best_and_dequeue_without_policy() {
    TaskQueue queue(5);

    // Testing on empty queue
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    Task emptyDequeued = queue.dequeue();
    assert(queue.size() == 0);

    // Enqueue tasks
    Task t1; // default task
    queue.enqueue(t1);

    assert(queue.bestIndex() == 0);
    Task b1 = queue.best();
    assert(queue.size() == 1);

    Task d1 = queue.dequeue();
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    std::cout << "Test test_best_and_dequeue_without_policy: PASS" << std::endl;
}

void test_policy_interaction() {
    TaskQueue queue(5);
    TestPolicy rejectPolicy(false, 1);
    TestPolicy acceptPolicy(true, 2);

    // Set policy that rejects everything
    queue.usePolicy(&rejectPolicy);

    Task t1;
    int res = queue.enqueue(t1);
    assert(res == -1);
    assert(queue.rejectedCount() == 1);
    assert(queue.isEmpty());

    // Switch to accepting policy
    queue.usePolicy(&acceptPolicy);
    res = queue.enqueue(t1);
    assert(res == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1); // unchanged

    // Remove policy (set to nullptr / 0)
    queue.usePolicy(0);
    Task t2;
    res = queue.enqueue(t2);
    assert(res == 2);
    assert(queue.size() == 2);

    std::cout << "Test test_policy_interaction: PASS" << std::endl;
}

void test_totalRemaining_and_pendingCount() {
    TaskQueue queue(5);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);

    Task t1;
    Task t2;

    queue.enqueue(t1);
    queue.enqueue(t2);

    // Total remaining and pending count based on tasks
    assert(queue.totalRemaining() >= 0);
    assert(queue.pendingCount() >= 0);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue(5);

    Task t1;
    Task t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    assert(queue.size() == 2);

    // Advance tasks by 0 or positive units
    int completed = queue.advanceAll(0);
    assert(completed >= 0);

    // Advance by a large number of units to ensure completion
    completed = queue.advanceAll(100000);
    assert(completed >= 0);

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == completed);
    assert(queue.size() == (2 - purged));

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_enqueue_and_capacity();
    test_best_and_dequeue_without_policy();
    test_policy_interaction();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();

    std::cout << "All TaskQueue tests passed successfully!" << std::endl;
    return 0;
}