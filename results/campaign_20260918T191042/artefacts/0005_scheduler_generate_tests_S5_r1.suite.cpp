#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#include "TaskQueue.h"
#include "Policy.h"
#include "Task.h"

// Test constructor defaults and parameterized limits
void test_constructors_and_initial_state() {
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());
    assert(defaultQueue.capacity == 8);
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.policy == nullptr);

    TaskQueue customQueue(5);
    assert(customQueue.capacity == 5);
    assert(customQueue.isEmpty());

    // Non-positive limits should default capacity to 1
    TaskQueue zeroQueue(0);
    assert(zeroQueue.capacity == 1);

    TaskQueue negativeQueue(-10);
    assert(negativeQueue.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

// Test basic enqueue and capacity limit rejection
void test_enqueue_and_capacity_rejections() {
    TaskQueue queue(2);
    Task t1(1, "Task 1", 5, 10);
    Task t2(2, "Task 2", 3, 20);
    Task t3(3, "Task 3", 1, 5);

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

    // Should fail and reject due to being full
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    std::cout << "Test test_enqueue_and_capacity_rejections: PASS" << std::endl;
}

// Test policy change and rejection based on policy rules
void test_policy_rejection() {
    TaskQueue queue(5);
    PriorityPolicy priorityPolicy(10); // Minimum priority 10
    
    int sizeAfterPolicy = queue.usePolicy(&priorityPolicy);
    assert(sizeAfterPolicy == 0);
    assert(queue.policy == &priorityPolicy);

    Task lowPriorityTask(1, "Low", 5, 10);
    Task highPriorityTask(2, "High", 12, 10);

    int resLow = queue.enqueue(lowPriorityTask);
    assert(resLow == -1);
    assert(queue.rejectedCount() == 1);
    assert(queue.size() == 0);

    int resHigh = queue.enqueue(highPriorityTask);
    assert(resHigh == 1);
    assert(queue.rejectedCount() == 1);
    assert(queue.size() == 1);

    std::cout << "Test test_policy_rejection: PASS" << std::endl;
}

// Test ranking and selection with default weight scoring (no policy)
void test_ranking_without_policy() {
    TaskQueue queue(5);
    assert(queue.bestIndex() == -1);
    Task defaultBest = queue.best();
    assert(defaultBest.identifier() == 0);

    Task t1(1, "Low Priority", 1, 10);   // weight = 10
    Task t2(2, "High Priority", 5, 10);  // weight = 50
    Task t3(3, "Medium Priority", 3, 10);// weight = 30

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.bestIndex() == 1);
    assert(queue.best().identifier() == 2);
    assert(queue.best().label() == "High Priority");

    std::cout << "Test test_ranking_without_policy: PASS" << std::endl;
}

// Test ranking with different policies (FifoPolicy, DeadlinePolicy, PriorityPolicy)
void test_ranking_with_policies() {
    // 1. FIFO Policy (lower id ranked higher due to negative weight)
    {
        TaskQueue queue(5);
        FifoPolicy fifo(1);
        queue.usePolicy(&fifo);

        Task t10(10, "First", 5, 10);
        Task t20(20, "Second", 5, 10);
        queue.enqueue(t20);
        queue.enqueue(t10);

        // t10 score: 0 - 10*1 = -10, t20 score: 0 - 20*1 = -20.
        // Higher score (-10 > -20) -> t10 is chosen
        assert(queue.bestIndex() == 1);
        assert(queue.best().identifier() == 10);
    }

    // 2. Deadline Policy
    {
        TaskQueue queue(5);
        DeadlinePolicy deadlinePolicy(50);
        queue.usePolicy(&deadlinePolicy);

        Task tEarly(1, "Early", 2, 10);
        tEarly.setDeadline(10); // urgency = 50 - 10 = 40; rank = 40*5 = 200

        Task tLate(2, "Late", 2, 40);
        tLate.setDeadline(40);  // urgency = 50 - 40 = 10; rank = 10*5 = 50

        queue.enqueue(tLate);
        queue.enqueue(tEarly);

        assert(queue.bestIndex() == 1);
        assert(queue.best().identifier() == 1);
    }

    std::cout << "Test test_ranking_with_policies: PASS" << std::endl;
}

// Test dequeue functionality on empty and populated queues
void test_dequeue() {
    TaskQueue queue(5);
    Task emptyTask = queue.dequeue();
    assert(emptyTask.identifier() == 0);

    Task t1(1, "Low", 2, 10);  // weight 20
    Task t2(2, "High", 8, 10); // weight 80
    Task t3(3, "Mid", 5, 10);  // weight 50

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    Task dequeued1 = queue.dequeue();
    assert(dequeued1.identifier() == 2);
    assert(queue.size() == 2);

    Task dequeued2 = queue.dequeue();
    assert(dequeued2.identifier() == 3);
    assert(queue.size() == 1);

    Task dequeued3 = queue.dequeue();
    assert(dequeued3.identifier() == 1);
    assert(queue.isEmpty());

    Task dequeuedEmpty = queue.dequeue();
    assert(dequeuedEmpty.identifier() == 0);

    std::cout << "Test test_dequeue: PASS" << std::endl;
}

// Test totalRemaining and pendingCount
void test_total_remaining_and_pending_count() {
    TaskQueue queue(5);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);

    Task t1(1, "Task1", 1, 10);
    Task t2(2, "Task2", 1, 15);
    queue.enqueue(t1);
    queue.enqueue(t2);

    assert(queue.pendingCount() == 2);
    assert(queue.totalRemaining() == 25);

    // Advance only one task
    queue.tasks[0].advance(4); // remaining 6, started = true (no longer pending)

    assert(queue.pendingCount() == 1);
    assert(queue.totalRemaining() == 21);

    std::cout << "Test test_total_remaining_and_pending_count: PASS" << std::endl;
}

// Test advanceAll and purgeComplete
void test_advance_all_and_purge_complete() {
    TaskQueue queue(5);
    Task t1(1, "Task 1", 1, 5);
    Task t2(2, "Task 2", 1, 10);
    Task t3(3, "Task 3", 1, 15);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    // Advance all by 5 units
    int completed1 = queue.advanceAll(5);
    assert(completed1 == 1); // t1 finished (5 units)
    assert(queue.tasks[0].isComplete());
    assert(!queue.tasks[1].isComplete());
    assert(!queue.tasks[2].isComplete());
    assert(queue.totalRemaining() == 0 + 5 + 10);

    // Purge complete tasks
    int purged = queue.purgeComplete();
    assert(purged == 1);
    assert(queue.size() == 2);
    assert(queue.tasks[0].identifier() == 2);
    assert(queue.tasks[1].identifier() == 3);

    // Advance by 10 more units (completing t2 and t3)
    int completed2 = queue.advanceAll(10);
    assert(completed2 == 2);
    
    purged = queue.purgeComplete();
    assert(purged == 2);
    assert(queue.isEmpty());

    // Purging on empty queue
    assert(queue.purgeComplete() == 0);

    std::cout << "Test test_advance_all_and_purge_complete: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_enqueue_and_capacity_rejections();
    test_policy_rejection();
    test_ranking_without_policy();
    test_ranking_with_policies();
    test_dequeue();
    test_total_remaining_and_pending_count();
    test_advance_all_and_purge_complete();

    std::cout << "All TaskQueue tests passed successfully!" << std::endl;
    return 0;
}