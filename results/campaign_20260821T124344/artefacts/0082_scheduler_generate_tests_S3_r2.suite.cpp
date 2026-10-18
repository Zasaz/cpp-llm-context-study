#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include "TaskQueue.h"
#include "Policy.h"
#include "Task.h"

void test_constructors_and_initial_state() {
    // Default constructor
    TaskQueue q1;
    assert(q1.size() == 0);
    assert(q1.isEmpty() == true);
    assert(q1.isFull() == false);
    assert(q1.capacity == 8);
    assert(q1.rejectedCount() == 0);
    assert(q1.policy == nullptr);

    // Parameterized constructor with positive limit
    TaskQueue q2(5);
    assert(q2.capacity == 5);
    assert(q2.size() == 0);
    assert(q2.isEmpty() == true);
    assert(q2.isFull() == false);

    // Parameterized constructor with zero or negative limit
    TaskQueue q3(0);
    assert(q3.capacity == 1);
    TaskQueue q4(-10);
    assert(q4.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity() {
    TaskQueue q(2);
    Task t1(1, "Task1", 5, 10);
    Task t2(2, "Task2", 3, 20);
    Task t3(3, "Task3", 8, 15);

    int res1 = q.enqueue(t1);
    assert(res1 == 1);
    assert(q.size() == 1);
    assert(!q.isEmpty());
    assert(!q.isFull());

    int res2 = q.enqueue(t2);
    assert(res2 == 2);
    assert(q.size() == 2);
    assert(q.isFull());

    // Enqueue when full should fail and increment rejected count
    int res3 = q.enqueue(t3);
    assert(res3 == -1);
    assert(q.size() == 2);
    assert(q.rejectedCount() == 1);

    std::cout << "Test test_enqueue_and_capacity: PASS" << std::endl;
}

void test_policy_rejection() {
    TaskQueue q(5);
    PriorityPolicy minPriorityPolicy(5);
    q.usePolicy(&minPriorityPolicy);

    Task tValid(1, "HighPriority", 6, 10);
    Task tInvalid(2, "LowPriority", 4, 10);

    int res1 = q.enqueue(tValid);
    assert(res1 == 1);
    assert(q.size() == 1);
    assert(q.rejectedCount() == 0);

    // PriorityPolicy requires priority >= floorPriority (5)
    int res2 = q.enqueue(tInvalid);
    assert(res2 == -1);
    assert(q.size() == 1);
    assert(q.rejectedCount() == 1);

    // Deadline policy rejection test
    TaskQueue qDeadline(5);
    DeadlinePolicy deadlinePolicy(50);
    qDeadline.usePolicy(&deadlinePolicy);

    Task tFeasible(3, "Feasible", 1, 30);
    Task tTooLong(4, "TooLong", 1, 60);

    assert(qDeadline.enqueue(tFeasible) == 1);
    assert(qDeadline.enqueue(tTooLong) == -1);
    assert(qDeadline.rejectedCount() == 1);

    std::cout << "Test test_policy_rejection: PASS" << std::endl;
}

void test_best_and_dequeue_without_policy() {
    TaskQueue q(5);
    // When no policy is set, default ranking is task.weight() = priority * 10 (+ 25 if overdue)
    Task t1(1, "Low", 1, 10);     // weight: 10
    Task t2(2, "High", 10, 10);   // weight: 100
    Task t3(3, "Medium", 5, 10);  // weight: 50

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    assert(q.bestIndex() == 1);
    assert(q.best().identifier() == 2);
    assert(q.best().label() == "High");

    // Dequeue should return best item and remove it
    Task popped = q.dequeue();
    assert(popped.identifier() == 2);
    assert(q.size() == 2);

    // Now medium should be best
    assert(q.bestIndex() == 1); // index 1 in the remaining [t1, t3]
    assert(q.best().identifier() == 3);

    Task popped2 = q.dequeue();
    assert(popped2.identifier() == 3);
    assert(q.size() == 1);

    Task popped3 = q.dequeue();
    assert(popped3.identifier() == 1);
    assert(q.isEmpty());

    // Dequeue and best on empty queue
    assert(q.bestIndex() == -1);
    Task emptyBest = q.best();
    assert(emptyBest.identifier() == 0);
    Task emptyPopped = q.dequeue();
    assert(emptyPopped.identifier() == 0);

    std::cout << "Test test_best_and_dequeue_without_policy: PASS" << std::endl;
}

void test_best_and_dequeue_with_policy() {
    TaskQueue q(5);
    FifoPolicy fifoPolicy(1); // rank = bias - task.id * 1 (lower id yields higher rank)
    q.usePolicy(&fifoPolicy);

    Task t1(10, "FirstIn", 1, 5);
    Task t2(5, "SecondInHigherIdOrder", 10, 5);
    Task t3(20, "ThirdIn", 5, 5);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    // t2 has id 5, rank = -5 (higher than -10 and -20)
    assert(q.bestIndex() == 1);
    assert(q.best().identifier() == 5);

    Task dequeued = q.dequeue();
    assert(dequeued.identifier() == 5);
    assert(q.size() == 2);

    // Next best is t1 (id 10, rank -10 vs t3 id 20, rank -20)
    assert(q.bestIndex() == 0);
    assert(q.dequeue().identifier() == 10);

    std::cout << "Test test_best_and_dequeue_with_policy: PASS" << std::endl;
}

void test_use_policy_switch() {
    TaskQueue q(5);
    Task t1(1, "Task1", 10, 50); // High priority, long duration
    Task t2(2, "Task2", 2, 5);   // Low priority, short duration
    q.enqueue(t1);
    q.enqueue(t2);

    // Without policy: default weight (t1=100, t2=20) -> t1 chosen
    assert(q.best().identifier() == 1);

    // Switch to FifoPolicy: rank = -id. t1 rank = -1, t2 rank = -2 -> t1 chosen
    FifoPolicy fifo;
    q.usePolicy(&fifo);
    assert(q.best().identifier() == 1);

    // Switch to PriorityPolicy: rank = max(p, floor)*100 - duration
    // t1: 10*100 - 50 = 950
    // t2: 2*100 - 5 = 195
    PriorityPolicy prio(0);
    q.usePolicy(&prio);
    assert(q.best().identifier() == 1);

    std::cout << "Test test_use_policy_switch: PASS" << std::endl;
}

void test_counts_and_remaining() {
    TaskQueue q(5);
    Task t1(1, "T1", 1, 10);
    Task t2(2, "T2", 1, 20);
    Task t3(3, "T3", 1, 30);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    assert(q.pendingCount() == 3);
    assert(q.totalRemaining() == 60);

    // Start advancing tasks via queue
    int completed = q.advanceAll(10);
    // t1 has duration 10, now completed (remaining: 0)
    // t2 has duration 20, elapsed 10 (remaining: 10)
    // t3 has duration 30, elapsed 10 (remaining: 20)
    assert(completed == 1);
    assert(q.totalRemaining() == 30);
    assert(q.pendingCount() == 0); // None are pending anymore (t1 is complete, t2 & t3 are running)

    // Advance further to finish all
    int completedMore = q.advanceAll(25);
    assert(completedMore == 3); // All 3 are now complete
    assert(q.totalRemaining() == 0);

    std::cout << "Test test_counts_and_remaining: PASS" << std::endl;
}

void test_purge_complete() {
    TaskQueue q(5);
    Task t1(1, "T1", 1, 10);
    Task t2(2, "T2", 1, 20);
    Task t3(3, "T3", 1, 30);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    // Advance 15 units: t1 completes (duration 10), t2 & t3 still have remaining
    q.advanceAll(15);
    assert(q.tasks[0].isComplete());
    assert(!q.tasks[1].isComplete());
    assert(!q.tasks[2].isComplete());

    int purged = q.purgeComplete();
    assert(purged == 1);
    assert(q.size() == 2);
    assert(q.tasks[0].identifier() == 2);
    assert(q.tasks[1].identifier() == 3);

    // Purging again when no completed tasks exist
    int purgedEmpty = q.purgeComplete();
    assert(purgedEmpty == 0);
    assert(q.size() == 2);

    // Finish all remaining and purge
    q.advanceAll(50);
    assert(q.purgeComplete() == 2);
    assert(q.size() == 0);
    assert(q.isEmpty());

    std::cout << "Test test_purge_complete: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_enqueue_and_capacity();
    test_policy_rejection();
    test_best_and_dequeue_without_policy();
    test_best_and_dequeue_with_policy();
    test_use_policy_switch();
    test_counts_and_remaining();
    test_purge_complete();

    std::cout << "\nAll TaskQueue tests passed successfully!" << std::endl;
    return 0;
}