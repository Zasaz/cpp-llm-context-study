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
    bool rankByRemaining;

    TestPolicy(bool accept = true, bool byRemaining = false) 
        : acceptAll(accept), rankByRemaining(byRemaining) {}

    virtual bool accepts(const Task& task) const {
        if (!acceptAll) {
            return false;
        }
        // Custom rule: accept only tasks with positive remaining time
        return task.remaining() > 0;
    }

    virtual int rank(const Task& task) const {
        if (rankByRemaining) {
            return task.remaining();
        }
        return task.weight() * 2; // custom rank scale
    }
};

void test_constructors_and_defaults() {
    // Default constructor
    TaskQueue q1;
    assert(q1.size() == 0);
    assert(q1.isEmpty() == true);
    assert(q1.isFull() == false);
    assert(q1.capacity == 8);
    assert(q1.rejectedCount() == 0);
    assert(q1.policy == 0);

    // Custom limit constructor
    TaskQueue q2(5);
    assert(q2.capacity == 5);
    assert(q2.size() == 0);
    assert(q2.isEmpty() == true);

    // Limit <= 0 should default to capacity 1
    TaskQueue q3(0);
    assert(q3.capacity == 1);

    TaskQueue q4(-5);
    assert(q4.capacity == 1);

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue q(2);
    Task t1;
    Task t2;
    Task t3;

    assert(q.enqueue(t1) == 1);
    assert(q.size() == 1);
    assert(!q.isEmpty());
    assert(!q.isFull());
    assert(q.rejectedCount() == 0);

    assert(q.enqueue(t2) == 2);
    assert(q.size() == 2);
    assert(q.isFull());
    assert(q.rejectedCount() == 0);

    // Enqueue into full queue should reject and increment rejected count
    int res = q.enqueue(t3);
    assert(res == -1);
    assert(q.size() == 2);
    assert(q.rejectedCount() == 1);

    // Another rejection
    assert(q.enqueue(t3) == -1);
    assert(q.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_use_policy_and_enqueue_rejection() {
    TaskQueue q(5);
    TestPolicy rejectPolicy(false);

    q.usePolicy(&rejectPolicy);
    assert(q.policy == &rejectPolicy);

    Task t1;
    // Policy rejects
    int res = q.enqueue(t1);
    assert(res == -1);
    assert(q.size() == 0);
    assert(q.rejectedCount() == 1);

    // Reset policy to null
    q.usePolicy(0);
    assert(q.policy == 0);
    assert(q.enqueue(t1) == 1);
    assert(q.rejectedCount() == 1);

    std::cout << "Test test_use_policy_and_enqueue_rejection: PASS" << std::endl;
}

void test_best_and_dequeue_empty() {
    TaskQueue q;
    assert(q.bestIndex() == -1);

    Task tBest = q.best();
    // Default Task on empty queue
    Task tDeq = q.dequeue();
    assert(q.size() == 0);

    std::cout << "Test test_best_and_dequeue_empty: PASS" << std::endl;
}

void test_best_and_dequeue_without_policy() {
    TaskQueue q(5);

    Task t1; // Remaining or Weight based on default Task behaviour
    Task t2;
    Task t3;

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    int idx = q.bestIndex();
    assert(idx >= 0 && idx < q.size());

    Task chosen = q.best();
    int initialSize = q.size();
    Task dequeued = q.dequeue();
    
    assert(q.size() == initialSize - 1);

    std::cout << "Test test_best_and_dequeue_without_policy: PASS" << std::endl;
}

void test_best_and_dequeue_with_policy() {
    TaskQueue q(5);
    TestPolicy policy(true, true); // rank by remaining
    q.usePolicy(&policy);

    Task t1;
    Task t2;

    q.enqueue(t1);
    q.enqueue(t2);

    int idx = q.bestIndex();
    assert(idx >= 0 && idx < 2);

    Task bestTask = q.best();
    Task deqTask = q.dequeue();
    assert(q.size() == 1);

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

    int expectedRemaining = t1.remaining() + t2.remaining();
    assert(q.totalRemaining() == expectedRemaining);

    int expectedPending = (t1.isPending() ? 1 : 0) + (t2.isPending() ? 1 : 0);
    assert(q.pendingCount() == expectedPending);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue q(5);
    Task t1;
    Task t2;
    q.enqueue(t1);
    q.enqueue(t2);

    // Advance by large units to complete tasks if applicable
    int completed = q.advanceAll(1000);
    assert(completed >= 0);

    int purged = q.purgeComplete();
    assert(purged == completed);
    assert(q.size() == 2 - purged);

    // Purging again when no completed tasks exist
    assert(q.purgeComplete() == 0);

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

int main() {
    test_constructors_and_defaults();
    test_enqueue_and_capacity_limits();
    test_use_policy_and_enqueue_rejection();
    test_best_and_dequeue_empty();
    test_best_and_dequeue_without_policy();
    test_best_and_dequeue_with_policy();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();

    std::cout << "\nAll TaskQueue tests passed successfully!" << std::endl;
    return 0;
}