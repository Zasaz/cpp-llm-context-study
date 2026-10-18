#include <cassert>
#include <iostream>
#include <vector>
#include <string>

#include "TaskQueue.h"
#include "Task.h"
#include "Policy.h"

// Test constructors and initial state
void test_TaskQueue_constructors() {
    // Default constructor
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.policy == 0);
    assert(defaultQueue.rejectedCount() == 0);

    // Parametric constructor with valid positive limit
    TaskQueue customQueue(5);
    assert(customQueue.size() == 0);
    assert(customQueue.isEmpty());
    assert(!customQueue.isFull());
    assert(customQueue.capacity == 5);
    assert(customQueue.policy == 0);
    assert(customQueue.rejectedCount() == 0);

    // Parametric constructor with zero or negative limit (should default to 1)
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negativeQueue(-10);
    assert(negativeQueue.capacity == 1);

    std::cout << "Test test_TaskQueue_constructors: PASS" << std::endl;
}

// Test basic enqueue and dequeue operations without custom policy (using Task weight)
void test_TaskQueue_enqueue_dequeue_defaultPolicy() {
    TaskQueue queue(3);

    Task t1(1, "Task1", 5, 10);
    Task t2(2, "Task2", 10, 20);
    Task t3(3, "Task3", 2, 5);

    assert(queue.enqueue(t1) == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());

    assert(queue.enqueue(t2) == 2);
    assert(queue.size() == 2);

    assert(queue.enqueue(t3) == 3);
    assert(queue.size() == 3);
    assert(queue.isFull());

    // Queue is full, enqueue should fail and increment rejected count
    Task t4(4, "Task4", 8, 15);
    assert(queue.enqueue(t4) == -1);
    assert(queue.rejectedCount() == 1);
    assert(queue.size() == 3);

    // best() should return t2 (highest weight = 10 * 10 = 100)
    assert(queue.bestIndex() == 1);
    Task bestTask = queue.best();
    assert(bestTask.identifier() == 2);
    assert(queue.size() == 3); // best does not remove

    // Dequeue should return t2 and remove it
    Task popped1 = queue.dequeue();
    assert(popped1.identifier() == 2);
    assert(queue.size() == 2);
    assert(!queue.isFull());

    // Next best should be t1 (priority 5 -> weight 50)
    assert(queue.bestIndex() == 0);
    Task popped2 = queue.dequeue();
    assert(popped2.identifier() == 1);
    assert(queue.size() == 1);

    // Next best should be t3 (priority 2 -> weight 20)
    Task popped3 = queue.dequeue();
    assert(popped3.identifier() == 3);
    assert(queue.isEmpty());

    // Dequeue on empty queue returns default Task (id 0) and bestIndex -1
    assert(queue.bestIndex() == -1);
    Task emptyTask = queue.dequeue();
    assert(emptyTask.identifier() == 0);
    assert(emptyTask.label() == "unnamed");

    Task bestOnEmpty = queue.best();
    assert(bestOnEmpty.identifier() == 0);

    std::cout << "Test test_TaskQueue_enqueue_dequeue_defaultPolicy: PASS" << std::endl;
}

// Test policy application: FifoPolicy
void test_TaskQueue_fifoPolicy() {
    TaskQueue queue(5);
    FifoPolicy fifo(2); // arrivalWeight = 2, rank = bias - id * 2
    queue.usePolicy(&fifo);

    Task t10(10, "Task10", 100, 5); // rank = 0 - 20 = -20
    Task t5(5, "Task5", 1, 5);       // rank = 0 - 10 = -10 (higher)
    Task t1(1, "Task1", 1, 5);       // rank = 0 - 2 = -2 (highest)

    queue.enqueue(t10);
    queue.enqueue(t5);
    queue.enqueue(t1);

    assert(queue.bestIndex() == 2); // t1 has rank -2
    Task best = queue.dequeue();
    assert(best.identifier() == 1);

    assert(queue.bestIndex() == 1); // t5 has rank -10
    best = queue.dequeue();
    assert(best.identifier() == 5);

    assert(queue.bestIndex() == 0); // t10 has rank -20
    best = queue.dequeue();
    assert(best.identifier() == 10);

    std::cout << "Test test_TaskQueue_fifoPolicy: PASS" << std::endl;
}

// Test policy application: PriorityPolicy and rejection filtering
void test_TaskQueue_priorityPolicy_and_rejection() {
    TaskQueue queue(5);
    PriorityPolicy priorityPolicy(5); // minimum priority = 5
    queue.usePolicy(&priorityPolicy);

    Task lowPriority(1, "Low", 3, 10);
    Task validPriority(2, "Valid", 7, 10);
    Task highPriority(3, "High", 9, 2);

    // lowPriority should be rejected because priority (3) < floorPriority (5)
    int res1 = queue.enqueue(lowPriority);
    assert(res1 == -1);
    assert(queue.rejectedCount() == 1);
    assert(queue.size() == 0);

    int res2 = queue.enqueue(validPriority);
    assert(res2 == 1);
    assert(queue.rejectedCount() == 1);

    int res3 = queue.enqueue(highPriority);
    assert(res3 == 2);

    // highPriority rank: 9 * 100 - 2 = 898
    // validPriority rank: 7 * 100 - 10 = 690
    assert(queue.bestIndex() == 1);
    Task best = queue.dequeue();
    assert(best.identifier() == 3);

    std::cout << "Test test_TaskQueue_priorityPolicy_and_rejection: PASS" << std::endl;
}

