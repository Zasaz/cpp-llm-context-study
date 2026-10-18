#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "TaskQueue.h"
#include "Policy.h"
#include "Task.h"

void test_constructors_and_initial_state() {
    // Test default constructor
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.policy == nullptr);

    // Test parameterized constructor with valid capacity
    TaskQueue customQueue(5);
    assert(customQueue.size() == 0);
    assert(customQueue.isEmpty());
    assert(!customQueue.isFull());
    assert(customQueue.capacity == 5);
    assert(customQueue.rejectedCount() == 0);

    // Test parameterized constructor with zero or negative capacity (should fallback to 1)
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negativeQueue(-10);
    assert(negativeQueue.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue queue(3);

    Task t1(1, "Task1", 5, 10);
    Task t2(2, "Task2", 3, 20);
    Task t3(3, "Task3", 8, 15);
    Task t4(4, "Task4", 1, 5);

    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());

    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);

    int res3 = queue.enqueue(t3);
    assert(res3 == 3);
    assert(queue.size() == 3);
    assert(queue.isFull());
    assert(queue.rejectedCount() == 0);

    // Enqueue beyond capacity should fail
    int res4 = queue.enqueue(t4);
    assert(res4 == -1);
    assert(queue.size() == 3);
    assert(queue.rejectedCount() == 1);

    // Another rejected enqueue
    int res5 = queue.enqueue(t4);
    assert(res5 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_enqueue_with_policy_rejection() {
    TaskQueue queue(5);
    PriorityPolicy minPriorityPolicy(5); // Only accepts priority >= 5
    queue.usePolicy(&minPriorityPolicy);

    Task lowPriorityTask(1, "Low", 3, 10);
    Task highPriorityTask(2, "High", 7, 10);

    // Should be rejected by policy
    int res1 = queue.enqueue(lowPriorityTask);
    assert(res1 == -1);
    assert(queue.size() == 0);
    assert(queue.rejectedCount() == 1);

    // Should be accepted
    int res2 = queue.enqueue(highPriorityTask);
    assert(res2 == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1);

    // Test with DeadlinePolicy
    TaskQueue deadlineQueue(5);
    DeadlinePolicy deadlinePolicy(50); // horizon = 50
    deadlineQueue.usePolicy(&deadlinePolicy);

    Task validDeadlineTask(3, "Quick", 1, 30);
    Task lateDeadlineTask(4, "Late", 1, 60);

    assert(deadlineQueue.enqueue(validDeadlineTask) == 1);
    assert(deadlineQueue.enqueue(lateDeadlineTask) == -1);
    assert(deadlineQueue.rejectedCount() == 1);

    std::cout << "Test test_enqueue_with_policy_rejection: PASS" << std::endl;
}

void test_usePolicy() {
    TaskQueue queue(5);
    queue.enqueue(Task(1, "T1", 2, 10));
    queue.enqueue(Task(2, "T2", 4, 10));

    FifoPolicy fifoPolicy;
    int currentSize = queue.usePolicy(&fifoPolicy);
    assert(currentSize == 2);
    assert(queue.policy == &fifoPolicy);

    // Switch policy to null
    currentSize = queue.usePolicy(nullptr);
    assert(currentSize == 2);
    assert(queue.policy == nullptr);

    std::cout << "Test test_usePolicy: PASS" << std::endl;
}

void test_bestIndex_and_best_without_policy() {
    TaskQueue queue(5);

    // When empty
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    assert(emptyBest.id == 0);
    assert(emptyBest.name == "unnamed");

    // Enqueue tasks with different weights (weight = priority * 10 [+25 if overdue])
    // t1: priority 2, duration 10, deadline 10 -> weight = 20
    Task t1(1, "Task1", 2, 10);
    // t2: priority 8, duration 10, deadline 10 -> weight = 80
    Task t2(2, "Task2", 8, 10);
    // t3: priority 5, duration 10, deadline 10 -> weight = 50
    Task t3(3, "Task3", 5, 10);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1);
    assert(queue.best().id == 2);
    assert(queue.best().name == "Task2");

    std::cout << "Test test_bestIndex_and_best_without_policy: PASS" << std::endl;
}

