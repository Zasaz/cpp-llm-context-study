#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#include "TaskQueue.h"
#include "Policy.h"
#include "Task.h"

void test_TaskQueue_constructorsAndInitialState() {
    // Default constructor
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty() == true);
    assert(defaultQueue.isFull() == false);
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.policy == 0);
    assert(defaultQueue.bestIndex() == -1);
    assert(defaultQueue.totalRemaining() == 0);
    assert(defaultQueue.pendingCount() == 0);

    // Parameterized constructor with valid capacity
    TaskQueue customQueue(5);
    assert(customQueue.size() == 0);
    assert(customQueue.capacity == 5);
    assert(customQueue.rejectedCount() == 0);

    // Parameterized constructor with zero or negative capacity (should fallback to 1)
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negQueue(-10);
    assert(negQueue.capacity == 1);

    std::cout << "Test test_TaskQueue_constructorsAndInitialState: PASS" << std::endl;
}

void test_TaskQueue_enqueueAndCapacityLimits() {
    TaskQueue q(2);
    Task t1(1, "Task1", 5, 10);
    Task t2(2, "Task2", 3, 20);
    Task t3(3, "Task3", 1, 30);

    assert(q.enqueue(t1) == 1);
    assert(q.size() == 1);
    assert(!q.isEmpty());
    assert(!q.isFull());

    assert(q.enqueue(t2) == 2);
    assert(q.size() == 2);
    assert(q.isFull());

    // Queue is full, enqueue should fail and increment rejected count
    assert(q.enqueue(t3) == -1);
    assert(q.size() == 2);
    assert(q.rejectedCount() == 1);

    std::cout << "Test test_TaskQueue_enqueueAndCapacityLimits: PASS" << std::endl;
}

void test_TaskQueue_enqueueWithPolicyAcceptance() {
    TaskQueue q(5);
    PriorityPolicy pPolicy(5); // Accepts only priority >= 5
    q.usePolicy(&pPolicy);

    Task validTask(1, "Valid", 10, 5);
    Task invalidTask(2, "Invalid", 2, 5);

    int res1 = q.enqueue(validTask);
    assert(res1 == 1);
    assert(q.size() == 1);
    assert(q.rejectedCount() == 0);

    int res2 = q.enqueue(invalidTask);
    assert(res2 == -1);
    assert(q.size() == 1);
    assert(q.rejectedCount() == 1);

    // Switch policy to a DeadlinePolicy with horizon 15
    DeadlinePolicy dPolicy(15);
    q.usePolicy(&dPolicy);

    Task withinDeadline(3, "Within", 1, 10); // default deadline = 10 <= 15
    Task beyondDeadline(4, "Beyond", 1, 20); // default deadline = 20 > 15

    assert(q.enqueue(withinDeadline) == 2);
    assert(q.enqueue(beyondDeadline) == -1);
    assert(q.rejectedCount() == 2);

    std::cout << "Test test_TaskQueue_enqueueWithPolicyAcceptance: PASS" << std::endl;
}

void test_TaskQueue_bestAndDequeue_noPolicy() {
    TaskQueue q(5);

    // When empty
    assert(q.bestIndex() == -1);
    Task emptyBest = q.best();
    assert(emptyBest.identifier() == 0);
    Task emptyDequeued = q.dequeue();
    assert(emptyDequeued.identifier() == 0);

    // Enqueue tasks without explicit policy (uses Task::weight() -> priority * 10 [+ 25 if overdue])
    Task t1(101, "LowPriority", 1, 10);
    Task t2(102, "HighPriority", 10, 10);
    Task t3(103, "MidPriority", 5, 10);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    assert(q.size() == 3);
    assert(q.bestIndex() == 1); // t2 has highest weight (100)
    assert(q.best().identifier() == 102);

    Task dequeued1 = q.dequeue();
    assert(dequeued1.identifier() == 102);
    assert(q.size() == 2);

    // Next best should be t3 (weight 50)
    assert(q.bestIndex() == 1); // index in remaining tasks: [t1, t3] -> t3 is at 1
    assert(q.best().identifier() == 103);

    Task dequeued2 = q.dequeue();
    assert(dequeued2.identifier() == 103);
    assert(q.size() == 1);

    Task dequeued3 = q.dequeue();
    assert(dequeued3.identifier() == 101);
    assert(q.isEmpty());

    std::cout << "Test test_TaskQueue_bestAndDequeue_noPolicy: PASS" << std::endl;
}

