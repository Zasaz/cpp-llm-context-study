#include <iostream>
#include <cassert>
#include <vector>
#include "TaskQueue.h"
#include "Task.h"
#include "Policy.h"

// Mock policy for testing Policy acceptance and ranking behavior
class TestMockPolicy : public Policy {
private:
    bool shouldAccept;
    int fixedRank;

public:
    TestMockPolicy(bool accept = true, int rankVal = 0)
        : shouldAccept(accept), fixedRank(rankVal) {}

    void setAccept(bool accept) {
        shouldAccept = accept;
    }

    void setRank(int rankVal) {
        fixedRank = rankVal;
    }

    virtual bool accepts(const Task& /*task*/) const {
        return shouldAccept;
    }

    virtual bool accepts(const Task& /*task*/) {
        return shouldAccept;
    }

    virtual int rank(const Task& /*task*/) const {
        return fixedRank;
    }

    virtual int rank(const Task& /*task*/) {
        return fixedRank;
    }
};

void test_defaultConstructor() {
    TaskQueue queue;
    assert(queue.size() == 0);
    assert(queue.isEmpty());
    assert(!queue.isFull());
    assert(queue.rejectedCount() == 0);
    assert(queue.bestIndex() == -1);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

void test_parameterizedConstructor_capacities() {
    // Valid positive capacity
    TaskQueue q1(3);
    assert(q1.size() == 0);
    assert(q1.isEmpty());
    assert(!q1.isFull());

    // Zero capacity should default to 1
    TaskQueue q2(0);
    assert(q2.isEmpty());
    assert(!q2.isFull());
    Task t;
    assert(q2.enqueue(t) == 1);
    assert(q2.isFull());

    // Negative capacity should default to 1
    TaskQueue q3(-5);
    assert(q3.isEmpty());
    assert(!q3.isFull());
    assert(q3.enqueue(t) == 1);
    assert(q3.isFull());

    std::cout << "Test test_parameterizedConstructor_capacities: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
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

    // Enqueueing beyond capacity should fail and increment rejected count
    assert(queue.enqueue(t3) == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    // Another rejected enqueue
    assert(queue.enqueue(t3) == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_empty_queue_operations() {
    TaskQueue queue;
    assert(queue.bestIndex() == -1);

    // best() on empty returns default Task
    Task b = queue.best();
    (void)b;

    // dequeue() on empty returns default Task and does not crash
    Task d = queue.dequeue();
    (void)d;
    assert(queue.isEmpty());
    assert(queue.size() == 0);

    std::cout << "Test test_empty_queue_operations: PASS" << std::endl;
}

void test_enqueue_and_dequeue() {
    TaskQueue queue(5);
    Task t1;
    Task t2;
    Task t3;

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);
    assert(queue.size() == 3);

    assert(queue.bestIndex() >= 0 && queue.bestIndex() < queue.size());

    Task removed = queue.dequeue();
    (void)removed;
    assert(queue.size() == 2);

    queue.dequeue();
    assert(queue.size() == 1);

    queue.dequeue();
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    std::cout << "Test test_enqueue_and_dequeue: PASS" << std::endl;
}

void test_totalRemaining_and_pendingCount() {
    TaskQueue queue(5);
    Task t1;
    Task t2;

    int expectedRemaining = t1.remaining() + t2.remaining();
    int expectedPending = (t1.isPending() ? 1 : 0) + (t2.isPending() ? 1 : 0);

    queue.enqueue(t1);
    queue.enqueue(t2);

    assert(queue.totalRemaining() == expectedRemaining);
    assert(queue.pendingCount() == expectedPending);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue(5);
    Task t1;
    Task t2;

    queue.enqueue(t1);
    queue.enqueue(t2);

    int completed = queue.advanceAll(1000);
    assert(completed >= 0 && completed <= 2);

    int purged = queue.purgeComplete();
    assert(purged == completed);
    assert(queue.size() == 2 - purged);

    // Purging again when no completed tasks remain should return 0
    int secondPurge = queue.purgeComplete();
    assert(secondPurge == 0);

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

void test_usePolicy_and_acceptance() {
    TaskQueue queue(5);
    TestMockPolicy rejectingPolicy(false);

    int currentSize = queue.usePolicy(&rejectingPolicy);
    assert(currentSize == 0);

    Task t1;
    // Policy rejects task
    assert(queue.enqueue(t1) == -1);
    assert(queue.rejectedCount() == 1);
    assert(queue.size() == 0);

    TestMockPolicy acceptingPolicy(true);
    queue.usePolicy(&acceptingPolicy);

    // Policy accepts task
    assert(queue.enqueue(t1) == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1); // Unchanged

    // Resetting policy to nullptr (0)
    queue.usePolicy(nullptr);
    assert(queue.enqueue(t1) == 2);
    assert(queue.size() == 2);

    std::cout << "Test test_usePolicy_and_acceptance: PASS" << std::endl;
}

void test_policy_ranking() {
    TaskQueue queue(5);
    TestMockPolicy policy(true, 100);
    queue.usePolicy(&policy);

    Task t1;
    Task t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    // Both have the same score, bestIndex should return 0 (first seen highest)
    assert(queue.bestIndex() == 0);

    std::cout << "Test test_policy_ranking: PASS" << std::endl;
}

int main() {
    test_defaultConstructor();
    test_parameterizedConstructor_capacities();
    test_enqueue_and_capacity_limits();
    test_empty_queue_operations();
    test_enqueue_and_dequeue();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();
    test_usePolicy_and_acceptance();
    test_policy_ranking();

    std::cout << "All TaskQueue tests passed successfully!" << std::endl;
    return 0;
}