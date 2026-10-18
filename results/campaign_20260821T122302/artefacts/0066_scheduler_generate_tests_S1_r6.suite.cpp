#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "Policy.h"
#include "Task.h"
#include "TaskQueue.h"

void test_constructors_and_defaults() {
    // Default constructor: capacity should be 8, size 0, rejected 0, policy nullptr
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.policy == 0);
    assert(defaultQueue.rejectedCount() == 0);

    // Custom capacity: positive value
    TaskQueue customQueue(5);
    assert(customQueue.size() == 0);
    assert(customQueue.capacity == 5);
    assert(customQueue.rejectedCount() == 0);

    // Custom capacity: zero or negative values default to capacity 1
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negativeQueue(-10);
    assert(negativeQueue.capacity == 1);

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue queue(3);
    assert(queue.capacity == 3);
    assert(queue.isEmpty());

    Task t1(1, "Task1", 10, 5);
    Task t2(2, "Task2", 20, 10);
    Task t3(3, "Task3", 30, 15);
    Task t4(4, "Task4", 40, 20);

    // Enqueue within capacity
    int s1 = queue.enqueue(t1);
    assert(s1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());

    int s2 = queue.enqueue(t2);
    assert(s2 == 2);
    assert(queue.size() == 2);

    int s3 = queue.enqueue(t3);
    assert(s3 == 3);
    assert(queue.size() == 3);
    assert(queue.isFull());
    assert(queue.rejectedCount() == 0);

    // Enqueue beyond capacity
    int s4 = queue.enqueue(t4);
    assert(s4 == -1);
    assert(queue.size() == 3);
    assert(queue.rejectedCount() == 1);

    // Another rejected enqueue increments rejectedCount
    int s5 = queue.enqueue(t4);
    assert(s5 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_policy_rejection_on_enqueue() {
    TaskQueue queue(5);
    PriorityPolicy priorityPolicy(15); // Minimum priority required is 15
    queue.usePolicy(&priorityPolicy);

    Task allowedTask(1, "Allowed", 20, 5);
    Task rejectedTask(2, "Rejected", 10, 5);

    int res1 = queue.enqueue(allowedTask);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 0);

    int res2 = queue.enqueue(rejectedTask);
    assert(res2 == -1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1);

    DeadlinePolicy deadlinePolicy(50); // Task deadline must be <= 50
    queue.usePolicy(&deadlinePolicy);

    Task okDeadline(3, "OKDeadline", 5, 20); // default deadline = duration = 20 <= 50
    Task lateDeadline(4, "LateDeadline", 5, 20);
    lateDeadline.setDeadline(100); // deadline 100 > 50

    assert(queue.enqueue(okDeadline) == 2);
    assert(queue.enqueue(lateDeadline) == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_policy_rejection_on_enqueue: PASS" << std::endl;
}

void test_best_and_dequeue_default_weight() {
    TaskQueue queue(5);

    // When policy is null, rank is task.weight() = priority * 10 + (isOverdue ? 25 : 0)
    Task t1(1, "LowPriority", 2, 5);    // weight = 20
    Task t2(2, "HighPriority", 10, 5);  // weight = 100
    Task t3(3, "MidPriority", 5, 5);    // weight = 50

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);
    assert(queue.size() == 3);

    // Dequeue best task (t2)
    Task dequeued = queue.dequeue();
    assert(dequeued.identifier() == 2);
    assert(queue.size() == 2);

    // Next best should be t3 (weight 50) at index 1
    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 3);

    // Advance t1 to make it overdue: duration 5, advance 6 -> overdue
    queue.tasks[0].deadline = 2;
    queue.tasks[0].advance(3); // elapsed = 3 > deadline 2 -> isOverdue is true -> weight = 20 + 25 = 45
    assert(queue.tasks[0].isOverdue());
    // t3 still has weight 50, so t3 is still best
    assert(queue.best().identifier() == 3);

    // Dequeue t3
    assert(queue.dequeue().identifier() == 3);
    assert(queue.size() == 1);

    // Only t1 left
    assert(queue.bestIndex() == 0);
    assert(queue.best().identifier() == 1);
    assert(queue.dequeue().identifier() == 1);
    assert(queue.isEmpty());

    std::cout << "Test test_best_and_dequeue_default_weight: PASS" << std::endl;
}

