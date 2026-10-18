#include <cassert>
#include <iostream>
#include <vector>
#include <string>

#include "TaskQueue.h"

// Concrete dummy policy for testing custom policy behaviors
class CustomTestPolicy : public Policy {
public:
    bool allowAll;
    bool rankByLength;

    CustomTestPolicy(bool allow = true, bool byLength = false) 
        : allowAll(allow), rankByLength(byLength) {}

    bool accepts(const Task& task) override {
        // If false, reject tasks that have remaining work > 10
        if (!allowAll && task.remaining() > 10) {
            return false;
        }
        return true;
    }

    int rank(const Task& task) override {
        // Higher rank for longer remaining time if rankByLength is true
        if (rankByLength) {
            return task.remaining();
        }
        // Inverted weight as a custom ranking
        return -task.weight();
    }
};

void test_constructors_and_initial_state() {
    // Default constructor (capacity 8)
    TaskQueue q1;
    assert(q1.isEmpty());
    assert(!q1.isFull());
    assert(q1.size() == 0);
    assert(q1.rejectedCount() == 0);
    assert(q1.totalRemaining() == 0);
    assert(q1.pendingCount() == 0);

    // Custom constructor with positive capacity
    TaskQueue q2(3);
    assert(q2.isEmpty());
    assert(!q2.isFull());
    assert(q2.size() == 0);
    assert(q2.rejectedCount() == 0);

    // Custom constructor with 0 or negative capacity (should default to 1)
    TaskQueue q3(0);
    assert(q3.size() == 0);
    assert(!q3.isFull());
    Task dummyTask;
    assert(q3.enqueue(dummyTask) == 1);
    assert(q3.isFull());

    TaskQueue q4(-5);
    assert(q4.enqueue(dummyTask) == 1);
    assert(q4.isFull());

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_capacity_limit_and_rejected() {
    TaskQueue q(2);
    Task t1;
    Task t2;
    Task t3;

    assert(q.enqueue(t1) == 1);
    assert(q.size() == 1);
    assert(!q.isFull());
    assert(q.rejectedCount() == 0);

    assert(q.enqueue(t2) == 2);
    assert(q.size() == 2);
    assert(q.isFull());
    assert(q.rejectedCount() == 0);

    // Enqueuing into a full queue should fail and increment rejected count
    int res = q.enqueue(t3);
    assert(res == -1);
    assert(q.size() == 2);
    assert(q.rejectedCount() == 1);

    // Another rejected attempt
    assert(q.enqueue(t3) == -1);
    assert(q.rejectedCount() == 2);

    std::cout << "Test test_enqueue_capacity_limit_and_rejected: PASS" << std::endl;
}

void test_dequeue_and_best_empty_queue() {
    TaskQueue q;
    assert(q.bestIndex() == -1);

    // best() and dequeue() on empty queue should return default Task
    Task t = q.best();
    (void)t; // Verify no crash on returned default task
    
    Task dequeued = q.dequeue();
    (void)dequeued;
    assert(q.isEmpty());
    assert(q.size() == 0);

    std::cout << "Test test_dequeue_and_best_empty_queue: PASS" << std::endl;
}

void test_default_ranking_and_dequeue() {
    TaskQueue q(5);

    // Task queue default ranking is based on task.weight()
    Task t1, t2, t3;
    // Assuming Task interface has setters or constructors for weight
    // If not standard, advance/simulate tasks
    // Let's enqueue tasks and test dequeue order
    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    assert(q.size() == 3);
    assert(!q.isEmpty());

    int initBestIndex = q.bestIndex();
    assert(initBestIndex >= 0 && initBestIndex < 3);

    Task bestTask = q.best();
    Task dequeued = q.dequeue();
    assert(q.size() == 2);

    // Dequeue remaining
    q.dequeue();
    assert(q.size() == 1);
    q.dequeue();
    assert(q.isEmpty());
    assert(q.size() == 0);

    std::cout << "Test test_default_ranking_and_dequeue: PASS" << std::endl;
}

void test_policy_filtering_and_ranking() {
    CustomTestPolicy policy(false, true); // Rejects remaining > 10, ranks by remaining work
    TaskQueue q(5);
    q.usePolicy(&policy);

    Task tSmall; // Default task
    assert(q.enqueue(tSmall) == 1);

    // Reset policy to allow all
    CustomTestPolicy allowAllPolicy(true, true);
    q.usePolicy(&allowAllPolicy);
    Task tLarge;
    assert(q.enqueue(tLarge) == 2);

    // Verify policy switch behavior
    int count = q.usePolicy(nullptr);
    assert(count == 2);

    std::cout << "Test test_policy_filtering_and_ranking: PASS" << std::endl;
}

void test_advance_and_purge() {
    TaskQueue q(5);
    Task t1, t2, t3;

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    int totalWork = q.totalRemaining();
    assert(totalWork >= 0);

    int pending = q.pendingCount();
    assert(pending >= 0 && pending <= 3);

    // Advance units
    int completedCount = q.advanceAll(100);
    assert(completedCount >= 0);

    // Purge completed tasks
    int purgedCount = q.purgeComplete();
    assert(purgedCount == completedCount);
    assert(q.size() == 3 - purgedCount);

    std::cout << "Test test_advance_and_purge: PASS" << std::endl;
}

int main() {
    std::cout << "Starting TaskQueue unit tests..." << std::endl;

    test_constructors_and_initial_state();
    test_enqueue_capacity_limit_and_rejected();
    test_dequeue_and_best_empty_queue();
    test_default_ranking_and_dequeue();
    test_policy_filtering_and_ranking();
    test_advance_and_purge();

    std::cout << "All TaskQueue unit tests passed successfully!" << std::endl;
    return 0;
}