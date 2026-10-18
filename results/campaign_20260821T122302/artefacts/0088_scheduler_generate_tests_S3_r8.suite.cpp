#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "Policy.h"
#include "Task.h"
#include "TaskQueue.h"

void test_constructors_and_initial_state() {
    // Default constructor
    TaskQueue q1;
    assert(q1.size() == 0);
    assert(q1.isEmpty());
    assert(!q1.isFull());
    assert(q1.capacity == 8);
    assert(q1.rejectedCount() == 0);
    assert(q1.policy == nullptr);

    // Parameterized constructor with positive limit
    TaskQueue q2(5);
    assert(q2.size() == 0);
    assert(q2.isEmpty());
    assert(!q2.isFull());
    assert(q2.capacity == 5);
    assert(q2.rejectedCount() == 0);

    // Parameterized constructor with zero or negative limit (fallback to 1)
    TaskQueue q3(0);
    assert(q3.capacity == 1);
    TaskQueue q4(-10);
    assert(q4.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue queue(3);
    assert(queue.capacity == 3);

    Task t1(1, "Task1", 5, 10);
    Task t2(2, "Task2", 3, 20);
    Task t3(3, "Task3", 8, 15);
    Task t4(4, "Task4", 1, 5);

    // Enqueue 1
    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());
    assert(queue.rejectedCount() == 0);

    // Enqueue 2
    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);

    // Enqueue 3 (fills the queue)
    int res3 = queue.enqueue(t3);
    assert(res3 == 3);
    assert(queue.size() == 3);
    assert(queue.isFull());
    assert(queue.rejectedCount() == 0);

    // Enqueue 4 (exceeds capacity)
    int res4 = queue.enqueue(t4);
    assert(res4 == -1);
    assert(queue.size() == 3);
    assert(queue.rejectedCount() == 1);

    // Another overflow attempt
    int res5 = queue.enqueue(t4);
    assert(res5 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_enqueue_with_policy_acceptance() {
    TaskQueue queue(5);
    PriorityPolicy minPriorityPolicy(5); // Accepts tasks with priority >= 5
    queue.usePolicy(&minPriorityPolicy);

    Task lowPriorityTask(1, "Low", 2, 10);
    Task highPriorityTask(2, "High", 7, 10);

    // Rejected by policy
    int res1 = queue.enqueue(lowPriorityTask);
    assert(res1 == -1);
    assert(queue.size() == 0);
    assert(queue.rejectedCount() == 1);

    // Accepted by policy
    int res2 = queue.enqueue(highPriorityTask);
    assert(res2 == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1);

    // Test with DeadlinePolicy
    TaskQueue deadlineQueue(5);
    DeadlinePolicy deadlinePolicy(50); // Accepts deadline <= 50
    deadlineQueue.usePolicy(&deadlinePolicy);

    Task farTask(3, "Far", 5, 20);
    farTask.setDeadline(100);
    Task nearTask(4, "Near", 5, 20);
    nearTask.setDeadline(30);

    assert(deadlineQueue.enqueue(farTask) == -1);
    assert(deadlineQueue.rejectedCount() == 1);

    assert(deadlineQueue.enqueue(nearTask) == 1);
    assert(deadlineQueue.rejectedCount() == 1);
    assert(deadlineQueue.size() == 1);

    std::cout << "Test test_enqueue_with_policy_acceptance: PASS" << std::endl;
}

void test_best_and_bestIndex_without_policy() {
    TaskQueue queue;

    // Empty queue
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    assert(emptyBest.id == 0);
    assert(emptyBest.name == "unnamed");

    Task t1(1, "Task1", 3, 10); // weight = 30
    Task t2(2, "Task2", 8, 10); // weight = 80
    Task t3(3, "Task3", 5, 10); // weight = 50

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Highest weight should be t2 (index 1)
    assert(queue.bestIndex() == 1);
    assert(queue.best().id == 2);
    assert(queue.best().name == "Task2");

    // Overdue task gets +25 weight
    Task t4(4, "Task4", 6, 10); // weight = 60 normally, but if overdue: 60 + 25 = 85
    t4.setDeadline(2);
    t4.advance(3); // elapsed = 3 > deadline = 2 => overdue!
    assert(t4.isOverdue());
    assert(t4.weight() == 85);

    queue.enqueue(t4);
    assert(queue.bestIndex() == 3);
    assert(queue.best().id == 4);

    // Test tie-breaking (first encountered max score is kept)
    TaskQueue tieQueue;
    Task tie1(10, "Tie1", 5, 10); // weight = 50
    Task tie2(20, "Tie2", 5, 10); // weight = 50
    tieQueue.enqueue(tie1);
    tieQueue.enqueue(tie2);
    assert(tieQueue.bestIndex() == 0);
    assert(tieQueue.best().id == 10);

    std::cout << "Test test_best_and_bestIndex_without_policy: PASS" << std::endl;
}

void test_best_and_bestIndex_with_policies() {
    TaskQueue queue;
    Task t1(1, "Task1", 5, 20);
    Task t2(2, "Task2", 10, 10);
    Task t3(3, "Task3", 10, 30);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // 1. FifoPolicy: rank = bias - task.id * arrivalWeight
    FifoPolicy fifo(1);
    queue.usePolicy(&fifo);
    // t1 rank: 0 - 1*1 = -1
    // t2 rank: 0 - 2*1 = -2
    // t3 rank: 0 - 3*1 = -3
    assert(queue.bestIndex() == 0);
    assert(queue.best().id == 1);

    // 2. PriorityPolicy: rank = score * 100 - duration + bias
    PriorityPolicy priorityPolicy(0);
    queue.usePolicy(&priorityPolicy);
    // t1: 5 * 100 - 20 = 480
    // t2: 10 * 100 - 10 = 990
    // t3: 10 * 100 - 30 = 970
    assert(queue.bestIndex() == 1);
    assert(queue.best().id == 2);

    // 3. DeadlinePolicy: urgency = horizon - task.deadline; rank = urgency * 5 + bias
    DeadlinePolicy deadlinePolicy(100);
    // t1 default deadline = 20 -> urgency = 80 -> rank = 400
    // t2 default deadline = 10 -> urgency = 90 -> rank = 450
    // t3 default deadline = 30 -> urgency = 70 -> rank = 350
    queue.usePolicy(&deadlinePolicy);
    assert(queue.bestIndex() == 1);
    assert(queue.best().id == 2);

    std::cout << "Test test_best_and_bestIndex_with_policies: PASS" << std::endl;
}

void test_dequeue() {
    TaskQueue queue;

    // Dequeue on empty queue
    Task emptyResult = queue.dequeue();
    assert(emptyResult.id == 0);
    assert(emptyResult.name == "unnamed");
    assert(queue.size() == 0);

    Task t1(1, "Task1", 3, 10);
    Task t2(2, "Task2", 8, 10);
    Task t3(3, "Task3", 5, 10);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // First dequeue: t2 (priority 8)
    Task d1 = queue.dequeue();
    assert(d1.id == 2);
    assert(queue.size() == 2);

    // Second dequeue: t3 (priority 5)
    Task d2 = queue.dequeue();
    assert(d2.id == 3);
    assert(queue.size() == 1);

    // Third dequeue: t1 (priority 3)
    Task d3 = queue.dequeue();
    assert(d3.id == 1);
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    // Further dequeue on empty
    Task d4 = queue.dequeue();
    assert(d4.id == 0);

    std::cout << "Test test_dequeue: PASS" << std::endl;
}

void test_totalRemaining_and_pendingCount() {
    TaskQueue queue;
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);

    Task t1(1, "Task1", 5, 10);
    Task t2(2, "Task2", 5, 20);
    Task t3(3, "Task3", 5, 15);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.totalRemaining() == 45);
    assert(queue.pendingCount() == 3);

    // Advance individual tasks
    queue.tasks[0].advance(4); // remaining: 6, started: true
    assert(queue.tasks[0].remaining() == 6);
    assert(queue.tasks[0].isRunning());
    assert(!queue.tasks[0].isPending());

    assert(queue.totalRemaining() == 41);
    assert(queue.pendingCount() == 2);

    // Advance to complete
    queue.tasks[1].advance(20); // remaining: 0, finished: true
    assert(queue.tasks[1].isComplete());
    assert(!queue.tasks[1].isPending());

    assert(queue.totalRemaining() == 21);
    assert(queue.pendingCount() == 1);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue;
    Task t1(1, "Task1", 5, 5);
    Task t2(2, "Task2", 5, 10);
    Task t3(3, "Task3", 5, 15);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Advance all by 4 units -> none finished yet
    int completedCount1 = queue.advanceAll(4);
    assert(completedCount1 == 0);
    assert(queue.tasks[0].remaining() == 1);
    assert(queue.tasks[1].remaining() == 6);
    assert(queue.tasks[2].remaining() == 11);

    // Purge when none complete
    int purgedCount1 = queue.purgeComplete();
    assert(purgedCount1 == 0);
    assert(queue.size() == 3);

    // Advance all by 3 more units -> t1 finishes (elapsed 7 >= 5), t2 has elapsed 7 < 10, t3 has elapsed 7 < 15
    int completedCount2 = queue.advanceAll(3);
    assert(completedCount2 == 1);
    assert(queue.tasks[0].isComplete());
    assert(!queue.tasks[1].isComplete());
    assert(!queue.tasks[2].isComplete());

    // Purge t1
    int purgedCount2 = queue.purgeComplete();
    assert(purgedCount2 == 1);
    assert(queue.size() == 2);
    assert(queue.tasks[0].id == 2);
    assert(queue.tasks[1].id == 3);

    // Advance all remaining tasks to finish (by 20 units)
    int completedCount3 = queue.advanceAll(20);
    assert(completedCount3 == 2);
    assert(queue.tasks[0].isComplete());
    assert(queue.tasks[1].isComplete());

    // Purge all remaining
    int purgedCount3 = queue.purgeComplete();
    assert(purgedCount3 == 2);
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

void test_usePolicy_method() {
    TaskQueue queue;
    Task t1(1, "T1", 10, 5);
    Task t2(2, "T2", 20, 5);
    queue.enqueue(t1);
    queue.enqueue(t2);

    FifoPolicy fifo;
    int returnedSize = queue.usePolicy(&fifo);
    assert(returnedSize == 2);
    assert(queue.policy == &fifo);

    PriorityPolicy prio;
    returnedSize = queue.usePolicy(&prio);
    assert(returnedSize == 2);
    assert(queue.policy == &prio);

    returnedSize = queue.usePolicy(nullptr);
    assert(returnedSize == 2);
    assert(queue.policy == nullptr);

    std::cout << "Test test_usePolicy_method: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_enqueue_and_capacity_limits();
    test_enqueue_with_policy_acceptance();
    test_best_and_bestIndex_without_policy();
    test_best_and_bestIndex_with_policies();
    test_dequeue();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();
    test_usePolicy_method();

    std::cout << "\nAll TaskQueue tests passed successfully!" << std::endl;
    return 0;
}