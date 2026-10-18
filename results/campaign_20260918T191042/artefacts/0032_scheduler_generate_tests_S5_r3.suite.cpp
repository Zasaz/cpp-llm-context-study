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

    // Parameterized constructor with positive limit
    TaskQueue customQueue(4);
    assert(customQueue.capacity == 4);
    assert(customQueue.isEmpty());
    assert(!customQueue.isFull());
    assert(customQueue.rejectedCount() == 0);

    // Parameterized constructor with non-positive limit (should clamp to 1)
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negQueue(-5);
    assert(negQueue.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue queue(2);
    Task t1(1, "Task1", 10, 5);
    Task t2(2, "Task2", 20, 8);
    Task t3(3, "Task3", 30, 12);

    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());

    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());

    // Enqueue beyond capacity should fail and increment rejected count
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    int res4 = queue.enqueue(t3);
    assert(res4 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_enqueue_with_policy_acceptance() {
    TaskQueue queue(5);
    PriorityPolicy policy(5); // Accepts tasks with priority >= 5
    queue.usePolicy(&policy);

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

    // Completed task rejected by base Policy acceptance
    Task completedTask(3, "Done", 10, 5);
    completedTask.advance(5);
    assert(completedTask.isComplete());

    Policy basePolicy;
    queue.usePolicy(&basePolicy);
    int res3 = queue.enqueue(completedTask);
    assert(res3 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_with_policy_acceptance: PASS" << std::endl;
}

void test_best_and_dequeue_without_policy() {
    TaskQueue queue(5);

    // Empty queue checks
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    assert(emptyBest.identifier() == 0);
    Task emptyDequeued = queue.dequeue();
    assert(emptyDequeued.identifier() == 0);

    // Insert tasks: weight = priority * 10 (+ 25 if overdue)
    Task t1(1, "Low", 2, 10);   // weight = 20
    Task t2(2, "High", 8, 10);  // weight = 80
    Task t3(3, "Mid", 5, 10);   // weight = 50

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);

    // Dequeue highest priority
    Task dequeued1 = queue.dequeue();
    assert(dequeued1.identifier() == 2);
    assert(queue.size() == 2);

    // Next highest is t3
    assert(queue.bestIndex() == 1); // index in {t1, t3} is 1
    assert(queue.best().identifier() == 3);

    Task dequeued2 = queue.dequeue();
    assert(dequeued2.identifier() == 3);
    assert(queue.size() == 1);

    // Next is t1
    Task dequeued3 = queue.dequeue();
    assert(dequeued3.identifier() == 1);
    assert(queue.isEmpty());

    std::cout << "Test test_best_and_dequeue_without_policy: PASS" << std::endl;
}

void test_best_and_dequeue_with_various_policies() {
    // FifoPolicy: rank = bias - task.id * arrivalWeight
    {
        TaskQueue queue(5);
        FifoPolicy fifoPolicy(2);
        queue.usePolicy(&fifoPolicy);

        Task t1(1, "First", 10, 5);  // rank = -2
        Task t2(2, "Second", 10, 5); // rank = -4
        queue.enqueue(t1);
        queue.enqueue(t2);

        assert(queue.bestIndex() == 0);
        assert(queue.best().identifier() == 1);
    }

    // DeadlinePolicy: rank = (horizon - task.deadline) * 5 + bias
    {
        TaskQueue queue(5);
        DeadlinePolicy deadlinePolicy(50);
        queue.usePolicy(&deadlinePolicy);

        Task t1(1, "Far", 10, 10);
        t1.setDeadline(40); // urgency = 10 -> rank = 50
        Task t2(2, "Urgent", 10, 10);
        t2.setDeadline(10); // urgency = 40 -> rank = 200

        queue.enqueue(t1);
        queue.enqueue(t2);

        assert(queue.bestIndex() == 1);
        assert(queue.best().identifier() == 2);
        assert(queue.dequeue().identifier() == 2);
        assert(queue.size() == 1);
    }

    std::cout << "Test test_best_and_dequeue_with_various_policies: PASS" << std::endl;
}

void test_totalRemaining_and_pendingCount() {
    TaskQueue queue(5);
    Task t1(1, "T1", 5, 10);
    Task t2(2, "T2", 5, 20);
    Task t3(3, "T3", 5, 15);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.totalRemaining() == 45);
    assert(queue.pendingCount() == 3);

    // Advance t1 partially
    queue.tasks[0].advance(4); // remaining: 6, started: true
    assert(queue.totalRemaining() == 41);
    assert(queue.pendingCount() == 2);

    // Advance t2 fully
    queue.tasks[1].advance(20); // remaining: 0, finished: true
    assert(queue.totalRemaining() == 21);
    assert(queue.pendingCount() == 1);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue(5);
    Task t1(1, "T1", 5, 5);
    Task t2(2, "T2", 5, 10);
    Task t3(3, "T3", 5, 15);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Advance all by 5 units
    // t1 duration 5 -> completes (elapsed = 5)
    // t2 duration 10 -> remaining 5 (elapsed = 5)
    // t3 duration 15 -> remaining 10 (elapsed = 5)
    int completedCount = queue.advanceAll(5);
    assert(completedCount == 1);
    assert(queue.size() == 3);

    // Advance all by another 5 units
    // t1 was already complete -> remains complete
    // t2 duration 10 -> completes (elapsed = 10)
    // t3 duration 15 -> remaining 5 (elapsed = 10)
    completedCount = queue.advanceAll(5);
    assert(completedCount == 2); // both t1 and t2 are complete now

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == 2);
    assert(queue.size() == 1);
    assert(queue.tasks[0].identifier() == 3);
    assert(queue.tasks[0].remaining() == 5);

    // Purge again when none are complete
    assert(queue.purgeComplete() == 0);
    assert(queue.size() == 1);

    // Complete remaining task and purge
    queue.advanceAll(10);
    assert(queue.tasks[0].isComplete());
    assert(queue.purgeComplete() == 1);
    assert(queue.isEmpty());

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_enqueue_and_capacity_limits();
    test_enqueue_with_policy_acceptance();
    test_best_and_dequeue_without_policy();
    test_best_and_dequeue_with_various_policies();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();

    std::cout << "\nAll TaskQueue tests passed successfully!" << std::endl;
    return 0;
}