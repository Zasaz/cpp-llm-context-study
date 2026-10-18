#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#include "TaskQueue.h"
#include "Task.h"
#include "Policy.h"

// Test constructor defaults and capacity constraints
void test_constructors_and_initial_state() {
    // Default constructor
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.policy == nullptr);

    // Parametric constructor with valid capacity
    TaskQueue customQueue(3);
    assert(customQueue.capacity == 3);
    assert(customQueue.isEmpty());
    assert(!customQueue.isFull());
    assert(customQueue.rejectedCount() == 0);

    // Parametric constructor with non-positive capacity fallback to 1
    TaskQueue zeroCapQueue(0);
    assert(zeroCapQueue.capacity == 1);

    TaskQueue negCapQueue(-5);
    assert(negCapQueue.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

// Test enqueue, capacity limits, and rejection tracking
void test_enqueue_and_capacity_limits() {
    TaskQueue queue(2);
    Task t1(1, "Task1", 5, 10);
    Task t2(2, "Task2", 3, 20);
    Task t3(3, "Task3", 1, 15);

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

    // Queue is full, enqueue should fail and increment rejected
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    // Try enqueueing another when full
    int res4 = queue.enqueue(t1);
    assert(res4 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

// Test policy application and policy-based enqueue rejection
void test_policy_filtering_on_enqueue() {
    TaskQueue queue(5);
    PriorityPolicy priorityPolicy(10); // Rejects tasks with priority < 10

    int returnedSize = queue.usePolicy(&priorityPolicy);
    assert(returnedSize == 0);
    assert(queue.policy == &priorityPolicy);

    Task lowPriority(1, "Low", 5, 10);
    Task highPriority(2, "High", 12, 10);

    // Should be rejected by policy
    int resLow = queue.enqueue(lowPriority);
    assert(resLow == -1);
    assert(queue.size() == 0);
    assert(queue.rejectedCount() == 1);

    // Should be accepted
    int resHigh = queue.enqueue(highPriority);
    assert(resHigh == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1);

    // Policy rejecting complete tasks
    Policy basePolicy;
    TaskQueue queue2(5);
    queue2.usePolicy(&basePolicy);

    Task finishedTask(3, "Finished", 10, 5);
    finishedTask.advance(5);
    assert(finishedTask.isComplete());

    int resFinished = queue2.enqueue(finishedTask);
    assert(resFinished == -1);
    assert(queue2.rejectedCount() == 1);

    std::cout << "Test test_policy_filtering_on_enqueue: PASS" << std::endl;
}

// Test bestIndex, best, and dequeue with default weighting (no policy)
void test_best_and_dequeue_no_policy() {
    TaskQueue queue(5);

    // Empty queue behavior
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    assert(emptyBest.identifier() == 0);
    assert(emptyBest.label() == "unnamed");

    Task emptyDequeued = queue.dequeue();
    assert(emptyDequeued.identifier() == 0);
    assert(queue.isEmpty());

    // Without policy: task.weight() is used.
    // weight = priority * 10 (+25 if overdue)
    Task t1(101, "LowPriority", 2, 10);   // weight = 20
    Task t2(102, "HighPriority", 8, 10);  // weight = 80
    Task t3(103, "MedPriority", 5, 10);   // weight = 50

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 102);

    // Dequeue highest priority
    Task dequeued1 = queue.dequeue();
    assert(dequeued1.identifier() == 102);
    assert(queue.size() == 2);

    // Next best should be t3 (identifier 103)
    assert(queue.bestIndex() == 1); // index 1 in the remaining [t1, t3]
    assert(queue.best().identifier() == 103);

    Task dequeued2 = queue.dequeue();
    assert(dequeued2.identifier() == 103);
    assert(queue.size() == 1);

    // Final remaining task
    assert(queue.bestIndex() == 0);
    Task dequeued3 = queue.dequeue();
    assert(dequeued3.identifier() == 101);
    assert(queue.isEmpty());

    std::cout << "Test test_best_and_dequeue_no_policy: PASS" << std::endl;
}

// Test ranking and selection with specific Policies (FifoPolicy, DeadlinePolicy)
void test_best_with_policies() {
    // FifoPolicy: rank = bias - task.id * arrivalWeight
    FifoPolicy fifo(2);
    TaskQueue fifoQueue(5);
    fifoQueue.usePolicy(&fifo);

    Task tA(10, "TaskA", 10, 5); // rank = 0 - 20 = -20
    Task tB(5, "TaskB", 1, 5);   // rank = 0 - 10 = -10 (higher)
    Task tC(15, "TaskC", 20, 5); // rank = 0 - 30 = -30

    fifoQueue.enqueue(tA);
    fifoQueue.enqueue(tB);
    fifoQueue.enqueue(tC);

    assert(fifoQueue.bestIndex() == 1);
    assert(fifoQueue.best().identifier() == 5);

    // DeadlinePolicy: rank = (horizon - deadline) * 5 + bias
    DeadlinePolicy deadlinePolicy(50);
    TaskQueue dlQueue(5);
    dlQueue.usePolicy(&deadlinePolicy);

    Task d1(1, "FarDeadline", 10, 10);
    d1.setDeadline(40); // urgency = 50 - 40 = 10 -> rank = 50
    Task d2(2, "NearDeadline", 1, 10);
    d2.setDeadline(10); // urgency = 50 - 10 = 40 -> rank = 200

    dlQueue.enqueue(d1);
    dlQueue.enqueue(d2);

    assert(dlQueue.bestIndex() == 1);
    assert(dlQueue.best().identifier() == 2);

    std::cout << "Test test_best_with_policies: PASS" << std::endl;
}

// Test totalRemaining and pendingCount computations
void test_total_remaining_and_pending_count() {
    TaskQueue queue(5);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);

    Task t1(1, "Task1", 1, 10);
    Task t2(2, "Task2", 1, 20);
    Task t3(3, "Task3", 1, 30);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.totalRemaining() == 60);
    assert(queue.pendingCount() == 3);

    // Start one task (making it running, no longer pending)
    queue.tasks[0].advance(4); // remaining: 6, elapsed: 4, running
    assert(queue.tasks[0].isRunning());
    assert(!queue.tasks[0].isPending());
    assert(queue.totalRemaining() == 56);
    assert(queue.pendingCount() == 2);

    // Complete one task
    queue.tasks[1].advance(20); // remaining: 0, complete
    assert(queue.tasks[1].isComplete());
    assert(!queue.tasks[1].isPending());
    assert(queue.totalRemaining() == 36);
    assert(queue.pendingCount() == 1);

    std::cout << "Test test_total_remaining_and_pending_count: PASS" << std::endl;
}

