#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#include "TaskQueue.h"
#include "Policy.h"
#include "Task.h"

// Test constructor defaults and capacity constraints
void test_constructors_and_capacities() {
    TaskQueue defaultQueue;
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.policy == nullptr);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());

    TaskQueue customQueue(4);
    assert(customQueue.capacity == 4);
    assert(customQueue.size() == 0);

    // Negative or zero limit should default to capacity 1
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negQueue(-5);
    assert(negQueue.capacity == 1);

    std::cout << "Test test_constructors_and_capacities: PASS" << std::endl;
}

// Test enqueueing tasks, size changes, and capacity limits
void test_enqueue_and_capacity_limits() {
    TaskQueue queue(2);
    Task t1(1, "Task1", 5, 10);
    Task t2(2, "Task2", 3, 20);
    Task t3(3, "Task3", 8, 15);

    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());

    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());

    // Enqueueing beyond capacity should fail and increment rejected count
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

// Test policy rejection logic during enqueue
void test_enqueue_policy_rejection() {
    TaskQueue queue(5);
    PriorityPolicy minPriorityPolicy(5);
    queue.usePolicy(&minPriorityPolicy);

    Task highPriority(1, "High", 6, 10);
    Task lowPriority(2, "Low", 2, 10);

    int res1 = queue.enqueue(highPriority);
    assert(res1 == 1);
    assert(queue.rejectedCount() == 0);

    // lowPriority has priority 2 < floorPriority 5 -> rejected by policy
    int res2 = queue.enqueue(lowPriority);
    assert(res2 == -1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1);

    // Deadline policy acceptance check
    DeadlinePolicy deadlinePolicy(50);
    TaskQueue dlQueue(5);
    dlQueue.usePolicy(&deadlinePolicy);

    Task withinDeadline(3, "Within", 1, 30);
    withinDeadline.setDeadline(40);
    Task pastDeadline(4, "Past", 1, 30);
    pastDeadline.setDeadline(60);

    assert(dlQueue.enqueue(withinDeadline) == 1);
    assert(dlQueue.enqueue(pastDeadline) == -1);
    assert(dlQueue.rejectedCount() == 1);

    std::cout << "Test test_enqueue_policy_rejection: PASS" << std::endl;
}

// Test default bestIndex and dequeue (uses Task::weight())
void test_best_and_dequeue_default_weight() {
    TaskQueue queue(5);

    // Empty queue checks
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    assert(emptyBest.identifier() == 0);
    Task emptyDeq = queue.dequeue();
    assert(emptyDeq.identifier() == 0);

    Task t1(1, "T1", 2, 10); // weight = 20
    Task t2(2, "T2", 5, 10); // weight = 50
    Task t3(3, "T3", 3, 10); // weight = 30

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);

    // Dequeue should return the best task and remove it
    Task dequeued = queue.dequeue();
    assert(dequeued.identifier() == 2);
    assert(queue.size() == 2);

    // Next best should now be t3 (weight 30 > t1 weight 20)
    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 3);

    std::cout << "Test test_best_and_dequeue_default_weight: PASS" << std::endl;
}

// Test bestIndex and dequeue when a policy is attached
void test_best_and_dequeue_with_policy() {
    TaskQueue queue(5);
    FifoPolicy fifoPolicy(2);
    queue.usePolicy(&fifoPolicy);

    // Fifo rank: bias - id * arrivalWeight => 0 - id * 2
    // Lower ID has a higher rank (e.g., -2 > -4 > -6)
    Task t1(10, "First", 1, 5);
    Task t2(2, "Second", 10, 5);
    Task t3(5, "Third", 20, 5);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1); // Task t2 has id 2 -> rank -4, t1 rank -20, t3 rank -10 => t2 is best
    Task best = queue.dequeue();
    assert(best.identifier() == 2);
    assert(queue.size() == 2);

    // Now t3 (id 5, rank -10) vs t1 (id 10, rank -20) -> t3 is best
    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 5);

    std::cout << "Test test_best_and_dequeue_with_policy: PASS" << std::endl;
}

// Test totalRemaining and pendingCount calculations
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

    // Advance queue partially
    queue.tasks[0].advance(4); // remaining 6, started
    assert(queue.pendingCount() == 2); // t2, t3 are still pending
    assert(queue.totalRemaining() == 56);

    std::cout << "Test test_totalRemaining_and_pendingCount: PASS" << std::endl;
}

// Test advanceAll and purgeComplete functionality
void test_advanceAll_and_purgeComplete() {
    TaskQueue queue(5);

    Task t1(1, "T1", 1, 5);
    Task t2(2, "T2", 1, 10);
    Task t3(3, "T3", 1, 15);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Advance by 5 units: t1 completes (duration 5), t2 has 5 left, t3 has 10 left
    int completedCount = queue.advanceAll(5);
    assert(completedCount == 1);
    assert(queue.tasks[0].isComplete());
    assert(!queue.tasks[1].isComplete());
    assert(!queue.tasks[2].isComplete());
    assert(queue.totalRemaining() == 15);

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == 1);
    assert(queue.size() == 2);
    assert(queue.tasks[0].identifier() == 2);
    assert(queue.tasks[1].identifier() == 3);

    // Advance remaining tasks by 20 units: both should finish
    completedCount = queue.advanceAll(20);
    assert(completedCount == 2);
    assert(queue.totalRemaining() == 0);

    purged = queue.purgeComplete();
    assert(purged == 2);
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

// Test changing policy dynamically on a populated queue
void test_usePolicy_switching() {
    TaskQueue queue(5);
    Task t1(1, "T1", 2, 20); // weight 20, priority 2
    Task t2(2, "T2", 5, 50); // weight 50, priority 5

    queue.enqueue(t1);
    queue.enqueue(t2);

    // Default: t2 has higher weight
    assert(queue.bestIndex() == 1);

    // Switch to FifoPolicy: rank = -id * 1 => t1 (id 1, rank -1) > t2 (id 2, rank -2)
    FifoPolicy fifo;
    int returnedSize = queue.usePolicy(&fifo);
    assert(returnedSize == 2);
    assert(queue.bestIndex() == 0);
    assert(queue.best().identifier() == 1);

    // Switch back to no policy
    queue.usePolicy(nullptr);
    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);

    std::cout << "Test test_usePolicy_switching: PASS" << std::endl;
}

int main() {
    test_constructors_and_capacities();
    test_enqueue_and_capacity_limits();
    test_enqueue_policy_rejection();
    test_best_and_dequeue_default_weight();
    test_best_and_dequeue_with_policy();
    test_totalRemaining_and_pendingCount();
    test_advanceAll_and_purgeComplete();
    test_usePolicy_switching();

    std::cout << "\nAll TaskQueue unit tests passed successfully!" << std::endl;
    return 0;
}