void test_best_and_dequeue_with_fifo_policy() {
    TaskQueue queue(5);
    FifoPolicy fifoPolicy(2); // rank = bias - task.id * 2
    queue.usePolicy(&fifoPolicy);

    Task t1(10, "Task10", 100, 5); // rank = 0 - 20 = -20
    Task t2(5, "Task5", 10, 5);    // rank = 0 - 10 = -10 (higher)
    Task t3(20, "Task20", 50, 5);  // rank = 0 - 40 = -40

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1); // t2 has rank -10, which is max
    assert(queue.best().identifier() == 5);

    Task d1 = queue.dequeue();
    assert(d1.identifier() == 5);
    assert(queue.size() == 2);

    // Next best should be t1 (rank -20 > rank -40)
    assert(queue.best().identifier() == 10);
    assert(queue.dequeue().identifier() == 10);

    // Last task is t3
    assert(queue.best().identifier() == 20);
    assert(queue.dequeue().identifier() == 20);
    assert(queue.isEmpty());

    std::cout << "Test test_best_and_dequeue_with_fifo_policy: PASS" << std::endl;
}

void test_best_and_dequeue_with_priority_policy() {
    TaskQueue queue(5);
    PriorityPolicy priorityPolicy(5); // floorPriority = 5
    // rank = max(priority, floorPriority) * 100 - duration + bias
    queue.usePolicy(&priorityPolicy);

    Task t1(1, "P5_D10", 5, 10); // rank = 500 - 10 = 490
    Task t2(2, "P5_D5", 5, 5);   // rank = 500 - 5 = 495
    Task t3(3, "P8_D20", 8, 20); // rank = 800 - 20 = 780

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 2); // t3 has rank 780
    assert(queue.best().identifier() == 3);

    Task d1 = queue.dequeue();
    assert(d1.identifier() == 3);
    assert(queue.size() == 2);

    // Next best is t2 (495 > 490)
    assert(queue.best().identifier() == 2);
    assert(queue.dequeue().identifier() == 2);

    // Next is t1
    assert(queue.best().identifier() == 1);
    assert(queue.dequeue().identifier() == 1);
    assert(queue.isEmpty());

    std::cout << "Test test_best_and_dequeue_with_priority_policy: PASS" << std::endl;
}

void test_best_and_dequeue_with_deadline_policy() {
    TaskQueue queue(5);
    DeadlinePolicy deadlinePolicy(100); // horizon = 100
    // rank = max(0, horizon - deadline) * 5 + bias
    queue.usePolicy(&deadlinePolicy);

    Task t1(1, "FarDeadline", 1, 10);
    t1.setDeadline(80); // urgency = 20 -> rank = 100

    Task t2(2, "UrgentDeadline", 1, 10);
    t2.setDeadline(20); // urgency = 80 -> rank = 400

    Task t3(3, "MidDeadline", 1, 10);
    t3.setDeadline(50); // urgency = 50 -> rank = 250

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1); // t2 is most urgent
    assert(queue.best().identifier() == 2);

    assert(queue.dequeue().identifier() == 2);
    assert(queue.best().identifier() == 3);
    assert(queue.dequeue().identifier() == 3);
    assert(queue.best().identifier() == 1);
    assert(queue.dequeue().identifier() == 1);

    std::cout << "Test test_best_and_dequeue_with_deadline_policy: PASS" << std::endl;
}

