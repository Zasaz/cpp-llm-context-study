#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "TaskQueue.h"

// Test custom policy implementation to verify policy-driven acceptance and ranking
class CustomTestPolicy : public Policy {
private:
    int minWorkAllowed;
public:
    explicit CustomTestPolicy(int minWork = 0) : minWorkAllowed(minWork) {}

    bool accepts(const Task& task) const {
        return task.remaining() >= minWorkAllowed;
    }

    int rank(const Task& task) const {
        // Ranks higher if task has higher remaining work
        return task.remaining();
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

    // Custom limit with edge cases (zero or negative limit defaults to 1)
    TaskQueue zeroLimitQueue(0);
    assert(zeroLimitQueue.isFull() == false);
    Task t1;
    assert(zeroLimitQueue.enqueue(t1) == 1);
    assert(zeroLimitQueue.isFull() == true);

    TaskQueue negativeLimitQueue(-5);
    assert(negativeLimitQueue.isFull() == false);
    assert(negativeLimitQueue.enqueue(t1) == 1);
    assert(negativeLimitQueue.isFull() == true);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_capacity_and_rejection() {
    TaskQueue queue(2);

    Task t1;
    Task t2;
    Task t3;

    assert(queue.enqueue(t1) == 1);
    assert(queue.size() == 1);
    assert(!queue.isFull());
    assert(queue.rejectedCount() == 0);

    assert(queue.enqueue(t2) == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());
    assert(queue.rejectedCount() == 0);

    // Exceeding capacity should reject and return -1
    assert(queue.enqueue(t3) == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    // Another rejected enqueue
    assert(queue.enqueue(t3) == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_capacity_and_rejection: PASS" << std::endl;
}

void test_best_and_dequeue_default_weight() {
    TaskQueue queue(5);

    // Dequeue and best on empty queue
    Task emptyBest = queue.best();
    Task emptyDequeued = queue.dequeue();
    assert(queue.bestIndex() == -1);
    assert(queue.size() == 0);

    Task t1; // default task
    Task t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    assert(queue.size() == 2);
    assert(queue.bestIndex() == 0);

    Task dequeued1 = queue.dequeue();
    assert(queue.size() == 1);

    Task dequeued2 = queue.dequeue();
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    std::cout << "Test test_best_and_dequeue_default_weight: PASS" << std::endl;
}

void test_policy_filtering_and_ranking() {
    TaskQueue queue(5);
    CustomTestPolicy policy(10); // Rejects tasks with remaining < 10

    queue.usePolicy(&policy);

    Task smallTask; // default constructor likely has 0 or small remaining work
    // If smallTask.remaining() < 10, it should be rejected by the policy
    if (smallTask.remaining() < 10) {
        assert(queue.enqueue(smallTask) == -1);
        assert(queue.rejectedCount() == 1);
        assert(queue.size() == 0);
    }

    // Switch policy to nullptr (no policy)
    queue.usePolicy(0);
    assert(queue.enqueue(smallTask) == 1);
    assert(queue.size() == 1);

    std::cout << "Test test_policy_filtering_and_ranking: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue(5);

    Task t1;
    Task t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    int initialRemaining = queue.totalRemaining();
    assert(initialRemaining >= 0);

    // Advance by some units
    queue.advanceAll(1);

    // Advance by a large number of units to ensure tasks are completed
    int completedCount = queue.advanceAll(100000);
    assert(completedCount == queue.size());
    assert(queue.totalRemaining() == 0);

    // Purge completed tasks
    int purgedCount = queue.purgeComplete();
    assert(purgedCount == completedCount);
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    // Purge on empty queue should return 0
    assert(queue.purgeComplete() == 0);

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

void test_pendingCount_and_totalRemaining() {
    TaskQueue queue(5);
    assert(queue.pendingCount() == 0);
    assert(queue.totalRemaining() == 0);

    Task t1;
    queue.enqueue(t1);

    int pending = queue.pendingCount();
    assert(pending >= 0 && pending <= 1);

    int remaining = queue.totalRemaining();
    assert(remaining >= 0);

    std::cout << "Test test_pendingCount_and_totalRemaining: PASS" << std::endl;
}

int main() {
    std::cout << "Running TaskQueue test suite..." << std::endl;

    test_constructors_and_initial_state();
    test_enqueue_capacity_and_rejection();
    test_best_and_dequeue_default_weight();
    test_policy_filtering_and_ranking();
    test_advanceAll_and_purgeComplete();
    test_pendingCount_and_totalRemaining();

    std::cout << "All TaskQueue tests passed successfully!" << std::endl;
    return 0;
}