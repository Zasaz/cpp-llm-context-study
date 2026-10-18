#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#include "TaskQueue.h"
#include "Policy.h"
#include "Task.h"

// Test TaskQueue constructors and initial state
void test_TaskQueue_constructors() {
    // Default constructor
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty() == true);
    assert(defaultQueue.isFull() == false);
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.policy == nullptr);

    // Parametric constructor with valid limit
    TaskQueue customQueue(4);
    assert(customQueue.capacity == 4);
    assert(customQueue.isEmpty() == true);
    assert(customQueue.rejectedCount() == 0);

    // Parametric constructor with invalid (zero / negative) limit, should default capacity to 1
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negativeQueue(-5);
    assert(negativeQueue.capacity == 1);

    std::cout << "Test test_TaskQueue_constructors: PASS" << std::endl;
}

// Test enqueueing, capacity limits, and rejection tracking
void test_TaskQueue_enqueue_and_capacity() {
    TaskQueue queue(2);
    Task t1(1, "Task1", 10, 5);
    Task t2(2, "Task2", 20, 10);
    Task t3(3, "Task3", 30, 15);

    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());

    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());

    // Should be rejected due to capacity
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    std::cout << "Test test_TaskQueue_enqueue_and_capacity: PASS" << std::endl;
}

// Test usePolicy and policy acceptance/rejection on enqueue
void test_TaskQueue_usePolicy_and_acceptance() {
    TaskQueue queue(5);
    PriorityPolicy priorityPolicy(5); // Only accepts priority >= 5

    queue.usePolicy(&priorityPolicy);
    assert(queue.policy == &priorityPolicy);

    Task highP(1, "HighPriority", 10, 5);
    Task lowP(2, "LowPriority", 3, 5);

    int resHigh = queue.enqueue(highP);
    assert(resHigh == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 0);

    // Should be rejected by policy
    int resLow = queue.enqueue(lowP);
    assert(resLow == -1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1);

    // Policy rejecting completed tasks
    Policy basePolicy;
    queue.usePolicy(&basePolicy);
    Task completedTask(3, "Completed", 10, 5);
    completedTask.advance(5); // Marks it complete
    assert(completedTask.isComplete());

    int resCompleted = queue.enqueue(completedTask);
    assert(resCompleted == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_TaskQueue_usePolicy_and_acceptance: PASS" << std::endl;
}

// Test ranking and selection (bestIndex, best, dequeue) with no policy (uses Task::weight)
void test_TaskQueue_best_and_dequeue_no_policy() {
    TaskQueue queue(5);

    // On empty queue
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    assert(emptyBest.id == 0);
    assert(emptyBest.name == "unnamed");
    Task emptyDequeued = queue.dequeue();
    assert(emptyDequeued.id == 0);

    Task t1(1, "Low", 2, 5);    // weight: 20
    Task t2(2, "High", 8, 5);   // weight: 80
    Task t3(3, "Medium", 5, 5); // weight: 50

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1);
    assert(queue.best().id == 2);
    assert(queue.size() == 3);

    Task dequeued = queue.dequeue();
    assert(dequeued.id == 2);
    assert(queue.size() == 2);

    // Next best should be t3
    assert(queue.bestIndex() == 1); // t3 is now at index 1
    assert(queue.best().id == 3);

    // Check overdue weight boost
    Task overdueTask(4, "Overdue", 2, 5); // base weight 20 + 25 = 45
    overdueTask.setDeadline(2);
    overdueTask.advance(3); // elapsed = 3 > deadline (2) -> overdue
    assert(overdueTask.isOverdue());
    queue.enqueue(overdueTask);

    // Low is 20, Overdue is 45, Medium is 50. Medium is still best.
    assert(queue.best().id == 3);
    queue.dequeue(); // dequeues Medium

    // Now Overdue (45) should be greater than Low (20)
    assert(queue.best().id == 4);

    std::cout << "Test test_TaskQueue_best_and_dequeue_no_policy: PASS" << std::endl;
}