// Test policy application: DeadlinePolicy
void test_TaskQueue_deadlinePolicy() {
    TaskQueue queue(5);
    DeadlinePolicy deadlinePolicy(50); // horizon = 50
    queue.usePolicy(&deadlinePolicy);

    Task tFar(1, "Far", 1, 10);
    tFar.setDeadline(60); // Exceeds horizon (50) -> should be rejected

    Task tClose(2, "Close", 1, 10);
    tClose.setDeadline(10); // rank = (50 - 10)*5 = 200

    Task tMid(3, "Mid", 1, 10);
    tMid.setDeadline(30); // rank = (50 - 30)*5 = 100

    assert(queue.enqueue(tFar) == -1);
    assert(queue.rejectedCount() == 1);

    assert(queue.enqueue(tClose) == 1);
    assert(queue.enqueue(tMid) == 2);

    assert(queue.bestIndex() == 0); // tClose has higher rank
    Task best = queue.dequeue();
    assert(best.identifier() == 2);

    std::cout << "Test test_TaskQueue_deadlinePolicy: PASS" << std::endl;
}

// Test task progression, advanceAll, and purgeComplete
void test_TaskQueue_advanceAll_and_purgeComplete() {
    TaskQueue queue(10);

    Task t1(1, "Short", 1, 3);
    Task t2(2, "Medium", 1, 6);
    Task t3(3, "Long", 1, 10);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.pendingCount() == 3);
    assert(queue.totalRemaining() == 3 + 6 + 10); // 19

    // Advance 3 units: t1 completes, t2 remaining 3, t3 remaining 7
    int completed = queue.advanceAll(3);
    assert(completed == 1);
    assert(queue.pendingCount() == 0); // All have started
    assert(queue.totalRemaining() == 0 + 3 + 7); // 10
    assert(queue.size() == 3); // Purge hasn't run yet

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == 1);
    assert(queue.size() == 2);
    assert(queue.tasks[0].identifier() == 2);
    assert(queue.tasks[1].identifier() == 3);

    // Advance another 4 units: t2 completes (needs 3), t3 remaining 3 (needs 7-4)
    completed = queue.advanceAll(4);
    assert(completed == 1);
    assert(queue.totalRemaining() == 0 + 3);

    purged = queue.purgeComplete();
    assert(purged == 1);
    assert(queue.size() == 1);
    assert(queue.tasks[0].identifier() == 3);

    // Purge when no completed tasks exist
    assert(queue.purgeComplete() == 0);
    assert(queue.size() == 1);

    // Advance remaining units for t3
    completed = queue.advanceAll(5);
    assert(completed == 1);
    assert(queue.totalRemaining() == 0);
    assert(queue.purgeComplete() == 1);
    assert(queue.isEmpty());

    std::cout << "Test test_TaskQueue_advanceAll_and_purgeComplete: PASS" << std::endl;
}

// Test edge cases: empty queue calculations, zero advances, policy switching
void test_TaskQueue_edgeCases() {
    TaskQueue queue(2);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);
    assert(queue.advanceAll(5) == 0);
    assert(queue.purgeComplete() == 0);

    Task t1(1, "Task1", 5, 10);
    queue.enqueue(t1);

    // Advance by 0 or negative
    assert(queue.advanceAll(0) == 0);
    assert(queue.advanceAll(-5) == 0);
    assert(queue.pendingCount() == 1); // Still pending as 0/neg units do not start task

    // Switching policy dynamically
    FifoPolicy fifo;
    PriorityPolicy priority;

    assert(queue.usePolicy(&fifo) == 1);
    assert(queue.policy == &fifo);

    assert(queue.usePolicy(&priority) == 1);
    assert(queue.policy == &priority);

    assert(queue.usePolicy(0) == 1);
    assert(queue.policy == 0);

    std::cout << "Test test_TaskQueue_edgeCases: PASS" << std::endl;
}

int main() {
    std::cout << "Starting TaskQueue unit tests..." << std::endl;

    test_TaskQueue_constructors();
    test_TaskQueue_enqueue_dequeue_defaultPolicy();
    test_TaskQueue_fifoPolicy();
    test_TaskQueue_priorityPolicy_and_rejection();
    test_TaskQueue_deadlinePolicy();
    test_TaskQueue_advanceAll_and_purgeComplete();
    test_TaskQueue_edgeCases();

    std::cout << "All TaskQueue unit tests passed successfully!" << std::endl;
    return 0;
}