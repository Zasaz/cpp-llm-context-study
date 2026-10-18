#include <iostream>
#include <cassert>
#include <vector>
#include <string>

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

    // Parametric constructor with positive limit
    TaskQueue customQueue(3);
    assert(customQueue.size() == 0);
    assert(customQueue.isEmpty());
    assert(!customQueue.isFull());
    assert(customQueue.capacity == 3);
    assert(customQueue.rejectedCount() == 0);

    // Parametric constructor with non-positive limit (should default to 1)
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

    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());
    assert(queue.rejectedCount() == 0);

    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);
    assert(!queue.isEmpty());
    assert(queue.isFull());
    assert(queue.rejectedCount() == 0);

    // Attempting to enqueue to full queue should fail and increment rejectedCount
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    // Another rejected enqueue
    int res4 = queue.enqueue(t1);
    assert(res4 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_policy_rejection_on_enqueue() {
    TaskQueue queue(5);
    PriorityPolicy priorityPolicy(10); // Accepts task.priority >= 10
    queue.usePolicy(&priorityPolicy);

    Task lowPriorityTask(1, "Low", 5, 10);
    Task highPriorityTask(2, "High", 12, 10);

    int res1 = queue.enqueue(lowPriorityTask);
    assert(res1 == -1);
    assert(queue.size() == 0);
    assert(queue.rejectedCount() == 1);

    int res2 = queue.enqueue(highPriorityTask);
    assert(res2 == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1);

    // Test completed task rejection with base Policy
    Policy basePolicy;
    queue.usePolicy(&basePolicy);
    Task finishedTask(3, "Done", 10, 5);
    finishedTask.advance(5);
    assert(finishedTask.isComplete());

    int res3 = queue.enqueue(finishedTask);
    assert(res3 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_policy_rejection_on_enqueue: PASS" << std::endl;
}

void test_best_and_bestIndex_without_policy() {
    TaskQueue queue;
    // On empty queue
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    assert(emptyBest.identifier() == 0);
    assert(emptyBest.label() == "unnamed");

    // Without policy, best uses Task::weight() -> priority * 10 (+ 25 if overdue)
    Task t1(1, "Low", 2, 10);    // weight = 20
    Task t2(2, "High", 5, 10);   // weight = 50
    Task t3(3, "Medium", 3, 10); // weight = 30

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);
    assert(queue.best().label() == "High");

    std::cout << "Test test_best_and_bestIndex_without_policy: PASS" << std::endl;
}

void test_best_and_bestIndex_with_various_policies() {
    TaskQueue queue;
    Task t1(1, "First", 5, 20);
    Task t2(2, "Second", 10, 5);
    Task t3(3, "Third", 10, 15);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Test FifoPolicy: rank = bias - task.id * arrivalWeight (Lower id gets higher rank)
    FifoPolicy fifoPolicy;
    queue.usePolicy(&fifoPolicy);
    assert(queue.bestIndex() == 0);
    assert(queue.best().identifier() == 1);

    // Test PriorityPolicy: rank = score * 100 - duration + bias
    // t1 score = 5*100 - 20 = 480
    // t2 score = 10*100 - 5 = 995
    // t3 score = 10*100 - 15 = 985
    PriorityPolicy prioPolicy;
    queue.usePolicy(&prioPolicy);
    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);

    // Test DeadlinePolicy: rank = (horizon - deadline) * 5 + bias
    // t1 deadline=20 => urgency=80 => rank=400
    // t2 deadline=5  => urgency=95 => rank=475
    // t3 deadline=15 => urgency=85 => rank=425
    DeadlinePolicy deadlinePolicy(100);
    queue.usePolicy(&deadlinePolicy);
    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);

    // Set custom deadline on t1 to make it urgently due at limit 1
    queue.tasks[0].setDeadline(1); // t1 urgency = 99 => rank = 495
    assert(queue.bestIndex() == 0);
    assert(queue.best().identifier() == 1);

    std::cout << "Test test_best_and_bestIndex_with_various_policies: PASS" << std::endl;
}

void test_dequeue() {
    TaskQueue queue;
    // Empty dequeue returns default task
    Task emptyDequeued = queue.dequeue();
    assert(emptyDequeued.identifier() == 0);

    Task t1(10, "Task10", 1, 10);
    Task t2(20, "Task20", 9, 10);
    Task t3(30, "Task30", 5, 10);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);
    assert(queue.size() == 3);

    // Without policy, Task20 has highest priority weight
    Task d1 = queue.dequeue();
    assert(d1.identifier() == 20);
    assert(queue.size() == 2);

    // Next highest is Task30
    Task d2 = queue.dequeue();
    assert(d2.identifier() == 30);
    assert(queue.size() == 1);

    // Last is Task10
    Task d3 = queue.dequeue();
    assert(d3.identifier() == 10);
    assert(queue.isEmpty());
    assert(queue.size() == 0);

    // Dequeueing from now empty queue
    Task d4 = queue.dequeue();
    assert(d4.identifier() == 0);

    std::cout << "Test test_dequeue: PASS" << std::endl;
}

void test_totalRemaining_and_pendingCount() {
    TaskQueue queue;
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);

    Task t1(1, "T1", 1, 10);
    Task t2(2, "T2", 1, 20);
    Task t3(3, "T3", 1, 30);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.totalRemaining() == 60);
    assert(queue.pendingCount() == 3);

    // Start advancing a task directly in the queue vector
    queue.tasks[0].advance(4); // remaining: 6, started: true (running)
    assert(queue.totalRemaining() == 56);
    assert(queue.pendingCount() == 2);

    queue.tasks[1].advance(20); // remaining: 0, finished: true (complete)
    assert(queue.totalRemaining() == 36);
    assert(queue.pendingCount() == 1);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue;
    Task t1(1, "T1", 1, 10);
    Task t2(2, "T2", 1, 15);
    Task t3(3, "T3", 1, 5);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Advance all by 5 units
    // t1: elapsed=5, remaining=5
    // t2: elapsed=5, remaining=10
    // t3: elapsed=5, remaining=0 (finished)
    int completed1 = queue.advanceAll(5);
    assert(completed1 == 1);
    assert(queue.size() == 3);
    assert(queue.totalRemaining() == 15);

    // Advance all by another 5 units
    // t1: elapsed=10, remaining=0 (finished)
    // t2: elapsed=10, remaining=5
    // t3: unchanged (already finished) -> completed count returns tasks that are complete
    int completed2 = queue.advanceAll(5);
    assert(completed2 == 2); // t1 and t3 are complete
    assert(queue.totalRemaining() == 5);

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == 2);
    assert(queue.size() == 1);
    assert(queue.tasks[0].identifier() == 2);
    assert(queue.totalRemaining() == 5);

    // Purge when none are complete
    int purgedAgain = queue.purgeComplete();
    assert(purgedAgain == 0);
    assert(queue.size() == 1);

    // Advance the last one to completion and purge
    queue.advanceAll(10);
    assert(queue.purgeComplete() == 1);
    assert(queue.isEmpty());

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_enqueue_and_capacity_limits();
    test_policy_rejection_on_enqueue();
    test_best_and_bestIndex_without_policy();
    test_best_and_bestIndex_with_various_policies();
    test_dequeue();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();

    std::cout << "\nAll TaskQueue unit tests passed successfully!" << std::endl;
    return 0;
}