// Test ranking and selection with various policies (FifoPolicy, PriorityPolicy, DeadlinePolicy)
void test_TaskQueue_best_with_policies() {
    TaskQueue queue(5);
    Task t1(1, "First", 5, 10);
    Task t2(2, "Second", 10, 5);
    Task t3(3, "Third", 8, 2);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // FifoPolicy: rank = bias - task.id * arrivalWeight (lower id gets higher rank)
    FifoPolicy fifoPolicy;
    queue.usePolicy(&fifoPolicy);
    assert(queue.best().id == 1);

    // PriorityPolicy: rank = max(priority, floorPriority) * 100 - duration + bias
    PriorityPolicy priorityPolicy(0);
    queue.usePolicy(&priorityPolicy);
    // t1: 5 * 100 - 10 = 490
    // t2: 10 * 100 - 5 = 995
    // t3: 8 * 100 - 2 = 798
    assert(queue.best().id == 2);

    // DeadlinePolicy: rank = max(0, horizon - deadline) * 5 + bias (earlier deadline gets higher rank)
    DeadlinePolicy deadlinePolicy(20);
    t1.setDeadline(15);
    t2.setDeadline(5);
    t3.setDeadline(1);

    queue.tasks[0] = t1;
    queue.tasks[1] = t2;
    queue.tasks[2] = t3;

    queue.usePolicy(&deadlinePolicy);
    // t1: (20 - 15) * 5 = 25
    // t2: (20 - 5) * 5 = 75
    // t3: (20 - 1) * 5 = 95
    assert(queue.best().id == 3);

    std::cout << "Test test_TaskQueue_best_with_policies: PASS" << std::endl;
}

// Test totalRemaining and pendingCount
void test_TaskQueue_aggregates() {
    TaskQueue queue(5);
    Task t1(1, "T1", 1, 10);
    Task t2(2, "T2", 1, 15);
    Task t3(3, "T3", 1, 20);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.totalRemaining() == 45);
    assert(queue.pendingCount() == 3);

    // Advance first task partially
    queue.tasks[0].advance(4); // remaining 6, started
    assert(queue.tasks[0].isRunning());
    assert(queue.totalRemaining() == 41);
    assert(queue.pendingCount() == 2); // t2 and t3 still pending

    std::cout << "Test test_TaskQueue_aggregates: PASS" << std::endl;
}

// Test advanceAll and purgeComplete
void test_TaskQueue_advanceAll_and_purgeComplete() {
    TaskQueue queue(5);
    Task t1(1, "Short", 1, 3);
    Task t2(2, "Medium", 1, 6);
    Task t3(3, "Long", 1, 10);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Advance all by 4 units: t1 completes (duration 3), t2 and t3 advance
    int completedCount = queue.advanceAll(4);
    assert(completedCount == 1);
    assert(queue.tasks[0].isComplete());
    assert(!queue.tasks[1].isComplete());
    assert(!queue.tasks[2].isComplete());
    assert(queue.tasks[1].remaining() == 2);
    assert(queue.tasks[2].remaining() == 6);

    // Purge completed tasks
    int purgedCount = queue.purgeComplete();
    assert(purgedCount == 1);
    assert(queue.size() == 2);
    assert(queue.tasks[0].id == 2);
    assert(queue.tasks[1].id == 3);

    // Advance remaining by 10 units: both should complete
    completedCount = queue.advanceAll(10);
    assert(completedCount == 2);
    assert(queue.tasks[0].isComplete());
    assert(queue.tasks[1].isComplete());

    purgedCount = queue.purgeComplete();
    assert(purgedCount == 2);
    assert(queue.isEmpty());
    assert(queue.size() == 0);

    // Purge on empty queue
    assert(queue.purgeComplete() == 0);

    std::cout << "Test test_TaskQueue_advanceAll_and_purgeComplete: PASS" << std::endl;
}

int main() {
    std::cout << "--- Starting TaskQueue Test Suite ---" << std::endl;

    test_TaskQueue_constructors();
    test_TaskQueue_enqueue_and_capacity();
    test_TaskQueue_usePolicy_and_acceptance();
    test_TaskQueue_best_and_dequeue_no_policy();
    test_TaskQueue_best_with_policies();
    test_TaskQueue_aggregates();
    test_TaskQueue_advanceAll_and_purgeComplete();

    std::cout << "--- All Tests Passed Successfully ---" << std::endl;
    return 0;
}