#include <iostream>
#include <cassert>
#include <vector>
#include "TaskQueue.h"

// Test Policy subclass to test custom acceptance and ranking behaviors
class TestCustomPolicy : public Policy {
public:
    bool shouldAccept;
    int rankMultiplier;

    TestCustomPolicy(bool accept = true, int multiplier = 1)
        : shouldAccept(accept), rankMultiplier(multiplier) {}

    bool accepts(const Task& /*task*/) override {
        return shouldAccept;
    }

    int rank(const Task& task) override {
        return task.weight() * rankMultiplier;
    }
};

void test_defaultConstructor() {
    TaskQueue queue;
    assert(queue.size() == 0);
    assert(queue.isEmpty());
    assert(!queue.isFull());
    assert(queue.rejectedCount() == 0);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_parameterizedConstructor() {
    // Normal positive limit
    TaskQueue queueNormal(3);
    assert(queueNormal.size() == 0);
    assert(queueNormal.isEmpty());
    assert(!queueNormal.isFull());

    // Zero or negative limit should default capacity to 1
    TaskQueue queueZero(0);
    assert(!queueZero.isFull());
    assert(queueZero.isEmpty());

    Task t1;
    int res1 = queueZero.enqueue(t1);
    assert(res1 == 1);
    assert(queueZero.isFull());

    TaskQueue queueNegative(-5);
    assert(!queueNegative.isFull());
    int res2 = queueNegative.enqueue(t1);
    assert(res2 == 1);
    assert(queueNegative.isFull());

    std::cout << "Test test_parameterizedConstructor: PASS" << std::endl;
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

    // Enqueueing beyond capacity should fail and increase rejectedCount
    int rejectedResult = queue.enqueue(t3);
    assert(rejectedResult == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    // Another rejected enqueue
    assert(queue.enqueue(t3) == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_empty_operations() {
    TaskQueue queue;

    assert(queue.bestIndex() == -1);
    Task bestTask = queue.best();
    // Default task returned
    Task dequeuedTask = queue.dequeue();
    assert(queue.size() == 0);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);
    assert(queue.advanceAll(5) == 0);
    assert(queue.purgeComplete() == 0);

    std::cout << "Test test_empty_operations: PASS" << std::endl;
}

void test_best_and_dequeue_without_policy() {
    TaskQueue queue(5);

    Task t1; // default task
    queue.enqueue(t1);

    assert(queue.bestIndex() == 0);
    assert(queue.size() == 1);

    Task best = queue.best();
    assert(queue.size() == 1); // best does not remove

    Task dequeued = queue.dequeue();
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    // Dequeue from now empty queue
    Task emptyDequeued = queue.dequeue();
    assert(queue.size() == 0);

    std::cout << "Test test_best_and_dequeue_without_policy: PASS" << std::endl;
}

void test_policy_filtering_and_ranking() {
    TaskQueue queue(5);
    TestCustomPolicy rejectPolicy(false, 1);

    queue.usePolicy(&rejectPolicy);

    Task t1;
    // Policy rejects
    int res = queue.enqueue(t1);
    assert(res == -1);
    assert(queue.rejectedCount() == 1);
    assert(queue.size() == 0);

    TestCustomPolicy acceptPolicy(true, 10);
    queue.usePolicy(&acceptPolicy);

    // Policy accepts
    res = queue.enqueue(t1);
    assert(res == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1); // unchanged

    assert(queue.bestIndex() == 0);

    std::cout << "Test test_policy_filtering_and_ranking: PASS" << std::endl;
}

void test_totalRemaining_and_pendingCount() {
    TaskQueue queue(5);
    Task t1;
    Task t2;

    queue.enqueue(t1);
    queue.enqueue(t2);

    int totalRem = queue.totalRemaining();
    int pending = queue.pendingCount();

    assert(totalRem >= 0);
    assert(pending >= 0 && pending <= 2);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue(5);

    Task t1;
    Task t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    assert(queue.size() == 2);

    // Advance all tasks by some units
    int completedCount = queue.advanceAll(100);
    assert(completedCount >= 0 && completedCount <= 2);

    // Purge completed tasks
    int purgedCount = queue.purgeComplete();
    assert(purgedCount == completedCount);
    assert(queue.size() == 2 - purgedCount);

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

void test_usePolicy_return_value() {
    TaskQueue queue(4);
    Task t1;
    Task t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    TestCustomPolicy policy(true, 1);
    int currentSize = queue.usePolicy(&policy);
    assert(currentSize == 2);

    currentSize = queue.usePolicy(nullptr);
    assert(currentSize == 2);

    std::cout << "Test test_usePolicy_return_value: PASS" << std::endl;
}

int main() {
    std::cout << "Starting TaskQueue unit tests..." << std::endl;

    test_defaultConstructor();
    test_parameterizedConstructor();
    test_enqueue_and_capacity_limits();
    test_empty_operations();
    test_best_and_dequeue_without_policy();
    test_policy_filtering_and_ranking();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();
    test_usePolicy_return_value();

    std::cout << "All TaskQueue unit tests passed successfully!" << std::endl;
    return 0;
}