// Test advanceAll and purgeComplete functionality
void test_advance_all_and_purge_complete() {
    TaskQueue queue(5);
    Task t1(1, "Short", 1, 5);
    Task t2(2, "Medium", 1, 10);
    Task t3(3, "Long", 1, 20);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Advance all by 5 units
    // t1 finishes (5 units duration)
    // t2 has 5 remaining
    // t3 has 15 remaining
    int completed1 = queue.advanceAll(5);
    assert(completed1 == 1);
    assert(queue.tasks[0].isComplete());
    assert(!queue.tasks[1].isComplete());
    assert(!queue.tasks[2].isComplete());
    assert(queue.totalRemaining() == 20);

    // Purge completed tasks
    int purged1 = queue.purgeComplete();
    assert(purged1 == 1);
    assert(queue.size() == 2);
    assert(queue.tasks[0].identifier() == 2);
    assert(queue.tasks[1].identifier() == 3);

    // Purge when no task is complete
    int purged2 = queue.purgeComplete();
    assert(purged2 == 0);
    assert(queue.size() == 2);

    // Advance remaining tasks so all finish
    int completed2 = queue.advanceAll(20);
    assert(completed2 == 2);
    assert(queue.totalRemaining() == 0);

    int purged3 = queue.purgeComplete();
    assert(purged3 == 2);
    assert(queue.isEmpty());

    std::cout << "Test test_advance_all_and_purge_complete: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running TaskQueue Unit Tests ===" << std::endl;

    test_constructors_and_initial_state();
    test_enqueue_and_capacity_limits();
    test_policy_filtering_on_enqueue();
    test_best_and_dequeue_no_policy();
    test_best_with_policies();
    test_total_remaining_and_pending_count();
    test_advance_all_and_purge_complete();

    std::cout << "=== All TaskQueue Tests Passed Successfully! ===" << std::endl;
    return 0;
}