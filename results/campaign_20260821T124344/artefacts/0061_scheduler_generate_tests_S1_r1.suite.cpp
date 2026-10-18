#include <cassert>
#include <iostream>
#include <vector>
#include <string>

#include "TaskQueue.h"
#include "Task.h"
#include "Policy.h"

void test_TaskQueue_defaultConstructor() {
    TaskQueue queue;
    assert(queue.size() == 0);
    assert(queue.isEmpty());
    assert(!queue.isFull());
    assert(queue.capacity == 8);
    assert(queue.rejectedCount() == 0);
    assert(queue.policy == 0);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);
    assert(queue.bestIndex() == -1);

    Task defaultBest = queue.best();
    assert(defaultBest.identifier() == 0);
    assert(defaultBest.label() == "unnamed");

    std::cout << "Test test_TaskQueue_defaultConstructor: PASS" << std::endl;
}

void test_TaskQueue_parameterizedConstructor() {
    TaskQueue queueCustom(5);
    assert(queueCustom.capacity == 5);
    assert(queueCustom.size() == 0);

    // Testing boundary conditions for capacity: <= 0 should set capacity to 1
    TaskQueue queueZero(0);
    assert(queueZero.capacity == 1);

    TaskQueue queueNegative(-5);
    assert(queueNegative.capacity == 1);

    std::cout << "Test test_TaskQueue_parameterizedConstructor: PASS" << std::endl;
}

void test_TaskQueue_enqueueAndCapacity() {
    TaskQueue queue(2);
    Task t1(1, "Task1", 5, 10);
    Task t2(2, "Task2", 3, 20);
    Task t3(3, "Task3", 8, 30);

    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());
    assert(queue.rejectedCount() == 0);

    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());
    assert(queue.rejectedCount() == 0);

    // Should reject because queue is full
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    std::cout << "Test test_TaskQueue_enqueueAndCapacity: PASS" << std::endl;
}

void test_TaskQueue_policyFiltering() {
    TaskQueue queue(5);
    PriorityPolicy priorityPolicy(10); // Only accepts priority >= 10
    queue.usePolicy(&priorityPolicy);

    Task lowPriorityTask(1, "Low", 5, 10);
    Task highPriorityTask(2, "High", 12, 10);

    int res1 = queue.enqueue(lowPriorityTask);
    assert(res1 == -1); // Rejected by policy
    assert(queue.rejectedCount() == 1);
    assert(queue.size() == 0);

    int res2 = queue.enqueue(highPriorityTask);
    assert(res2 == 1); // Accepted
    assert(queue.rejectedCount() == 1);
    assert(queue.size() == 1);

    // Changing policy to DeadlinePolicy
    DeadlinePolicy deadlinePolicy(50);
    queue.usePolicy(&deadlinePolicy);

    Task tightTask(3, "Tight", 1, 20);
    tightTask.setDeadline(40); // <= 50 (accept)
    Task lateTask(4, "Late", 1, 20);
    lateTask.setDeadline(60); // > 50 (reject)

    assert(queue.enqueue(tightTask) == 2);
    assert(queue.enqueue(lateTask) == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_TaskQueue_policyFiltering: PASS" << std::endl;
}

void test_TaskQueue_bestAndDequeueDefaultPolicy() {
    TaskQueue queue(5);
    // When policy is null, rank is task.weight() = priority * 10 (+ 25 if overdue)
    Task t1(1, "LowPri", 2, 10);   // weight = 20
    Task t2(2, "HighPri", 9, 10);  // weight = 90
    Task t3(3, "MidPri", 5, 10);   // weight = 50

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);

    Task dequeued = queue.dequeue();
    assert(dequeued.identifier() == 2);
    assert(queue.size() == 2);

    // Remaining: t1 (idx 0), t3 (idx 1)
    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 3);

    Task dequeued2 = queue.dequeue();
    assert(dequeued2.identifier() == 3);
    assert(queue.size() == 1);

    Task dequeued3 = queue.dequeue();
    assert(dequeued3.identifier() == 1);
    assert(queue.size() == 0);

    // Dequeueing from empty queue
    Task emptyDequeued = queue.dequeue();
    assert(emptyDequeued.identifier() == 0);

    std::cout << "Test test_TaskQueue_bestAndDequeueDefaultPolicy: PASS" << std::endl;
}

