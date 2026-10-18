#include <iostream>
#include <cassert>
#include <string>
#include <vector>

#include "TaskQueue.h"
#include "Policy.h"
#include "Task.h"

void test_constructors_and_initial_state() {
    // Default constructor
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.policy == nullptr);
    assert(defaultQueue.bestIndex() == -1);
    assert(defaultQueue.totalRemaining() == 0);
    assert(defaultQueue.pendingCount() == 0);

    // Parameterized constructor with positive limit
    TaskQueue customQueue(3);
    assert(customQueue.capacity == 3);
    assert(customQueue.size() == 0);
    assert(customQueue.isEmpty());
    assert(!customQueue.isFull());
    assert(customQueue.rejectedCount() == 0);

    // Parameterized constructor with zero or negative limit (should default to 1)
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negativeQueue(-5);
    assert(negativeQueue.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue queue(2);

    Task t1(1, "Task1", 5, 10);
    Task t2(2, "Task2", 3, 20);
    Task t3(3, "Task3", 8, 15);

    // Enqueue first task
    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());
    assert(queue.rejectedCount() == 0);

    // Enqueue second task (reaches capacity)
    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);
    assert(!queue.isEmpty());
    assert(queue.isFull());
    assert(queue.rejectedCount() == 0);

    // Enqueue third task when full (should be rejected)
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    // Attempt another rejection
    int res4 = queue.enqueue(t3);
    assert(res4 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_policy_acceptance_and_rejection() {
    TaskQueue queue(5);
    PriorityPolicy priorityPolicy(5); // Rejects tasks with priority < 5
    queue.usePolicy(&priorityPolicy);

    Task validTask(1, "HighPriority", 7, 10);
    Task edgeTask(2, "ExactFloorPriority", 5, 10);
    Task lowTask(3, "LowPriority", 4, 10);

    assert(queue.enqueue(validTask) == 1);
    assert(queue.enqueue(edgeTask) == 2);
    assert(queue.rejectedCount() == 0);

    // Should be rejected by policy
    assert(queue.enqueue(lowTask) == -1);
    assert(queue.rejectedCount() == 1);
    assert(queue.size() == 2);

    // Test with DeadlinePolicy
    DeadlinePolicy deadlinePolicy(50);
    TaskQueue dlQueue(5);
    dlQueue.usePolicy(&deadlinePolicy);

    Task withinDeadline(4, "OnTime", 1, 30); // deadline is 30 <= 50
    Task lateTask(5, "TooLate", 1, 60);       // deadline is 60 > 50

    assert(dlQueue.enqueue(withinDeadline) == 1);
    assert(dlQueue.enqueue(lateTask) == -1);
    assert(dlQueue.rejectedCount() == 1);
    assert(dlQueue.size() == 1);

    // Test base Policy rejection of completed tasks
    Policy basePolicy;
    TaskQueue baseQueue(5);
    baseQueue.usePolicy(&basePolicy);

    Task completedTask(6, "Done", 1, 10);
    completedTask.advance(10);
    assert(completedTask.isComplete());

    assert(baseQueue.enqueue(completedTask) == -1);
    assert(baseQueue.rejectedCount() == 1);

    std::cout << "Test test_policy_acceptance_and_rejection: PASS" << std::endl;
}

void test_best_and_bestIndex_without_policy() {
    TaskQueue queue;

    // Empty queue checks
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    assert(emptyBest.id == 0);
    assert(emptyBest.name == "unnamed");

    // Adding tasks without policy uses Task::weight()
    // Task::weight() = priority * 10 (+ 25 if overdue)
    Task t1(1, "Low", 2, 10);   // weight = 20
    Task t2(2, "High", 8, 10);  // weight = 80
    Task t3(3, "Mid", 5, 10);   // weight = 50

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);

    // Test overdue weight bonus: t1 has base 20. If overdue (elapsed > deadline), weight = 20 + 25 = 45.
    // Let's create an overdue task with base priority 7 (weight 70) and make it overdue -> 70 + 25 = 95
    Task t4(4, "Overdue", 7, 10);
    t4.setDeadline(2);
    t4.advance(5); // elapsed = 5 > deadline (2), isOverdue() is true -> weight = 70 + 25 = 95
    queue.enqueue(t4);

    assert(queue.bestIndex() == 3);
    assert(queue.best().identifier() == 4);

    // Test tie breaking: first task encountered with maximum score is chosen
    TaskQueue tieQueue;
    Task tie1(10, "Tie1", 5, 10); // weight 50
    Task tie2(11, "Tie2", 5, 10); // weight 50
    tieQueue.enqueue(tie1);
    tieQueue.enqueue(tie2);
    assert(tieQueue.bestIndex() == 0);
    assert(tieQueue.best().identifier() == 10);

    std::cout << "Test test_best_and_bestIndex_without_policy: PASS" << std::endl;
}