void test_TaskQueue_bestAndDequeue_withPolicies() {
    TaskQueue q(5);

    Task t1(1, "Task1", 5, 20); // Fifo rank with weight 1: -1
    Task t2(2, "Task2", 5, 10); // Fifo rank with weight 1: -2
    Task t3(3, "Task3", 10, 5); // Fifo rank with weight 1: -3

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    // FIFO Policy
    FifoPolicy fifoPolicy;
    q.usePolicy(&fifoPolicy);
    // Highest rank is t1 (-1 > -2 > -3)
    assert(q.bestIndex() == 0);
    assert(q.best().identifier() == 1);

    // Priority Policy: rank = max(priority, floor)*100 - duration + bias
    PriorityPolicy priorityPolicy(0);
    q.usePolicy(&priorityPolicy);
    // t1: 5*100 - 20 = 480
    // t2: 5*100 - 10 = 490
    // t3: 10*100 - 5 = 995
    assert(q.bestIndex() == 2);
    assert(q.best().identifier() == 3);

    Task chosen = q.dequeue();
    assert(chosen.identifier() == 3);
    assert(q.size() == 2);

    // Now among t1 (480) and t2 (490), t2 is better (index 1)
    assert(q.bestIndex() == 1);
    assert(q.best().identifier() == 2);

    std::cout << "Test test_TaskQueue_bestAndDequeue_withPolicies: PASS" << std::endl;
}

void test_TaskQueue_advanceAllAndTotalRemaining() {
    TaskQueue q(5);

    Task t1(1, "Task1", 1, 10);
    Task t2(2, "Task2", 1, 5);
    Task t3(3, "Task3", 1, 20);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    assert(q.totalRemaining() == 35);
    assert(q.pendingCount() == 3);

    // Advance by 5 units
    int completedCount = q.advanceAll(5);
    // t1: remaining 5, running
    // t2: remaining 0, complete
    // t3: remaining 15, running
    assert(completedCount == 1);
    assert(q.totalRemaining() == 20);
    assert(q.pendingCount() == 0);

    // Advance by 0 or negative units (should have no effect)
    assert(q.advanceAll(0) == 1); // t2 is still complete
    assert(q.advanceAll(-5) == 1);
    assert(q.totalRemaining() == 20);

    // Advance by 10 units
    completedCount = q.advanceAll(10);
    // t1: remaining 0, complete
    // t2: remaining 0, complete
    // t3: remaining 5, running
    assert(completedCount == 2);
    assert(q.totalRemaining() == 5);

    std::cout << "Test test_TaskQueue_advanceAllAndTotalRemaining: PASS" << std::endl;
}

void test_TaskQueue_purgeComplete() {
    TaskQueue q(5);

    Task t1(1, "Task1", 1, 5);
    Task t2(2, "Task2", 1, 15);
    Task t3(3, "Task3", 1, 5);
    Task t4(4, "Task4", 1, 20);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);
    q.enqueue(t4);

    // Initially none complete
    assert(q.purgeComplete() == 0);
    assert(q.size() == 4);

    // Advance 5 units, completing t1 and t3
    q.advanceAll(5);
    assert(q.size() == 4);

    int purged = q.purgeComplete();
    assert(purged == 2);
    assert(q.size() == 2);
    assert(q.tasks[0].identifier() == 2);
    assert(q.tasks[1].identifier() == 4);

    // Purge when no completed tasks exist
    assert(q.purgeComplete() == 0);
    assert(q.size() == 2);

    // Advance remaining tasks to completion
    q.advanceAll(100);
    purged = q.purgeComplete();
    assert(purged == 2);
    assert(q.isEmpty());

    std::cout << "Test test_TaskQueue_purgeComplete: PASS" << std::endl;
}

int main() {
    test_TaskQueue_constructorsAndInitialState();
    test_TaskQueue_enqueueAndCapacityLimits();
    test_TaskQueue_enqueueWithPolicyAcceptance();
    test_TaskQueue_bestAndDequeue_noPolicy();
    test_TaskQueue_bestAndDequeue_withPolicies();
    test_TaskQueue_advanceAllAndTotalRemaining();
    test_TaskQueue_purgeComplete();

    std::cout << "All TaskQueue unit tests passed successfully!" << std::endl;
    return 0;
}