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

    // Custom constructor with positive capacity
    TaskQueue customQueue(4);
    assert(customQueue.capacity == 4);
    assert(customQueue.isEmpty());
    assert(!customQueue.isFull());

    // Custom constructor with zero or negative capacity (should fallback to 1)
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negativeQueue(-5);
    assert(negativeQueue.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue queue(2);

    Task t1(1, "Task 1", 5, 10);
    Task t2(2, "Task 2", 3, 20);
    Task t3(3, "Task 3", 8, 15);

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

    // Enqueue when full -> rejected
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    // Attempt another enqueue to verify rejection count increments
    Task t4(4, "Task 4", 1, 5);
    int res4 = queue.enqueue(t4);
    assert(res4 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_policy_filtering_on_enqueue() {
    TaskQueue queue(5);

    // PriorityPolicy with minimum priority floor of 5
    PriorityPolicy pPolicy(5);
    int retSize = queue.usePolicy(&pPolicy);
    assert(retSize == 0);
    assert(queue.policy == &pPolicy);

    Task validTask(1, "HighPriority", 7, 10);
    Task invalidTask(2, "LowPriority", 3, 10);

    assert(queue.enqueue(validTask) == 1);
    assert(queue.rejectedCount() == 0);

    // Should be rejected by PriorityPolicy (priority 3 < 5)
    assert(queue.enqueue(invalidTask) == -1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1);

    // DeadlinePolicy with horizon 50
    DeadlinePolicy dPolicy(50);
    queue.usePolicy(&dPolicy);

    Task taskWithinDeadline(3, "Quick", 1, 30); // deadline defaults to duration = 30 <= 50
    Task taskPastDeadline(4, "Late", 1, 60);     // deadline defaults to duration = 60 > 50

    assert(queue.enqueue(taskWithinDeadline) == 2);
    assert(queue.enqueue(taskPastDeadline) == -1);
    assert(queue.rejectedCount() == 2);

    // Default policy rejection when task is complete
    Policy defaultPolicy;
    queue.usePolicy(&defaultPolicy);

    Task finishedTask(5, "Done", 1, 10);
    finishedTask.advance(10); // completes task
    assert(finishedTask.isComplete());

    assert(queue.enqueue(finishedTask) == -1);
    assert(queue.rejectedCount() == 3);

    std::cout << "Test test_policy_filtering_on_enqueue: PASS" << std::endl;
}

void test_best_and_bestIndex_without_policy() {
    TaskQueue queue(10);

    // When empty
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    assert(emptyBest.id == 0);
    assert(emptyBest.name == "unnamed");

    // Task weight = priority * 10 (+ 25 if overdue)
    Task t1(1, "Low", 2, 10);      // weight: 20
    Task t2(2, "High", 8, 10);     // weight: 80
    Task t3(3, "Medium", 5, 10);   // weight: 50

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);
    assert(queue.best().label() == "High");

    // Overdue task edge case:
    // t1 priority 2 -> base 20. If overdue (elapsed > deadline) -> 20 + 25 = 45
    // Let's create an overdue task with priority 6 -> 60 + 25 = 85 (beats t2's 80)
    Task t4(4, "OverdueHero", 6, 10);
    t4.setDeadline(2);
    t4.advance(5); // elapsed = 5 > deadline (2) -> isOverdue() is true
    assert(t4.isOverdue());
    assert(t4.weight() == 60 + 25);

    queue.enqueue(t4);
    assert(queue.bestIndex() == 3);
    assert(queue.best().identifier() == 4);

    std::cout << "Test test_best_and_bestIndex_without_policy: PASS" << std::endl;
}

void test_best_with_different_policies() {
    TaskQueue queue(10);
    Task t1(1, "First", 5, 20);
    t1.setDeadline(50);
    Task t2(2, "Second", 10, 10);
    t2.setDeadline(10);
    Task t3(3, "Third", 2, 5);
    t3.setDeadline(80);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // 1. FIFO Policy: rank = bias - task.id * arrivalWeight
    FifoPolicy fifo(2);
    queue.usePolicy(&fifo);
    // t1: 0 - 1*2 = -2
    // t2: 0 - 2*2 = -4
    // t3: 0 - 3*2 = -6
    assert(queue.bestIndex() == 0);
    assert(queue.best().identifier() == 1);

    // 2. Priority Policy: rank = max(priority, floor) * 100 - duration + bias
    PriorityPolicy priorityPol(0);
    queue.usePolicy(&priorityPol);
    // t1: 5 * 100 - 20 = 480
    // t2: 10 * 100 - 10 = 990
    // t3: 2 * 100 - 5 = 195
    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);

    // 3. Deadline Policy: rank = max(0, horizon - deadline) * 5 + bias
    DeadlinePolicy deadlinePol(100);
    queue.usePolicy(&deadlinePol);
    // t1: (100 - 50) * 5 = 250
    // t2: (100 - 10) * 5 = 450
    // t3: (100 - 80) * 5 = 100
    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);

    // Adjust bias on policy
    deadlinePol.adjust(50);
    assert(deadlinePol.bias == 50);
    assert(queue.bestIndex() == 1); // Relative ordering should still hold

    std::cout << "Test test_best_with_different_policies: PASS" << std::endl;
}

