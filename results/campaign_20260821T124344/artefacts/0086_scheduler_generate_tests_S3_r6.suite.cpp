#include <cassert>
#include <iostream>
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
    assert(defaultQueue.policy == nullptr);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.totalRemaining() == 0);
    assert(defaultQueue.pendingCount() == 0);
    assert(defaultQueue.bestIndex() == -1);

    // Parameterized constructor with valid capacity
    TaskQueue customQueue(5);
    assert(customQueue.capacity == 5);
    assert(customQueue.isEmpty());

    // Parameterized constructor with invalid/zero/negative capacity (should default to 1)
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negativeQueue(-10);
    assert(negativeQueue.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue queue(2);
    Task t1(1, "Task 1", 5, 10);
    Task t2(2, "Task 2", 3, 20);
    Task t3(3, "Task 3", 1, 15);

    // Enqueue within capacity
    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());

    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());

    // Enqueue exceeding capacity
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    // Another rejection increases counter
    queue.enqueue(t3);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_enqueue_with_policy_acceptance() {
    TaskQueue queue(5);
    PriorityPolicy minPriorityPolicy(5); // Rejects tasks with priority < 5
    queue.usePolicy(&minPriorityPolicy);

    Task lowPriorityTask(1, "Low", 3, 10);
    Task highPriorityTask(2, "High", 7, 10);

    // Low priority task rejected by policy
    int res1 = queue.enqueue(lowPriorityTask);
    assert(res1 == -1);
    assert(queue.size() == 0);
    assert(queue.rejectedCount() == 1);

    // High priority task accepted
    int res2 = queue.enqueue(highPriorityTask);
    assert(res2 == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1);

    // Deadline policy test
    TaskQueue deadlineQueue(5);
    DeadlinePolicy deadlinePolicy(50); // Horizon of 50
    deadlineQueue.usePolicy(&deadlinePolicy);

    Task onTimeTask(3, "OnTime", 1, 30);
    Task overdueTask(4, "Late", 1, 100);

    assert(deadlineQueue.enqueue(onTimeTask) == 1);
    assert(deadlineQueue.enqueue(overdueTask) == -1); // Rejected because deadline > 50
    assert(deadlineQueue.rejectedCount() == 1);

    std::cout << "Test test_enqueue_with_policy_acceptance: PASS" << std::endl;
}

void test_best_and_bestIndex_default_weight() {
    TaskQueue queue;
    assert(queue.bestIndex() == -1);
    Task defaultBest = queue.best();
    assert(defaultBest.identifier() == 0);

    // Without policy, bestIndex uses Task::weight() (priority * 10 [+ 25 if overdue])
    Task t1(1, "Low Priority", 2, 10);   // Weight = 20
    Task t2(2, "High Priority", 8, 10);  // Weight = 80
    Task t3(3, "Medium Priority", 5, 10); // Weight = 50

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);
    assert(queue.best().label() == "High Priority");

    std::cout << "Test test_best_and_bestIndex_default_weight: PASS" << std::endl;
}

void test_best_and_bestIndex_with_policies() {
    TaskQueue queue;
    Task t1(1, "T1", 5, 10);
    Task t2(2, "T2", 10, 20);
    Task t3(3, "T3", 2, 5);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // FIFO Policy: rank is bias - task.id * arrivalWeight
    // t1 rank: 0 - 1*1 = -1
    // t2 rank: 0 - 2*1 = -2
    // t3 rank: 0 - 3*1 = -3
    FifoPolicy fifo;
    queue.usePolicy(&fifo);
    assert(queue.bestIndex() == 0);
    assert(queue.best().identifier() == 1);

    // Priority Policy: rank is max(priority, floor) * 100 - duration + bias
    // t1: 5 * 100 - 10 = 490
    // t2: 10 * 100 - 20 = 980
    // t3: 2 * 100 - 5 = 195
    PriorityPolicy priorityPol;
    queue.usePolicy(&priorityPol);
    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);

    // Deadline Policy: rank is max(0, horizon - deadline) * 5 + bias
    // Default deadline equals duration.
    // t1 (deadline 10): (50 - 10) * 5 = 200
    // t2 (deadline 20): (50 - 20) * 5 = 150
    // t3 (deadline 5): (50 - 5) * 5 = 225
    DeadlinePolicy deadlinePol(50);
    queue.usePolicy(&deadlinePol);
    assert(queue.bestIndex() == 2);
    assert(queue.best().identifier() == 3);

    // Switch back to no policy (nullptr)
    queue.usePolicy(nullptr);
    assert(queue.bestIndex() == 1); // t2 has highest weight (100)

    std::cout << "Test test_best_and_bestIndex_with_policies: PASS" << std::endl;
}

void test_dequeue() {
    TaskQueue queue;
    // Empty dequeue returns default Task
    Task emptyTask = queue.dequeue();
    assert(emptyTask.identifier() == 0);
    assert(queue.isEmpty());

    Task t1(10, "Task 10", 2, 10);
    Task t2(20, "Task 20", 9, 10);
    Task t3(30, "Task 30", 5, 10);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Highest priority task (t2) should be dequeued first
    Task d1 = queue.dequeue();
    assert(d1.identifier() == 20);
    assert(queue.size() == 2);

    // Next highest is t3
    Task d2 = queue.dequeue();
    assert(d2.identifier() == 30);
    assert(queue.size() == 1);

    // Last is t1
    Task d3 = queue.dequeue();
    assert(d3.identifier() == 10);
    assert(queue.isEmpty());

    // Queue is now empty
    Task d4 = queue.dequeue();
    assert(d4.identifier() == 0);

    std::cout << "Test test_dequeue: PASS" << std::endl;
}

void test_totalRemaining_and_pendingCount() {
    TaskQueue queue;
    Task t1(1, "T1", 1, 10);
    Task t2(2, "T2", 1, 20);
    Task t3(3, "T3", 1, 30);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.totalRemaining() == 60);
    assert(queue.pendingCount() == 3);

    // Advance one task directly
    queue.tasks[0].advance(4); // Remaining 6, running
    assert(queue.totalRemaining() == 56);
    assert(queue.pendingCount() == 2); // t1 is no longer pending (it is running)

    // Complete t1
    queue.tasks[0].advance(6); // Remaining 0, complete
    assert(queue.totalRemaining() == 50);
    assert(queue.pendingCount() == 2); // t2, t3 are still pending

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue;
    Task t1(1, "Short", 1, 5);
    Task t2(2, "Medium", 1, 10);
    Task t3(3, "Long", 1, 20);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Advance all by 5 units -> t1 completes (duration 5)
    int completedCount = queue.advanceAll(5);
    assert(completedCount == 1);
    assert(queue.tasks[0].isComplete());
    assert(!queue.tasks[1].isComplete());
    assert(!queue.tasks[2].isComplete());
    assert(queue.totalRemaining() == 0 + 5 + 15);

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == 1);
    assert(queue.size() == 2);
    assert(queue.tasks[0].identifier() == 2);
    assert(queue.tasks[1].identifier() == 3);

    // Advance remaining by 20 units -> both complete
    completedCount = queue.advanceAll(20);
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
    test_constructors_and_initial_state();
    test_enqueue_and_capacity_limits();
    test_enqueue_with_policy_acceptance();
    test_best_and_bestIndex_default_weight();
    test_best_and_bestIndex_with_policies();
    test_dequeue();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();

    std::cout << "\nAll TaskQueue tests passed successfully!" << std::endl;
    return 0;
}