void test_best_and_bestIndex_with_policies() {
    // 1. FifoPolicy
    // FifoPolicy::rank() = bias - task.id * arrivalWeight
    FifoPolicy fifo(2);
    TaskQueue fifoQueue;
    fifoQueue.usePolicy(&fifo);

    Task ft1(5, "Task_ID_5", 10, 10); // rank = 0 - 5 * 2 = -10
    Task ft2(2, "Task_ID_2", 1, 10);  // rank = 0 - 2 * 2 = -4
    Task ft3(8, "Task_ID_8", 5, 10);  // rank = 0 - 8 * 2 = -16

    fifoQueue.enqueue(ft1);
    fifoQueue.enqueue(ft2);
    fifoQueue.enqueue(ft3);

    assert(fifoQueue.bestIndex() == 1); // ID 2 has highest score (-4)
    assert(fifoQueue.best().identifier() == 2);

    // 2. PriorityPolicy
    // PriorityPolicy::rank() = score * 100 - duration + bias
    PriorityPolicy prioPolicy(2);
    TaskQueue prioQueue;
    prioQueue.usePolicy(&prioPolicy);

    Task pt1(1, "P1", 4, 30); // score = 4 * 100 - 30 = 370
    Task pt2(2, "P2", 4, 10); // score = 4 * 100 - 10 = 390
    Task pt3(3, "P3", 5, 50); // score = 5 * 100 - 50 = 450

    prioQueue.enqueue(pt1);
    prioQueue.enqueue(pt2);
    prioQueue.enqueue(pt3);

    assert(prioQueue.bestIndex() == 2); // pt3 has 450
    assert(prioQueue.best().identifier() == 3);

    // 3. DeadlinePolicy
    // DeadlinePolicy::rank() = max(0, horizon - deadline) * 5 + bias
    DeadlinePolicy dlPolicy(100);
    TaskQueue dlQueue;
    dlQueue.usePolicy(&dlPolicy);

    Task dt1(1, "D1", 1, 80); // urgency = 100 - 80 = 20 -> rank = 100
    Task dt2(2, "D2", 1, 20); // urgency = 100 - 20 = 80 -> rank = 400
    Task dt3(3, "D3", 1, 50); // urgency = 100 - 50 = 50 -> rank = 250

    dlQueue.enqueue(dt1);
    dlQueue.enqueue(dt2);
    dlQueue.enqueue(dt3);

    assert(dlQueue.bestIndex() == 1); // dt2 has 400
    assert(dlQueue.best().identifier() == 2);

    // Adjust bias on policy
    dlPolicy.adjust(50);
    assert(dlQueue.bestIndex() == 1);

    std::cout << "Test test_best_and_bestIndex_with_policies: PASS" << std::endl;
}

void test_dequeue() {
    TaskQueue queue;

    // Dequeue from empty queue
    Task emptyDequeued = queue.dequeue();
    assert(emptyDequeued.id == 0);
    assert(queue.size() == 0);

    Task t1(1, "Low", 2, 10);
    Task t2(2, "High", 9, 10);
    Task t3(3, "Mid", 5, 10);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);
    assert(queue.size() == 3);

    // Dequeue should remove the best task (t2 with priority 9)
    Task popped1 = queue.dequeue();
    assert(popped1.identifier() == 2);
    assert(queue.size() == 2);

    // Remaining are t1 (weight 20) and t3 (weight 50). Next best is t3.
    Task popped2 = queue.dequeue();
    assert(popped2.identifier() == 3);
    assert(queue.size() == 1);

    // Last is t1
    Task popped3 = queue.dequeue();
    assert(popped3.identifier() == 1);
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    // Queue is empty again
    Task popped4 = queue.dequeue();
    assert(popped4.id == 0);

    std::cout << "Test test_dequeue: PASS" << std::endl;
}