void test_TaskQueue_bestAndDequeueCustomPolicies() {
    TaskQueue queue(5);
    Task t1(1, "T1", 5, 10);
    Task t2(2, "T2", 5, 10);
    queue.enqueue(t1);
    queue.enqueue(t2);

    // Test FifoPolicy: rank = bias - task.id * arrivalWeight
    FifoPolicy fifoPolicy(1);
    queue.usePolicy(&fifoPolicy);

    // T1 score: 0 - 1*1 = -1
    // T2 score: 0 - 2*1 = -2
    // T1 has higher score (-1 > -2)
    assert(queue.bestIndex() == 0);
    assert(queue.best().identifier() == 1);

    // Test PriorityPolicy
    PriorityPolicy priorityPolicy(0);
    queue.usePolicy(&priorityPolicy);
    // T3: high priority
    Task t3(3, "T3", 20, 10);
    queue.enqueue(t3);

    assert(queue.best().identifier() == 3);

    std::cout << "Test test_TaskQueue_bestAndDequeueCustomPolicies: PASS" << std::endl;
}

void test_TaskQueue_totalRemainingAndPendingCount() {
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
    queue.tasks[0].advance(4); // remaining 6, running
    assert(queue.totalRemaining() == 41);
    assert(queue.pendingCount() == 2);

    // Advance first task to completion
    queue.tasks[0].advance(6); // remaining 0, finished
    assert(queue.totalRemaining() == 35);
    assert(queue.pendingCount() == 2);

    std::cout << "Test test_TaskQueue_totalRemainingAndPendingCount: PASS" << std::endl;
}

void test_TaskQueue_advanceAllAndPurgeComplete() {
    TaskQueue queue(5);
    Task t1(1, "T1", 1, 5);
    Task t2(2, "T2", 1, 10);
    Task t3(3, "T3", 1, 15);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Advance all by 5 units
    // t1 finishes (5/5)
    // t2 has 5 left (5/10)
    // t3 has 10 left (5/15)
    int completedCount = queue.advanceAll(5);
    assert(completedCount == 1);
    assert(queue.size() == 3);
    assert(queue.tasks[0].isComplete());
    assert(!queue.tasks[1].isComplete());
    assert(!queue.tasks[2].isComplete());

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == 1);
    assert(queue.size() == 2);
    assert(queue.tasks[0].identifier() == 2);
    assert(queue.tasks[1].identifier() == 3);

    // Advance all remaining by 10 units
    // t2 (5 left -> completes)
    // t3 (10 left -> completes)
    int completedCount2 = queue.advanceAll(10);
    assert(completedCount2 == 2);

    int purged2 = queue.purgeComplete();
    assert(purged2 == 2);
    assert(queue.isEmpty());

    // Purge on empty queue
    assert(queue.purgeComplete() == 0);

    std::cout << "Test test_TaskQueue_advanceAllAndPurgeComplete: PASS" << std::endl;
}

int main() {
    test_TaskQueue_defaultConstructor();
    test_TaskQueue_parameterizedConstructor();
    test_TaskQueue_enqueueAndCapacity();
    test_TaskQueue_policyFiltering();
    test_TaskQueue_bestAndDequeueDefaultPolicy();
    test_TaskQueue_bestAndDequeueCustomPolicies();
    test_TaskQueue_totalRemainingAndPendingCount();
    test_TaskQueue_advanceAllAndPurgeComplete();

    std::cout << "\nAll TaskQueue tests passed successfully!" << std::endl;
    return 0;
}