#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#include "Task.h"
#include "Policy.h"
#include "TaskQueue.h"

void test_constructors_and_defaults() {
    // Default constructor
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.policy == nullptr);

    // Parameterized constructor with positive capacity
    TaskQueue customQueue(4);
    assert(customQueue.size() == 0);
    assert(customQueue.isEmpty());
    assert(!customQueue.isFull());
    assert(customQueue.capacity == 4);
    assert(customQueue.rejectedCount() == 0);
    assert(customQueue.policy == nullptr);

    // Parameterized constructor with invalid capacity (<= 0 defaults to 1)
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negativeQueue(-5);
    assert(negativeQueue.capacity == 1);

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue q(3);
    Task t1(1, "Task 1", 5, 10);
    Task t2(2, "Task 2", 3, 20);
    Task t3(3, "Task 3", 1, 15);
    Task t4(4, "Task 4", 2, 8);

    assert(q.enqueue(t1) == 1);
    assert(q.size() == 1);
    assert(!q.isEmpty());
    assert(!q.isFull());

    assert(q.enqueue(t2) == 2);
    assert(q.size() == 2);
    assert(!q.isFull());

    assert(q.enqueue(t3) == 3);
    assert(q.size() == 3);
    assert(q.isFull());
    assert(q.rejectedCount() == 0);

    // Enqueue beyond capacity should fail and increment rejected count
    int res = q.enqueue(t4);
    assert(res == -1);
    assert(q.size() == 3);
    assert(q.rejectedCount() == 1);

    // Enqueue another rejected task
    assert(q.enqueue(t4) == -1);
    assert(q.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_policy_filtering_on_enqueue() {
    TaskQueue q(5);
    PriorityPolicy priorityPolicy(10); // Rejects tasks with priority < 10
    q.usePolicy(&priorityPolicy);

    Task validTask(1, "High Priority", 12, 10);
    Task invalidTask(2, "Low Priority", 5, 10);

    assert(q.enqueue(validTask) == 1);
    assert(q.size() == 1);
    assert(q.rejectedCount() == 0);

    // Should be rejected by policy
    assert(q.enqueue(invalidTask) == -1);
    assert(q.size() == 1);
    assert(q.rejectedCount() == 1);

    // Change policy to deadline policy
    DeadlinePolicy deadlinePolicy(50);
    q.usePolicy(&deadlinePolicy);

    Task urgentTask(3, "Urgent", 1, 30); // deadline is 30 (duration) <= 50 -> accepted
    Task overdueTask(4, "Too late", 1, 60); // deadline is 60 > 50 -> rejected

    assert(q.enqueue(urgentTask) == 2);
    assert(q.size() == 2);
    assert(q.rejectedCount() == 1);

    assert(q.enqueue(overdueTask) == -1);
    assert(q.size() == 2);
    assert(q.rejectedCount() == 2);

    std::cout << "Test test_policy_filtering_on_enqueue: PASS" << std::endl;
}

void test_default_ranking_and_dequeue() {
    // When no policy is set, Task::weight() is used for ranking: priority * 10 (+ 25 if overdue)
    TaskQueue q;

    // Test best() and dequeue() on empty queue
    assert(q.bestIndex() == -1);
    Task emptyBest = q.best();
    assert(emptyBest.identifier() == 0);
    Task emptyDequeued = q.dequeue();
    assert(emptyDequeued.identifier() == 0);

    Task t1(1, "Low", 2, 10);   // weight = 20
    Task t2(2, "High", 10, 10); // weight = 100
    Task t3(3, "Mid", 5, 10);   // weight = 50

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    assert(q.bestIndex() == 1);
    assert(q.best().identifier() == 2);

    // Dequeue highest weight (t2)
    Task dequeued = q.dequeue();
    assert(dequeued.identifier() == 2);
    assert(q.size() == 2);

    // Next best should now be t3 (index 1 in remaining list [t1, t3])
    assert(q.bestIndex() == 1);
    assert(q.best().identifier() == 3);

    // Dequeue remaining
    assert(q.dequeue().identifier() == 3);
    assert(q.dequeue().identifier() == 1);
    assert(q.isEmpty());

    std::cout << "Test test_default_ranking_and_dequeue: PASS" << std::endl;
}

void test_policy_ranking_and_usePolicy() {
    TaskQueue q;
    
    // FifoPolicy rank: bias - id * arrivalWeight (lower id = higher rank)
    FifoPolicy fifo(2);
    int policyRes = q.usePolicy(&fifo);
    assert(policyRes == 0);
    assert(q.policy == &fifo);

    Task t1(10, "First", 5, 20);  // rank = 0 - 20 = -20
    Task t2(5, "Second", 1, 20);  // rank = 0 - 10 = -10 (better score)
    Task t3(20, "Third", 10, 20); // rank = 0 - 40 = -40

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    assert(q.bestIndex() == 1); // t2 has score -10
    assert(q.best().identifier() == 5);

    // Switch to PriorityPolicy: rank = max(priority, floor) * 100 - duration + bias
    PriorityPolicy priorityPolicy(0);
    q.usePolicy(&priorityPolicy);
    // t1: 5 * 100 - 20 = 480
    // t2: 1 * 100 - 20 = 80
    // t3: 10 * 100 - 20 = 980 (highest)
    assert(q.bestIndex() == 2);
    assert(q.best().identifier() == 20);

    // Reset policy to null
    q.usePolicy(nullptr);
    // Weights: t1=50, t2=10, t3=100 -> t3 is best
    assert(q.bestIndex() == 2);
    assert(q.best().identifier() == 20);

    std::cout << "Test test_policy_ranking_and_usePolicy: PASS" << std::endl;
}

void test_total_remaining_and_pending_count() {
    TaskQueue q;

    assert(q.totalRemaining() == 0);
    assert(q.pendingCount() == 0);

    Task t1(1, "A", 1, 10);
    Task t2(2, "B", 1, 20);
    Task t3(3, "C", 1, 30);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    assert(q.totalRemaining() == 60);
    assert(q.pendingCount() == 3);

    // Advance first task partially
    q.tasks[0].advance(4);
    assert(q.tasks[0].isRunning());
    assert(q.tasks[0].remaining() == 6);
    assert(q.totalRemaining() == 56);
    assert(q.pendingCount() == 2); // only t2 and t3 are still pending

    std::cout << "Test test_total_remaining_and_pending_count: PASS" << std::endl;
}

void test_advance_all_and_purge_complete() {
    TaskQueue q;
    Task t1(1, "Short", 1, 5);
    Task t2(2, "Medium", 1, 10);
    Task t3(3, "Long", 1, 20);

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    // Advance 5 units: t1 completes, t2 has 5 left, t3 has 15 left
    int completed = q.advanceAll(5);
    assert(completed == 1);
    assert(q.tasks[0].isComplete());
    assert(!q.tasks[1].isComplete());
    assert(!q.tasks[2].isComplete());
    assert(q.totalRemaining() == 0 + 5 + 15);

    // Advance 10 more units: t2 completes, t3 has 5 left
    completed = q.advanceAll(10);
    assert(completed == 2); // t1 was already complete, t2 newly completed
    assert(q.tasks[0].isComplete());
    assert(q.tasks[1].isComplete());
    assert(!q.tasks[2].isComplete());

    // Advance with 0 or negative units should not alter progress
    completed = q.advanceAll(0);
    assert(completed == 2);
    completed = q.advanceAll(-5);
    assert(completed == 2);

    // Purge completed tasks (t1 and t2)
    int removed = q.purgeComplete();
    assert(removed == 2);
    assert(q.size() == 1);
    assert(q.tasks[0].identifier() == 3);
    assert(q.tasks[0].remaining() == 5);

    // Purging again when no completed tasks exist
    assert(q.purgeComplete() == 0);
    assert(q.size() == 1);

    // Finish last task and purge
    q.advanceAll(10);
    assert(q.purgeComplete() == 1);
    assert(q.isEmpty());

    std::cout << "Test test_advance_all_and_purge_complete: PASS" << std::endl;
}

int main() {
    test_constructors_and_defaults();
    test_enqueue_and_capacity_limits();
    test_policy_filtering_on_enqueue();
    test_default_ranking_and_dequeue();
    test_policy_ranking_and_usePolicy();
    test_total_remaining_and_pending_count();
    test_advance_all_and_purge_complete();

    std::cout << "\nAll TaskQueue tests passed successfully!" << std::endl;
    return 0;
}