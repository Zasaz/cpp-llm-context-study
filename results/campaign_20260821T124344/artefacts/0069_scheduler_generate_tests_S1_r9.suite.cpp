#include <iostream>
#include <cassert>
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
    assert(q1.policy == 0);
    assert(q1.rejectedCount() == 0);
    assert(q1.totalRemaining() == 0);
    assert(q1.pendingCount() == 0);
    assert(q1.bestIndex() == -1);

    // Custom limit constructor
    TaskQueue q2(3);
    assert(q2.capacity == 3);
    assert(q2.isEmpty() == true);

    // Custom limit <= 0 should default to capacity 1
    TaskQueue q3(0);
    assert(q3.capacity == 1);

    TaskQueue q4(-5);
    assert(q4.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue q(2);
    Task t1(1, "Task1", 5, 10);
    Task t2(2, "Task2", 3, 20);
    Task t3(3, "Task3", 1, 30);

    int res1 = q.enqueue(t1);
    assert(res1 == 1);
    assert(q.size() == 1);
    assert(!q.isEmpty());
    assert(!q.isFull());

    int res2 = q.enqueue(t2);
    assert(res2 == 2);
    assert(q.size() == 2);
    assert(q.isFull());

    // Exceeding capacity
    int res3 = q.enqueue(t3);
    assert(res3 == -1);
    assert(q.size() == 2);
    assert(q.rejectedCount() == 1);

    // Reject count should accumulate
    q.enqueue(t3);
    assert(q.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_policy_interaction_and_rejection() {
    TaskQueue q(5);
    PriorityPolicy priorityPolicy(10); // rejects tasks with priority < 10

    q.usePolicy(&priorityPolicy);
    assert(q.policy == &priorityPolicy);

    Task lowPriorityTask(1, "Low", 5, 10);
    Task highPriorityTask(2, "High", 12, 10);

    int res1 = q.enqueue(lowPriorityTask);
    assert(res1 == -1);
    assert(q.size() == 0);
    assert(q.rejectedCount() == 1);

    int res2 = q.enqueue(highPriorityTask);
    assert(res2 == 1);
    assert(q.size() == 1);
    assert(q.rejectedCount() == 1);

    // Switch policy to DeadlinePolicy
    DeadlinePolicy deadlinePolicy(15);
    q.usePolicy(&deadlinePolicy);

    Task taskWithinDeadline(3, "Timely", 1, 10);
    taskWithinDeadline.setDeadline(15);
    Task taskPastDeadline(4, "Late", 1, 10);
    taskPastDeadline.setDeadline(20);

    assert(q.enqueue(taskWithinDeadline) == 2);
    assert(q.enqueue(taskPastDeadline) == -1);
    assert(q.rejectedCount() == 2);

    std::cout << "Test test_policy_interaction_and_rejection: PASS" << std::endl;
}

void test_default_ranking_and_best() {
    TaskQueue q(5);
    // When policy is 0, rank uses task.weight() = priority * 10 (+ 25 if overdue)
    Task t1(1, "Task1", 3, 10); // weight = 30
    Task t2(2, "Task2", 5, 10); // weight = 50
    Task t3(3, "Task3", 2, 10); // weight = 20

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    assert(q.bestIndex() == 1);
    Task bestTask = q.best();
    assert(bestTask.identifier() == 2);
    assert(bestTask.label() == "Task2");

    // Empty queue best behavior
    TaskQueue emptyQ;
    assert(emptyQ.bestIndex() == -1);
    Task emptyBest = emptyQ.best();
    assert(emptyBest.identifier() == 0);

    std::cout << "Test test_default_ranking_and_best: PASS" << std::endl;
}

void test_custom_policy_ranking() {
    TaskQueue q(5);
    FifoPolicy fifo; // rank = bias - task.id * arrivalWeight (earlier/lower id has higher rank)
    q.usePolicy(&fifo);

    Task t1(10, "Task10", 5, 10);
    Task t2(5, "Task5", 1, 10);
    Task t3(20, "Task20", 10, 10);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    // t2 (id=5) has rank = -5, t1 has -10, t3 has -20. Best should be t2 (index 1).
    assert(q.bestIndex() == 1);
    assert(q.best().identifier() == 5);

    std::cout << "Test test_custom_policy_ranking: PASS" << std::endl;
}

void test_dequeue_behavior() {
    TaskQueue q(5);
    Task t1(1, "Task1", 10, 10);
    Task t2(2, "Task2", 20, 10);
    Task t3(3, "Task3", 15, 10);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    // Should dequeue highest weight first (t2)
    Task d1 = q.dequeue();
    assert(d1.identifier() == 2);
    assert(q.size() == 2);

    // Next highest is t3
    Task d2 = q.dequeue();
    assert(d2.identifier() == 3);
    assert(q.size() == 1);

    // Next is t1
    Task d3 = q.dequeue();
    assert(d3.identifier() == 1);
    assert(q.isEmpty());

    // Dequeue from empty queue
    Task dEmpty = q.dequeue();
    assert(dEmpty.identifier() == 0);
    assert(q.isEmpty());

    std::cout << "Test test_dequeue_behavior: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue q(5);
    Task t1(1, "T1", 1, 10);
    Task t2(2, "T2", 1, 5);
    Task t3(3, "T3", 1, 15);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    assert(q.pendingCount() == 3);
    assert(q.totalRemaining() == 30); // 10 + 5 + 15

    // Advance 5 units
    int completedCount = q.advanceAll(5);
    assert(completedCount == 1); // T2 should be complete
    assert(q.pendingCount() == 0); // All started
    assert(q.totalRemaining() == 15); // (10-5) + 0 + (15-5) = 5 + 0 + 10 = 15

    // Purge completed tasks
    int purged = q.purgeComplete();
    assert(purged == 1);
    assert(q.size() == 2);
    assert(q.tasks[0].identifier() == 1);
    assert(q.tasks[1].identifier() == 3);

    // Advance another 10 units
    completedCount = q.advanceAll(10);
    assert(completedCount == 2); // Both T1 (5 left) and T3 (10 left) complete
    assert(q.totalRemaining() == 0);

    purged = q.purgeComplete();
    assert(purged == 2);
    assert(q.isEmpty());

    // Purge on empty queue
    assert(q.purgeComplete() == 0);

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_enqueue_and_capacity_limits();
    test_policy_interaction_and_rejection();
    test_default_ranking_and_best();
    test_custom_policy_ranking();
    test_dequeue_behavior();
    test_advanceAll_and_purgeComplete();

    std::cout << "All TaskQueue unit tests passed successfully." << std::endl;
    return 0;
}