void test_empty_queue_operations() {
    TaskQueue queue(4);

    assert(queue.isEmpty());
    assert(queue.size() == 0);
    assert(queue.bestIndex() == -1);

    Task bestEmpty = queue.best();
    assert(bestEmpty.identifier() == 0);
    assert(bestEmpty.label() == "unnamed");

    Task dequeuedEmpty = queue.dequeue();
    assert(dequeuedEmpty.identifier() == 0);
    assert(dequeuedEmpty.label() == "unnamed");

    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);
    assert(queue.advanceAll(5) == 0);
    assert(queue.purgeComplete() == 0);

    std::cout << "Test test_empty_queue_operations: PASS" << std::endl;
}

void test_totalRemaining_and_pendingCount() {
    TaskQueue queue(5);

    Task t1(1, "T1", 1, 10);
    Task t2(2, "T2", 1, 20);
    Task t3(3, "T3", 1, 30);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.pendingCount() == 3);
    assert(queue.totalRemaining() == 60);

    // Partially advance one task
    queue.tasks[0].advance(4); // remaining becomes 6, started = true
    assert(queue.tasks[0].remaining() == 6);
    assert(!queue.tasks[0].isPending());
    assert(queue.tasks[0].isRunning());

    assert(queue.pendingCount() == 2);
    assert(queue.totalRemaining() == 56);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue(5);

    Task t1(1, "Short", 1, 5);
    Task t2(2, "Medium", 1, 10);
    Task t3(3, "Long", 1, 15);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Advance all by 4 units -> none finished
    int finishedCount1 = queue.advanceAll(4);
    assert(finishedCount1 == 0);
    assert(queue.tasks[0].remaining() == 1);
    assert(queue.tasks[1].remaining() == 6);
    assert(queue.tasks[2].remaining() == 11);
    assert(queue.purgeComplete() == 0);
    assert(queue.size() == 3);

    // Advance all by 2 units -> t1 finishes (needs 1, gets 2)
    int finishedCount2 = queue.advanceAll(2);
    assert(finishedCount2 == 1);
    assert(queue.tasks[0].isComplete());
    assert(queue.tasks[1].remaining() == 4);
    assert(queue.tasks[2].remaining() == 9);

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == 1);
    assert(queue.size() == 2);
    assert(queue.tasks[0].identifier() == 2);
    assert(queue.tasks[1].identifier() == 3);

    // Advance all remaining tasks to completion
    int finishedCount3 = queue.advanceAll(10);
    assert(finishedCount3 == 2);
    assert(queue.purgeComplete() == 2);
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

void test_policy_switching_and_adjust() {
    TaskQueue queue(5);
    Task t1(1, "Low", 5, 10);
    Task t2(2, "High", 10, 20);

    queue.enqueue(t1);
    queue.enqueue(t2);

    // Default ranking: t2 weight 100 > t1 weight 50
    assert(queue.best().identifier() == 2);

    FifoPolicy fifoPolicy(10); // rank = bias - id * 10 -> t1 rank -10, t2 rank -20
    int sizeRet = queue.usePolicy(&fifoPolicy);
    assert(sizeRet == 2);
    assert(queue.best().identifier() == 1);

    // Adjust bias on policy
    fifoPolicy.adjust(50);
    assert(fifoPolicy.bias == 50);
    assert(queue.best().identifier() == 1);

    // Switch back to no policy
    queue.usePolicy(0);
    assert(queue.policy == 0);
    assert(queue.best().identifier() == 2);

    std::cout << "Test test_policy_switching_and_adjust: PASS" << std::endl;
}

int main() {
    test_constructors_and_defaults();
    test_enqueue_and_capacity_limits();
    test_policy_rejection_on_enqueue();
    test_best_and_dequeue_default_weight();
    test_best_and_dequeue_with_fifo_policy();
    test_best_and_dequeue_with_priority_policy();
    test_best_and_dequeue_with_deadline_policy();
    test_empty_queue_operations();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();
    test_policy_switching_and_adjust();

    std::cout << "All TaskQueue unit tests passed successfully!" << std::endl;
    return 0;
}