void test_dequeue_operations() {
    TaskQueue queue(5);

    // Dequeue from empty queue
    Task emptyResult = queue.dequeue();
    assert(emptyResult.identifier() == 0);
    assert(queue.isEmpty());

    Task t1(1, "Task1", 10, 10);
    Task t2(2, "Task2", 50, 10);
    Task t3(3, "Task3", 30, 10);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);
    assert(queue.size() == 3);

    // Without policy, best is t2 (priority 50 -> weight 500)
    Task dequeued1 = queue.dequeue();
    assert(dequeued1.identifier() == 2);
    assert(queue.size() == 2);

    // Next best is t3 (priority 30 -> weight 300)
    Task dequeued2 = queue.dequeue();
    assert(dequeued2.identifier() == 3);
    assert(queue.size() == 1);

    // Next best is t1 (priority 10 -> weight 100)
    Task dequeued3 = queue.dequeue();
    assert(dequeued3.identifier() == 1);
    assert(queue.isEmpty());
    assert(queue.size() == 0);

    // Dequeue again should be empty task
    Task dequeued4 = queue.dequeue();
    assert(dequeued4.identifier() == 0);

    std::cout << "Test test_dequeue_operations: PASS" << std::endl;
}

void test_totalRemaining_and_pendingCount() {
    TaskQueue queue(5);

    Task t1(1, "T1", 5, 20);
    Task t2(2, "T2", 5, 15);
    Task t3(3, "T3", 5, 10);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.totalRemaining() == 45);
    assert(queue.pendingCount() == 3);

    // Partially advance one task
    queue.tasks[0].advance(5); // remaining becomes 15, started is true -> no longer pending
    assert(queue.tasks[0].isRunning());
    assert(!queue.tasks[0].isPending());
    assert(queue.totalRemaining() == 40);
    assert(queue.pendingCount() == 2);

    // Complete another task
    queue.tasks[1].advance(15); // remaining becomes 0, finished is true
    assert(queue.tasks[1].isComplete());
    assert(!queue.tasks[1].isPending());
    assert(queue.totalRemaining() == 25);
    assert(queue.pendingCount() == 1);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue(5);

    Task t1(1, "Short", 1, 5);
    Task t2(2, "Medium", 1, 10);
    Task t3(3, "Long", 1, 20);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.pendingCount() == 3);

    // Advance all by 5 units:
    // t1 duration 5 -> elapsed 5 (complete)
    // t2 duration 10 -> elapsed 5 (remaining 5)
    // t3 duration 20 -> elapsed 5 (remaining 15)
    int completedCount = queue.advanceAll(5);
    assert(completedCount == 1);
    assert(queue.tasks[0].isComplete());
    assert(!queue.tasks[1].isComplete());
    assert(!queue.tasks[2].isComplete());
    assert(queue.pendingCount() == 0); // All have started now

    // Advance all by another 5 units:
    // t1 already complete -> stays complete
    // t2 elapsed 10 -> complete
    // t3 elapsed 10 -> remaining 10
    completedCount = queue.advanceAll(5);
    assert(completedCount == 2); // t1 and t2 are now complete
    assert(queue.tasks[0].isComplete());
    assert(queue.tasks[1].isComplete());
    assert(!queue.tasks[2].isComplete());

    // Advance by 0 or negative units
    completedCount = queue.advanceAll(0);
    assert(completedCount == 2); // 2 remain complete
    completedCount = queue.advanceAll(-5);
    assert(completedCount == 2);

    // Purge completed tasks
    int purgedCount = queue.purgeComplete();
    assert(purgedCount == 2);
    assert(queue.size() == 1);
    assert(queue.tasks[0].identifier() == 3);
    assert(queue.tasks[0].remaining() == 10);

    // Purging again when no completed tasks exist
    assert(queue.purgeComplete() == 0);
    assert(queue.size() == 1);

    // Advance remaining task to finish and purge
    queue.advanceAll(10);
    assert(queue.tasks[0].isComplete());
    assert(queue.purgeComplete() == 1);
    assert(queue.isEmpty());

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

void test_tie_breaking_and_edge_cases() {
    TaskQueue queue(5);

    // In bestIndex(), when scores are equal, the first encountered is kept
    Task t1(1, "FirstTie", 5, 10);
    Task t2(2, "SecondTie", 5, 10);

    queue.enqueue(t1);
    queue.enqueue(t2);

    assert(queue.bestIndex() == 0);
    assert(queue.best().identifier() == 1);

    // Restarting a task
    queue.tasks[0].advance(5);
    assert(queue.tasks[0].isRunning());
    int prevElapsed = queue.tasks[0].restart();
    assert(prevElapsed == 5);
    assert(queue.tasks[0].isPending());
    assert(queue.tasks[0].remaining() == 10);

    // Switching policy dynamically
    FifoPolicy fifoPolicy;
    queue.usePolicy(&fifoPolicy);
    assert(queue.policy == &fifoPolicy);
    queue.usePolicy(nullptr);
    assert(queue.policy == nullptr);

    std::cout << "Test test_tie_breaking_and_edge_cases: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_enqueue_and_capacity_limits();
    test_policy_filtering_on_enqueue();
    test_best_and_bestIndex_without_policy();
    test_best_with_different_policies();
    test_dequeue_operations();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();
    test_tie_breaking_and_edge_cases();

    std::cout << "\nAll TaskQueue tests passed successfully!" << std::endl;
    return 0;
}