void test_totalRemaining_and_pendingCount() {
    TaskQueue queue;

    Task t1(1, "T1", 1, 10);
    Task t2(2, "T2", 1, 20);
    Task t3(3, "T3", 1, 15);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.totalRemaining() == 45);
    assert(queue.pendingCount() == 3);

    // Start advancing some tasks directly in queue
    queue.tasks[0].advance(4); // remaining 6, isRunning = true, isPending = false
    assert(queue.totalRemaining() == 41);
    assert(queue.pendingCount() == 2);

    queue.tasks[1].advance(20); // remaining 0, isComplete = true, isPending = false
    assert(queue.totalRemaining() == 21);
    assert(queue.pendingCount() == 1);

    // Advance remaining units
    queue.tasks[2].advance(15);
    assert(queue.totalRemaining() == 6);
    assert(queue.pendingCount() == 0);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue;

    Task t1(1, "T1", 1, 5);
    Task t2(2, "T2", 1, 10);
    Task t3(3, "T3", 1, 15);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Advance all by 4 units:
    // T1: duration 5, elapsed 4 -> not complete
    // T2: duration 10, elapsed 4 -> not complete
    // T3: duration 15, elapsed 4 -> not complete
    int completed1 = queue.advanceAll(4);
    assert(completed1 == 0);
    assert(queue.totalRemaining() == (1 + 6 + 11));

    // Advance all by 3 more units (total elapsed 7):
    // T1: duration 5 -> completes (elapsed capped at 5)
    // T2: duration 10, elapsed 7 -> not complete
    // T3: duration 15, elapsed 7 -> not complete
    int completed2 = queue.advanceAll(3);
    assert(completed2 == 1); // 1 task complete (T1)

    // Advance all by 0 or negative units: should do nothing
    assert(queue.advanceAll(0) == 1);
    assert(queue.advanceAll(-2) == 1);

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == 1);
    assert(queue.size() == 2);
    assert(queue.tasks[0].identifier() == 2);
    assert(queue.tasks[1].identifier() == 3);

    // Advance remaining tasks to completion
    int completed3 = queue.advanceAll(10);
    assert(completed3 == 2); // Both T2 and T3 complete now

    int purged2 = queue.purgeComplete();
    assert(purged2 == 2);
    assert(queue.isEmpty());
    assert(queue.size() == 0);

    // Purging on empty queue
    assert(queue.purgeComplete() == 0);

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

void test_usePolicy_and_dynamic_switching() {
    TaskQueue queue;
    Task t1(1, "LowPriorityLateDeadline", 2, 20);
    Task t2(2, "HighPriorityEarlyDeadline", 8, 80);

    queue.enqueue(t1);
    queue.enqueue(t2);

    // Without policy: priority * 10 -> t2 has weight 80, t1 has weight 20
    assert(queue.bestIndex() == 1);

    // Switch to FifoPolicy: higher score for lower task id
    FifoPolicy fifoPolicy;
    int sizeRet = queue.usePolicy(&fifoPolicy);
    assert(sizeRet == 2);
    assert(queue.bestIndex() == 0); // t1 has id 1, t2 has id 2 -> t1 is chosen

    // Switch to DeadlinePolicy (horizon 100):
    // t1: deadline 20 -> urgency 80 -> rank 400
    // t2: deadline 80 -> urgency 20 -> rank 100
    DeadlinePolicy dlPolicy(100);
    queue.usePolicy(&dlPolicy);
    assert(queue.bestIndex() == 0); // t1 has rank 400

    // Switch back to no policy (nullptr)
    queue.usePolicy(nullptr);
    assert(queue.bestIndex() == 1); // t2 has higher weight

    std::cout << "Test test_usePolicy_and_dynamic_switching: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_enqueue_and_capacity_limits();
    test_policy_acceptance_and_rejection();
    test_best_and_bestIndex_without_policy();
    test_best_and_bestIndex_with_policies();
    test_dequeue();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();
    test_usePolicy_and_dynamic_switching();

    std::cout << "\nAll TaskQueue tests passed successfully!" << std::endl;
    return 0;
}