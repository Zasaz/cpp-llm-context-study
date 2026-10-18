#include <iostream>
#include <cassert>
#include "TaskQueue.h"
#include "Policy.h"
#include "Task.h"

void test_constructors_and_initial_state() {
    // Default constructor
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty() == true);
    assert(defaultQueue.isFull() == false);
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.policy == nullptr);

    // Parametric constructor with positive limit
    TaskQueue sizedQueue(4);
    assert(sizedQueue.size() == 0);
    assert(sizedQueue.capacity == 4);
    assert(sizedQueue.rejectedCount() == 0);
    assert(sizedQueue.policy == nullptr);

    // Parametric constructor with zero or negative limit (should clamp to 1)
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negativeQueue(-5);
    assert(negativeQueue.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity() {
    TaskQueue queue(3);
    Task t1(1, "Task1", 2, 10);
    Task t2(2, "Task2", 3, 15);
    Task t3(3, "Task3", 1, 5);
    Task t4(4, "Task4", 5, 20);

    assert(queue.isEmpty());

    int ret1 = queue.enqueue(t1);
    assert(ret1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());

    int ret2 = queue.enqueue(t2);
    assert(ret2 == 2);
    assert(queue.size() == 2);

    int ret3 = queue.enqueue(t3);
    assert(ret3 == 3);
    assert(queue.size() == 3);
    assert(queue.isFull());

    // Queue is full; adding t4 should fail and increment rejected count
    int ret4 = queue.enqueue(t4);
    assert(ret4 == -1);
    assert(queue.size() == 3);
    assert(queue.rejectedCount() == 1);

    // Another rejected enqueue
    queue.enqueue(t4);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity: PASS" << std::endl;
}

void test_enqueue_with_policy_acceptance() {
    TaskQueue queue(5);
    PriorityPolicy priorityPolicy(10); // Accepts only priority >= 10
    queue.usePolicy(&priorityPolicy);

    Task lowPriorityTask(1, "Low", 5, 10);
    Task highPriorityTask(2, "High", 12, 10);

    // lowPriorityTask should be rejected by the policy
    int retLow = queue.enqueue(lowPriorityTask);
    assert(retLow == -1);
    assert(queue.size() == 0);
    assert(queue.rejectedCount() == 1);

    // highPriorityTask should be accepted
    int retHigh = queue.enqueue(highPriorityTask);
    assert(retHigh == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1);

    // Completed task policy acceptance with base Policy
    Policy basePolicy;
    queue.usePolicy(&basePolicy);
    Task completedTask(3, "Done", 1, 5);
    completedTask.advance(5); // Make it finished
    assert(completedTask.isComplete());

    int retDone = queue.enqueue(completedTask);
    assert(retDone == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_with_policy_acceptance: PASS" << std::endl;
}

void test_usePolicy() {
    TaskQueue queue(5);
    queue.enqueue(Task(1, "T1", 2, 5));
    queue.enqueue(Task(2, "T2", 4, 5));

    FifoPolicy fifoPolicy;
    int sizeReturned = queue.usePolicy(&fifoPolicy);
    assert(sizeReturned == 2);
    assert(queue.policy == &fifoPolicy);

    // Resetting to null
    sizeReturned = queue.usePolicy(nullptr);
    assert(sizeReturned == 2);
    assert(queue.policy == nullptr);

    std::cout << "Test test_usePolicy: PASS" << std::endl;
}

void test_bestIndex_and_best_no_policy() {
    TaskQueue queue;
    // Empty queue checks
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    assert(emptyBest.identifier() == 0);

    Task t1(10, "LowWeight", 1, 10);   // weight: 1 * 10 = 10
    Task t2(20, "HighWeight", 5, 10);  // weight: 5 * 10 = 50
    Task t3(30, "MidWeight", 3, 10);   // weight: 3 * 10 = 30

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 20);
    assert(queue.best().label() == "HighWeight");

    // Overdue task increases weight by 25
    Task t4(40, "OverdueTask", 3, 10); // weight: 3 * 10 + 25 = 55 (if overdue)
    t4.setDeadline(2);
    t4.advance(3); // elapsed = 3 > deadline = 2 -> isOverdue() is true
    assert(t4.isOverdue());
    assert(t4.weight() == 55);

    queue.enqueue(t4);
    assert(queue.bestIndex() == 3);
    assert(queue.best().identifier() == 40);

    std::cout << "Test test_bestIndex_and_best_no_policy: PASS" << std::endl;
}

void test_bestIndex_and_best_with_policies() {
    // 1. FifoPolicy: rank = bias - task.id * arrivalWeight
    {
        TaskQueue queue;
        FifoPolicy fifo(2); // arrivalWeight = 2
        queue.usePolicy(&fifo);

        Task t1(5, "Task5", 10, 10); // rank = 0 - 5 * 2 = -10
        Task t2(2, "Task2", 1, 10);  // rank = 0 - 2 * 2 = -4
        Task t3(8, "Task8", 5, 10);  // rank = 0 - 8 * 2 = -16

        queue.enqueue(t1);
        queue.enqueue(t2);
        queue.enqueue(t3);

        // t2 has highest rank (-4)
        assert(queue.bestIndex() == 1);
        assert(queue.best().identifier() == 2);
    }

    // 2. PriorityPolicy: rank = max(floor, priority) * 100 - duration + bias
    {
        TaskQueue queue;
        PriorityPolicy priorityPolicy(1);
        queue.usePolicy(&priorityPolicy);

        Task t1(1, "T1", 5, 50); // rank = 5 * 100 - 50 = 450
        Task t2(2, "T2", 5, 10); // rank = 5 * 100 - 10 = 490
        Task t3(3, "T3", 2, 5);  // rank = 2 * 100 - 5 = 195

        queue.enqueue(t1);
        queue.enqueue(t2);
        queue.enqueue(t3);

        // t2 has higher rank due to shorter duration
        assert(queue.bestIndex() == 1);
        assert(queue.best().identifier() == 2);
    }

    // 3. DeadlinePolicy: rank = max(0, horizon - deadline) * 5 + bias
    {
        TaskQueue queue;
        DeadlinePolicy deadlinePolicy(50);
        queue.usePolicy(&deadlinePolicy);

        Task t1(1, "T1", 1, 10);
        t1.setDeadline(40); // urgency = 50 - 40 = 10 -> rank = 50
        Task t2(2, "T2", 1, 10);
        t2.setDeadline(10); // urgency = 50 - 10 = 40 -> rank = 200

        queue.enqueue(t1);
        queue.enqueue(t2);

        // t2 has earlier deadline, hence higher urgency rank
        assert(queue.bestIndex() == 1);
        assert(queue.best().identifier() == 2);
    }

    std::cout << "Test test_bestIndex_and_best_with_policies: PASS" << std::endl;
}

void test_dequeue() {
    TaskQueue queue;

    // Dequeue on empty queue
    Task emptyDequeued = queue.dequeue();
    assert(emptyDequeued.identifier() == 0);
    assert(queue.size() == 0);

    Task t1(1, "Low", 1, 10);
    Task t2(2, "High", 10, 10);
    Task t3(3, "Mid", 5, 10);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Dequeue highest priority (t2)
    Task popped1 = queue.dequeue();
    assert(popped1.identifier() == 2);
    assert(queue.size() == 2);

    // Next highest is t3
    Task popped2 = queue.dequeue();
    assert(popped2.identifier() == 3);
    assert(queue.size() == 1);

    // Last is t1
    Task popped3 = queue.dequeue();
    assert(popped3.identifier() == 1);
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    std::cout << "Test test_dequeue: PASS" << std::endl;
}

void test_totalRemaining_and_pendingCount() {
    TaskQueue queue;
    Task t1(1, "T1", 1, 10);
    Task t2(2, "T2", 2, 20);
    Task t3(3, "T3", 3, 30);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.totalRemaining() == 60);
    assert(queue.pendingCount() == 3);

    // Start advancing one task
    queue.tasks[0].advance(4); // remaining: 6, started: true
    assert(queue.totalRemaining() == 56);
    assert(queue.pendingCount() == 2); // t1 is running, t2 and t3 are pending

    // Complete the first task
    queue.tasks[0].advance(6); // remaining: 0, finished: true
    assert(queue.totalRemaining() == 50);
    assert(queue.pendingCount() == 2);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue;
    Task t1(1, "Short", 1, 5);
    Task t2(2, "Medium", 2, 10);
    Task t3(3, "Long", 3, 15);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Advance all by 5 units -> t1 completes, t2 has 5 left, t3 has 10 left
    int completedCount = queue.advanceAll(5);
    assert(completedCount == 1);
    assert(queue.tasks[0].isComplete());
    assert(!queue.tasks[1].isComplete());
    assert(!queue.tasks[2].isComplete());
    assert(queue.totalRemaining() == 15);

    // Advance all by another 5 units -> t2 completes, t3 has 5 left
    completedCount = queue.advanceAll(5);
    assert(completedCount == 2); // t1 was already complete (stayed complete), t2 became complete
    assert(queue.tasks[1].isComplete());
    assert(!queue.tasks[2].isComplete());
    assert(queue.totalRemaining() == 5);

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == 2); // t1 and t2 removed
    assert(queue.size() == 1);
    assert(queue.tasks[0].identifier() == 3);
    assert(queue.totalRemaining() == 5);

    // Advance remaining task to finish
    queue.advanceAll(10);
    assert(queue.tasks[0].isComplete());
    purged = queue.purgeComplete();
    assert(purged == 1);
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    // Purge on empty queue
    assert(queue.purgeComplete() == 0);

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

int main() {
    std::cout << "Running TaskQueue test suite..." << std::endl;

    test_constructors_and_initial_state();
    test_enqueue_and_capacity();
    test_enqueue_with_policy_acceptance();
    test_usePolicy();
    test_bestIndex_and_best_no_policy();
    test_bestIndex_and_best_with_policies();
    test_dequeue();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();

    std::cout << "All TaskQueue tests PASSED successfully!" << std::endl;
    return 0;
}