#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#include "TaskQueue.h"
#include "Policy.h"
#include "Task.h"

// Test constructors and initial state
void test_constructors_and_initial_state() {
    // Default constructor
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty() == true);
    assert(defaultQueue.isFull() == false);
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.policy == nullptr);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.bestIndex() == -1);

    // Parametrized constructor with positive capacity
    TaskQueue customQueue(3);
    assert(customQueue.capacity == 3);
    assert(customQueue.size() == 0);
    assert(customQueue.isEmpty() == true);

    // Parametrized constructor with non-positive capacity (should default to 1)
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negativeQueue(-5);
    assert(negativeQueue.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

// Test enqueue and capacity limits
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

    // Try enqueuing again when full
    int res4 = q.enqueue(t3);
    assert(res4 == -1);
    assert(q.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity: PASS" << std::endl;
}

// Test enqueue with policy acceptance/rejection
void test_enqueue_with_policy() {
    TaskQueue q(5);
    PriorityPolicy minPriorityPolicy(5); // Accepts tasks with priority >= 5

    q.usePolicy(&minPriorityPolicy);

    Task validTask(1, "Valid", 10, 10);
    Task invalidTask(2, "Invalid", 3, 10);

    int res1 = q.enqueue(validTask);
    assert(res1 == 1);
    assert(q.size() == 1);
    assert(q.rejectedCount() == 0);

    int res2 = q.enqueue(invalidTask);
    assert(res2 == -1);
    assert(q.size() == 1);
    assert(q.rejectedCount() == 1);

    // Policy rejecting completed task (default Policy base class)
    Policy basePolicy;
    TaskQueue q2(5);
    q2.usePolicy(&basePolicy);

    Task finishedTask(3, "Finished", 5, 10);
    finishedTask.advance(10);
    assert(finishedTask.isComplete());

    int res3 = q2.enqueue(finishedTask);
    assert(res3 == -1);
    assert(q2.rejectedCount() == 1);

    std::cout << "Test test_enqueue_with_policy: PASS" << std::endl;
}

// Test usePolicy switching
void test_use_policy() {
    TaskQueue q(5);
    assert(q.policy == nullptr);

    FifoPolicy fifoPolicy;
    int size = q.usePolicy(&fifoPolicy);
    assert(size == 0);
    assert(q.policy == &fifoPolicy);

    q.enqueue(Task(1, "T1", 2, 5));
    size = q.usePolicy(nullptr);
    assert(size == 1);
    assert(q.policy == nullptr);

    std::cout << "Test test_use_policy: PASS" << std::endl;
}

// Test bestIndex, best, and dequeue without custom policy (uses Task::weight)
void test_best_and_dequeue_default_weight() {
    TaskQueue q(5);

    // Empty queue checks
    assert(q.bestIndex() == -1);
    Task emptyBest = q.best();
    assert(emptyBest.id == 0);
    Task emptyDequeued = q.dequeue();
    assert(emptyDequeued.id == 0);

    // Tasks: weight = priority * 10 (+ 25 if overdue)
    Task t1(1, "Low", 2, 10);    // weight = 20
    Task t2(2, "High", 8, 10);   // weight = 80
    Task t3(3, "Medium", 5, 10); // weight = 50

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    assert(q.bestIndex() == 1);
    assert(q.best().id == 2);
    assert(q.best().name == "High");

    // Dequeue should remove highest weight task (t2)
    Task popped = q.dequeue();
    assert(popped.id == 2);
    assert(q.size() == 2);

    // Next best should be t3 (weight 50)
    assert(q.bestIndex() == 1); // in current vector: [0]=t1, [1]=t3
    assert(q.best().id == 3);

    popped = q.dequeue();
    assert(popped.id == 3);
    assert(q.size() == 1);

    popped = q.dequeue();
    assert(popped.id == 1);
    assert(q.isEmpty());

    std::cout << "Test test_best_and_dequeue_default_weight: PASS" << std::endl;
}

// Test ranking with different policies (FifoPolicy, PriorityPolicy, DeadlinePolicy)
void test_best_with_policies() {
    // FifoPolicy: rank = bias - task.id * arrivalWeight
    {
        TaskQueue q(5);
        FifoPolicy fifo(2); // Higher id -> lower rank score
        q.usePolicy(&fifo);

        q.enqueue(Task(10, "First", 5, 10));  // rank: 0 - 20 = -20
        q.enqueue(Task(5, "Second", 5, 10));  // rank: 0 - 10 = -10 (Highest score)
        q.enqueue(Task(20, "Third", 5, 10));  // rank: 0 - 40 = -40

        assert(q.bestIndex() == 1);
        assert(q.best().id == 5);
    }

    // PriorityPolicy: rank = score * 100 - task.duration + bias
    {
        TaskQueue q(5);
        PriorityPolicy priorityPolicy(1);
        q.usePolicy(&priorityPolicy);

        // Same priority, shorter duration ranks higher
        q.enqueue(Task(1, "Long", 5, 50));  // rank: 500 - 50 = 450
        q.enqueue(Task(2, "Short", 5, 10)); // rank: 500 - 10 = 490
        q.enqueue(Task(3, "Lower", 4, 5));  // rank: 400 - 5 = 395

        assert(q.bestIndex() == 1);
        assert(q.best().id == 2);
    }

    // DeadlinePolicy: rank = (horizon - deadline) * 5 + bias
    {
        TaskQueue q(5);
        DeadlinePolicy deadlinePolicy(100);
        q.usePolicy(&deadlinePolicy);

        Task t1(1, "FarDeadline", 5, 10);
        t1.setDeadline(80); // urgency: 100 - 80 = 20 -> rank: 100

        Task t2(2, "UrgentDeadline", 5, 10);
        t2.setDeadline(20); // urgency: 100 - 20 = 80 -> rank: 400

        q.enqueue(t1);
        q.enqueue(t2);

        assert(q.bestIndex() == 1);
        assert(q.best().id == 2);
    }

    std::cout << "Test test_best_with_policies: PASS" << std::endl;
}

// Test totalRemaining and pendingCount
void test_metrics_remaining_and_pending() {
    TaskQueue q(5);

    Task t1(1, "T1", 1, 10);
    Task t2(2, "T2", 1, 15);
    Task t3(3, "T3", 1, 5);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    assert(q.totalRemaining() == 10 + 15 + 5);
    assert(q.pendingCount() == 3);

    // Advance first task partially
    q.tasks[0].advance(4); // remaining: 6, isPending: false, isRunning: true
    assert(q.totalRemaining() == 6 + 15 + 5);
    assert(q.pendingCount() == 2);

    // Advance third task to completion
    q.tasks[2].advance(5); // remaining: 0, isPending: false, isComplete: true
    assert(q.totalRemaining() == 6 + 15 + 0);
    assert(q.pendingCount() == 1);

    std::cout << "Test test_metrics_remaining_and_pending: PASS" << std::endl;
}

// Test advanceAll and purgeComplete
void test_advance_all_and_purge() {
    TaskQueue q(5);

    Task t1(1, "T1", 1, 10);
    Task t2(2, "T2", 1, 20);
    Task t3(3, "T3", 1, 5);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    // Advance by 6 units:
    // t1 (10): elapsed=6, complete=false
    // t2 (20): elapsed=6, complete=false
    // t3 (5):  elapsed=5, complete=true
    int completedCount = q.advanceAll(6);
    assert(completedCount == 1);
    assert(q.tasks[0].elapsed == 6);
    assert(q.tasks[1].elapsed == 6);
    assert(q.tasks[2].elapsed == 5);
    assert(q.tasks[2].isComplete());

    // Purge completed tasks
    int purgedCount = q.purgeComplete();
    assert(purgedCount == 1);
    assert(q.size() == 2);
    assert(q.tasks[0].id == 1);
    assert(q.tasks[1].id == 2);

    // Purge when no task is complete
    int purgedAgain = q.purgeComplete();
    assert(purgedAgain == 0);
    assert(q.size() == 2);

    // Advance remaining tasks to completion
    completedCount = q.advanceAll(20);
    assert(completedCount == 2);
    assert(q.tasks[0].isComplete());
    assert(q.tasks[1].isComplete());

    purgedCount = q.purgeComplete();
    assert(purgedCount == 2);
    assert(q.isEmpty());

    std::cout << "Test test_advance_all_and_purge: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_enqueue_and_capacity();
    test_enqueue_with_policy();
    test_use_policy();
    test_best_and_dequeue_default_weight();
    test_best_with_policies();
    test_metrics_remaining_and_pending();
    test_advance_all_and_purge();

    std::cout << "All TaskQueue unit tests passed successfully!" << std::endl;
    return 0;
}