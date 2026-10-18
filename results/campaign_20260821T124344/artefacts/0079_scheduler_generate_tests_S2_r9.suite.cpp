#include <iostream>
#include <cassert>
#include <vector>
#include "TaskQueue.h"

// Mock Policy implementation to test policy-based acceptance and ranking logic.
class MockPolicy : public Policy {
public:
    bool allowAll;
    int minDuration;

    MockPolicy(bool allow = true, int minDur = 0) : allowAll(allow), minDuration(minDur) {}

    bool accepts(const Task& task) override {
        if (!allowAll) return false;
        return task.remaining() >= minDuration;
    }

    int rank(const Task& task) override {
        // Reverse ranking: smaller weight/remaining gets higher score
        return 1000 - task.weight();
    }
};

void test_constructors_and_capacity() {
    // Default constructor (capacity 8)
    TaskQueue q1;
    assert(q1.isEmpty());
    assert(!q1.isFull());
    assert(q1.size() == 0);
    assert(q1.rejectedCount() == 0);

    // Custom limit constructor
    TaskQueue q2(3);
    assert(q2.isEmpty());
    assert(!q2.isFull());
    assert(q2.size() == 0);

    // Non-positive limit constructor fallback (limit <= 0 should become 1)
    TaskQueue q3(0);
    assert(!q3.isFull());
    q3.enqueue(Task("Task1", 5, 2));
    assert(q3.isFull());
    assert(q3.size() == 1);

    TaskQueue q4(-5);
    q4.enqueue(Task("Task1", 5, 2));
    assert(q4.isFull());
    assert(q4.size() == 1);

    std::cout << "Test test_constructors_and_capacity: PASS" << std::endl;
}

void test_enqueue_and_rejection() {
    TaskQueue q(2);
    Task t1("T1", 10, 1);
    Task t2("T2", 20, 2);
    Task t3("T3", 30, 3);

    assert(q.enqueue(t1) == 1);
    assert(q.size() == 1);
    assert(!q.isFull());

    assert(q.enqueue(t2) == 2);
    assert(q.size() == 2);
    assert(q.isFull());

    // Should be rejected because the queue is full
    assert(q.enqueue(t3) == -1);
    assert(q.size() == 2);
    assert(q.rejectedCount() == 1);

    // Reject again
    assert(q.enqueue(t3) == -1);
    assert(q.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_rejection: PASS" << std::endl;
}

void test_best_and_dequeue_default_weight() {
    TaskQueue q;
    
    // Test empty queue behavior
    assert(q.bestIndex() == -1);
    Task emptyBest = q.best();
    Task emptyDequeued = q.dequeue();
    assert(emptyBest.remaining() == 0 || emptyBest.weight() == 0);

    // Enqueue tasks with varying weights
    Task t1("LowWeight", 10, 1);
    Task t2("HighWeight", 10, 10);
    Task t3("MidWeight", 10, 5);

    q.enqueue(t1); // index 0
    q.enqueue(t2); // index 1
    q.enqueue(t3); // index 2

    assert(q.bestIndex() == 1);
    assert(q.best().weight() == 10);
    assert(q.size() == 3);

    // Dequeue the best task
    Task d1 = q.dequeue();
    assert(d1.weight() == 10);
    assert(q.size() == 2);

    // Next best should be MidWeight (weight 5)
    assert(q.bestIndex() == 1); // Now index 1 of remaining [LowWeight, MidWeight]
    assert(q.best().weight() == 5);

    Task d2 = q.dequeue();
    assert(d2.weight() == 5);
    assert(q.size() == 1);

    // Last remaining
    Task d3 = q.dequeue();
    assert(d3.weight() == 1);
    assert(q.isEmpty());

    std::cout << "Test test_best_and_dequeue_default_weight: PASS" << std::endl;
}

void test_policy_filtering_and_ranking() {
    TaskQueue q(5);
    MockPolicy policy(true, 5); // Accepts tasks with remaining >= 5

    assert(q.usePolicy(&policy) == 0);

    Task t1("Short", 3, 10);
    Task t2("LongLowWeight", 10, 2);
    Task t3("LongHighWeight", 12, 8);

    // t1 rejected by policy (< 5 remaining)
    assert(q.enqueue(t1) == -1);
    assert(q.rejectedCount() == 1);
    assert(q.size() == 0);

    // t2 and t3 accepted
    assert(q.enqueue(t2) == 1);
    assert(q.enqueue(t3) == 2);
    assert(q.size() == 2);

    // Policy ranks smaller weight higher (1000 - weight)
    // t2 score: 1000 - 2 = 998
    // t3 score: 1000 - 8 = 992
    // Therefore, t2 is chosen as best
    assert(q.bestIndex() == 0);
    assert(q.best().weight() == 2);

    Task d = q.dequeue();
    assert(d.weight() == 2);
    assert(q.size() == 1);

    // Reset policy to null (reverts to default weight ranking)
    q.usePolicy(0);
    assert(q.best().weight() == 8);

    std::cout << "Test test_policy_filtering_and_ranking: PASS" << std::endl;
}

void test_advance_and_purge() {
    TaskQueue q;
    Task t1("T1", 5, 1);
    Task t2("T2", 10, 1);
    Task t3("T3", 15, 1);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    assert(q.totalRemaining() == 30);
    assert(q.pendingCount() == 3);

    // Advance all by 5 units
    // t1 remaining: 0 (complete)
    // t2 remaining: 5 (pending)
    // t3 remaining: 10 (pending)
    int completed = q.advanceAll(5);
    assert(completed == 1);
    assert(q.totalRemaining() == 15);
    assert(q.pendingCount() == 2);

    // Purge completed tasks
    int purged = q.purgeComplete();
    assert(purged == 1);
    assert(q.size() == 2);

    // Advance remaining by 10 units
    // t2 remaining: 0 (complete)
    // t3 remaining: 0 (complete)
    completed = q.advanceAll(10);
    assert(completed == 2);
    assert(q.totalRemaining() == 0);
    assert(q.pendingCount() == 0);

    purged = q.purgeComplete();
    assert(purged == 2);
    assert(q.isEmpty());
    assert(q.size() == 0);

    // Purging on empty queue
    assert(q.purgeComplete() == 0);

    std::cout << "Test test_advance_and_purge: PASS" << std::endl;
}

int main() {
    test_constructors_and_capacity();
    test_enqueue_and_rejection();
    test_best_and_dequeue_default_weight();
    test_policy_filtering_and_ranking();
    test_advance_and_purge();

    std::cout << "\nAll TaskQueue tests passed successfully!" << std::endl;
    return 0;
}