void test_bestIndex_and_best_with_policies() {
    // 1. Test with FifoPolicy (rank = bias - id * arrivalWeight)
    {
        TaskQueue queue(5);
        FifoPolicy fifo(1);
        queue.usePolicy(&fifo);

        Task t10(10, "Task10", 1, 10); // rank = 0 - 10 = -10
        Task t5(5, "Task5", 1, 10);    // rank = 0 - 5 = -5
        Task t20(20, "Task20", 1, 10); // rank = 0 - 20 = -20

        queue.enqueue(t10);
        queue.enqueue(t5);
        queue.enqueue(t20);

        // Highest score is t5 (-5 > -10 > -20)
        assert(queue.bestIndex() == 1);
        assert(queue.best().id == 5);
    }

    // 2. Test with PriorityPolicy (rank = score * 100 - duration + bias)
    {
        TaskQueue queue(5);
        PriorityPolicy priorityPolicy(0);
        queue.usePolicy(&priorityPolicy);

        Task t1(1, "LowPri", 2, 10);   // rank = 200 - 10 = 190
        Task t2(2, "HighPri", 5, 20);  // rank = 500 - 20 = 480
        Task t3(3, "HighPriShort", 5, 5); // rank = 500 - 5 = 495

        queue.enqueue(t1);
        queue.enqueue(t2);
        queue.enqueue(t3);

        assert(queue.bestIndex() == 2);
        assert(queue.best().id == 3);
    }

    std::cout << "Test test_bestIndex_and_best_with_policies: PASS" << std::endl;
}

void test_dequeue_behavior() {
    TaskQueue queue(5);

    // Dequeue on empty
    Task emptyDequeued = queue.dequeue();
    assert(emptyDequeued.id == 0);
    assert(emptyDequeued.name == "unnamed");

    Task t1(1, "Task1", 1, 10);
    Task t2(2, "Task2", 9, 10);
    Task t3(3, "Task3", 5, 10);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Highest priority should be dequeued first (t2)
    Task d1 = queue.dequeue();
    assert(d1.id == 2);
    assert(queue.size() == 2);

    // Next highest is t3
    Task d2 = queue.dequeue();
    assert(d2.id == 3);
    assert(queue.size() == 1);

    // Last is t1
    Task d3 = queue.dequeue();
    assert(d3.id == 1);
    assert(queue.isEmpty());

    // Next dequeue on now empty queue
    Task d4 = queue.dequeue();
    assert(d4.id == 0);

    std::cout << "Test test_dequeue_behavior: PASS" << std::endl;
}

void test_totalRemaining_and_pendingCount() {
    TaskQueue queue(5);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);

    Task t1(1, "T1", 1, 10); // remaining: 10, pending: true
    Task t2(2, "T2", 2, 20); // remaining: 20, pending: true
    Task t3(3, "T3", 3, 30); // remaining: 30, pending: true

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.totalRemaining() == 60);
    assert(queue.pendingCount() == 3);

    // Modify a task's state in the queue directly to test started / partial progress
    queue.tasks[0].advance(4); // remaining 6, running (not pending)
    assert(queue.totalRemaining() == 56);
    assert(queue.pendingCount() == 2);

    // Complete the task
    queue.tasks[0].advance(6); // remaining 0, finished
    assert(queue.totalRemaining() == 50);
    assert(queue.pendingCount() == 2);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue(5);

    Task t1(1, "T1", 1, 5);
    Task t2(2, "T2", 2, 10);
    Task t3(3, "T3", 3, 15);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Advance all by 5 units: t1 completes, t2 has 5 left, t3 has 10 left
    int completedCount = queue.advanceAll(5);
    assert(completedCount == 1);
    assert(queue.tasks[0].isComplete());
    assert(!queue.tasks[1].isComplete());
    assert(!queue.tasks[2].isComplete());
    assert(queue.totalRemaining() == 15);

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == 1);
    assert(queue.size() == 2);
    assert(queue.tasks[0].id == 2);
    assert(queue.tasks[1].id == 3);

    // Advance remaining tasks by 10 units: both complete
    completedCount = queue.advanceAll(10);
    assert(completedCount == 2);
    assert(queue.totalRemaining() == 0);

    purged = queue.purgeComplete();
    assert(purged == 2);
    assert(queue.isEmpty());

    // Purge on empty queue
    assert(queue.purgeComplete() == 0);

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

int main() {
    std::cout << "--- Running TaskQueue Unit Tests ---" << std::endl;

    test_constructors_and_initial_state();
    test_enqueue_and_capacity_limits();
    test_enqueue_with_policy_rejection();
    test_usePolicy();
    test_bestIndex_and_best_without_policy();
    test_bestIndex_and_best_with_policies();
    test_dequeue_behavior();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();

    std::cout << "All TaskQueue unit tests passed successfully!" << std::endl;
    return 0;
}