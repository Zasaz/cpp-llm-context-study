#include <iostream>
#include <cassert>
#include <vector>
#include "TaskQueue.h"

// Mock policy for testing policy filtering and ranking
class AllowAllPolicy : public Policy {
public:
    virtual bool accepts(const Task&) { return true; }
    virtual bool accepts(const Task&) const { return true; }
    virtual int rank(const Task& t) { return t.weight() * 10; }
    virtual int rank(const Task& t) const { return t.weight() * 10; }
};

class RejectAllPolicy : public Policy {
public:
    virtual bool accepts(const Task&) { return false; }
    virtual bool accepts(const Task&) const { return false; }
    virtual int rank(const Task&) { return 0; }
    virtual int rank(const Task&) const { return 0; }
};

class ReverseRankPolicy : public Policy {
public:
    virtual bool accepts(const Task&) { return true; }
    virtual bool accepts(const Task&) const { return true; }
    virtual int rank(const Task& t) { return -t.weight(); }
    virtual int rank(const Task& t) const { return -t.weight(); }
};

void test_constructors_and_capacity_limits() {
    // Default constructor: capacity should be 8, queue empty
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());
    assert(defaultQueue.rejectedCount() == 0);

    // Custom constructor with positive capacity
    TaskQueue sizedQueue(3);
    assert(sizedQueue.size() == 0);
    assert(sizedQueue.isEmpty());
    assert(!sizedQueue.isFull());
    assert(sizedQueue.rejectedCount() == 0);

    // Custom constructor with 0 (should default capacity to 1)
    TaskQueue zeroQueue(0);
    assert(zeroQueue.size() == 0);
    assert(!zeroQueue.isFull());
    zeroQueue.enqueue(Task());
    assert(zeroQueue.isFull());

    // Custom constructor with negative limit (should default capacity to 1)
    TaskQueue negQueue(-5);
    assert(negQueue.size() == 0);
    assert(!negQueue.isFull());
    negQueue.enqueue(Task());
    assert(negQueue.isFull());

    std::cout << "Test test_constructors_and_capacity_limits: PASS" << std::endl;
}

void test_empty_queue_operations() {
    TaskQueue queue;

    assert(queue.isEmpty());
    assert(queue.size() == 0);
    assert(queue.bestIndex() == -1);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);
    assert(queue.advanceAll(5) == 0);
    assert(queue.purgeComplete() == 0);
    assert(queue.rejectedCount() == 0);

    // best() and dequeue() on empty queue should return default Task
    Task bestEmpty = queue.best();
    Task dequeuedEmpty = queue.dequeue();
    assert(queue.size() == 0);

    std::cout << "Test test_empty_queue_operations: PASS" << std::endl;
}

void test_enqueue_capacity_and_rejection() {
    TaskQueue queue(2);
    assert(!queue.isFull());

    // Enqueue first item
    int res1 = queue.enqueue(Task());
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());

    // Enqueue second item -> reaches capacity
    int res2 = queue.enqueue(Task());
    assert(res2 == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());

    // Enqueue third item -> should be rejected
    int res3 = queue.enqueue(Task());
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    // Enqueue another rejected item
    int res4 = queue.enqueue(Task());
    assert(res4 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_capacity_and_rejection: PASS" << std::endl;
}

void test_dequeue_and_best_selection() {
    TaskQueue queue(5);
    Task t1;
    Task t2;
    Task t3;

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.size() == 3);
    assert(queue.bestIndex() >= 0 && queue.bestIndex() < 3);

    Task bestTask = queue.best();
    assert(queue.size() == 3); // best() should not modify queue

    Task dequeuedTask = queue.dequeue();
    assert(queue.size() == 2);

    queue.dequeue();
    queue.dequeue();
    assert(queue.isEmpty());
    assert(queue.dequeue().remaining() == Task().remaining());

    std::cout << "Test test_dequeue_and_best_selection: PASS" << std::endl;
}

void test_advance_all_and_purge_complete() {
    TaskQueue queue(5);
    Task t1;
    Task t2;

    queue.enqueue(t1);
    queue.enqueue(t2);

    int initialRemaining = queue.totalRemaining();
    int pending = queue.pendingCount();
    assert(pending >= 0);

    // Advance by some units
    int completedCount = queue.advanceAll(1000);
    assert(completedCount >= 0);

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == completedCount);
    assert(queue.size() == 2 - purged);

    std::cout << "Test test_advance_all_and_purge_complete: PASS" << std::endl;
}

void test_policy_filtering_and_ranking() {
    TaskQueue queue(5);
    RejectAllPolicy rejectPolicy;
    AllowAllPolicy allowPolicy;
    ReverseRankPolicy reversePolicy;

    // Set reject policy: all enqueues should be rejected
    queue.usePolicy(&rejectPolicy);
    int res = queue.enqueue(Task());
    assert(res == -1);
    assert(queue.rejectedCount() == 1);
    assert(queue.isEmpty());

    // Switch to allow policy
    queue.usePolicy(&allowPolicy);
    res = queue.enqueue(Task());
    assert(res == 1);
    assert(queue.size() == 1);

    queue.enqueue(Task());
    assert(queue.size() == 2);
    assert(queue.bestIndex() != -1);

    // Switch to reverse ranking policy
    queue.usePolicy(&reversePolicy);
    assert(queue.bestIndex() != -1);

    // Reset policy to null (default weight ranking)
    int sizeAfterReset = queue.usePolicy(nullptr);
    assert(sizeAfterReset == 2);
    assert(queue.bestIndex() != -1);

    std::cout << "Test test_policy_filtering_and_ranking: PASS" << std::endl;
}

int main() {
    test_constructors_and_capacity_limits();
    test_empty_queue_operations();
    test_enqueue_capacity_and_rejection();
    test_dequeue_and_best_selection();
    test_advance_all_and_purge_complete();
    test_policy_filtering_and_ranking();

    std::cout << "\nAll TaskQueue tests passed successfully!" << std::endl;
    return 0;
}