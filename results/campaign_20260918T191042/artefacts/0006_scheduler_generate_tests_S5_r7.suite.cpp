#include <iostream>
#include <cassert>
#include <vector>
#include <string>

#include "TaskQueue.h"
#include "Policy.h"
#include "Task.h"

// Test constructors, default values, and capacity initialization
void test_constructors_and_initial_state() {
    // Default constructor
    TaskQueue q1;
    assert(q1.size() == 0);
    assert(q1.isEmpty() == true);
    assert(q1.isFull() == false);
    assert(q1.capacity == 8);
    assert(q1.rejectedCount() == 0);
    assert(q1.policy == nullptr);

    // Custom capacity constructor
    TaskQueue q2(5);
    assert(q2.capacity == 5);
    assert(q2.isEmpty() == true);
    assert(q2.isFull() == false);
    assert(q2.rejectedCount() == 0);

    // Custom capacity constructor with zero/negative limit (should clamp to 1)
    TaskQueue q3(0);
    assert(q3.capacity == 1);

    TaskQueue q4(-10);
    assert(q4.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

// Test enqueueing under normal conditions, capacity limits, and rejection tracking
void test_enqueue_capacity_and_rejection() {
    TaskQueue queue(2);
    Task t1(1, "Task1", 5, 10);
    Task t2(2, "Task2", 3, 20);
    Task t3(3, "Task3", 1, 30);

    // Enqueue 1st task
    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());
    assert(queue.rejectedCount() == 0);

    // Enqueue 2nd task (queue becomes full)
    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());
    assert(queue.rejectedCount() == 0);

    // Enqueue 3rd task (should fail and increment rejected)
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.rejectedCount() == 1);

    // Another failed enqueue
    int res4 = queue.enqueue(t3);
    assert(res4 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_capacity_and_rejection: PASS" << std::endl;
}

// Test queue behavior with default ranking (no policy assigned, uses Task::weight())
void test_best_and_dequeue_without_policy() {
    TaskQueue queue(5);
    
    // Empty queue checks
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    assert(emptyBest.identifier() == 0);
    Task emptyDequeued = queue.dequeue();
    assert(emptyDequeued.identifier() == 0);

    // Insert tasks: weight = priority * 10 (unless overdue)
    Task t1(101, "Low", 2, 10);      // weight: 20
    Task t2(102, "High", 8, 10);     // weight: 80
    Task t3(103, "Medium", 5, 10);   // weight: 50

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.size() == 3);
    assert(queue.bestIndex() == 1); // t2 has highest weight
    assert(queue.best().identifier() == 102);

    // Dequeue highest
    Task dequeued1 = queue.dequeue();
    assert(dequeued1.identifier() == 102);
    assert(queue.size() == 2);

    // Next highest should now be t3 (Medium, id 103)
    assert(queue.bestIndex() == 1); // index in remaining vector [t1, t3] is 1
    assert(queue.best().identifier() == 103);

    Task dequeued2 = queue.dequeue();
    assert(dequeued2.identifier() == 103);
    assert(queue.size() == 1);

    Task dequeued3 = queue.dequeue();
    assert(dequeued3.identifier() == 101);
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    std::cout << "Test test_best_and_dequeue_without_policy: PASS" << std::endl;
}

// Test policy application, policy switching, and policy-driven enqueue rejection
void test_policy_ranking_and_acceptance() {
    TaskQueue queue(5);

    // 1. Test with PriorityPolicy
    PriorityPolicy pPolicy(5); // Minimum priority 5
    queue.usePolicy(&pPolicy);

    Task invalidPriority(1, "LowPriority", 3, 10);
    Task validPriority1(2, "ValidHigh", 10, 20); // rank: 10 * 100 - 20 = 980
    Task validPriority2(3, "ValidMid", 6, 5);    // rank: 6 * 100 - 5 = 595

    int res = queue.enqueue(invalidPriority);
    assert(res == -1);
    assert(queue.rejectedCount() == 1);
    assert(queue.isEmpty());

    queue.enqueue(validPriority2);
    queue.enqueue(validPriority1);
    assert(queue.size() == 2);
    assert(queue.best().identifier() == 2);

    // 2. Switch to FifoPolicy
    FifoPolicy fPolicy(1); // rank: bias - id * arrivalWeight => -id
    queue.usePolicy(&fPolicy);
    // Task 2: rank -2, Task 3: rank -3. Task 2 has higher rank (-2 > -3)
    assert(queue.best().identifier() == 2);

    FifoPolicy fPolicy2(2);
    queue.usePolicy(&fPolicy2);
    assert(queue.best().identifier() == 2);

    // 3. Switch to DeadlinePolicy
    DeadlinePolicy dPolicy(50); // horizon 50
    queue.usePolicy(&dPolicy);

    Task taskOverHorizon(4, "FarDeadline", 10, 10);
    taskOverHorizon.setDeadline(100); // 100 > horizon (50)
    int resOver = queue.enqueue(taskOverHorizon);
    assert(resOver == -1);
    assert(queue.rejectedCount() == 2);

    // 4. Test Policy::adjust() modifying queue rankings
    pPolicy.adjust(1000);
    queue.usePolicy(&pPolicy);
    assert(queue.policy->bias == 1000);

    // 5. Reset to nullptr policy
    queue.usePolicy(nullptr);
    assert(queue.policy == nullptr);

    std::cout << "Test test_policy_ranking_and_acceptance: PASS" << std::endl;
}

// Test advanceAll, totalRemaining, pendingCount, and purgeComplete methods
void test_advanceAll_totalRemaining_and_purge() {
    TaskQueue queue(5);
    Task t1(1, "Short", 1, 5);
    Task t2(2, "Medium", 2, 10);
    Task t3(3, "Long", 3, 20);

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.totalRemaining() == 5 + 10 + 20);
    assert(queue.pendingCount() == 3);

    // Advance all by 5 units
    // t1: elapsed 5/5 -> complete
    // t2: elapsed 5/10 -> remaining 5, running
    // t3: elapsed 5/20 -> remaining 15, running
    int completedCount = queue.advanceAll(5);
    assert(completedCount == 1);
    assert(queue.pendingCount() == 0);
    assert(queue.totalRemaining() == 0 + 5 + 15);

    // Advance all by 10 more units
    // t1: already complete -> 0 remaining
    // t2: elapsed 10/10 -> complete
    // t3: elapsed 15/20 -> remaining 5
    completedCount = queue.advanceAll(10);
    assert(completedCount == 2); // t1 and t2 are now complete in queue
    assert(queue.totalRemaining() == 5);

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == 2);
    assert(queue.size() == 1);
    assert(queue.tasks[0].identifier() == 3);
    assert(queue.totalRemaining() == 5);

    // Advance remaining task to completion and purge again
    queue.advanceAll(10);
    assert(queue.tasks[0].isComplete());
    purged = queue.purgeComplete();
    assert(purged == 1);
    assert(queue.isEmpty());
    assert(queue.totalRemaining() == 0);

    std::cout << "Test test_advanceAll_totalRemaining_and_purge: PASS" << std::endl;
}

// Test edge cases: advance with zero/negative units, empty queue operations
void test_edge_cases() {
    TaskQueue queue(3);

    // Operations on empty queue
    assert(queue.advanceAll(10) == 0);
    assert(queue.purgeComplete() == 0);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);
    assert(queue.bestIndex() == -1);

    Task t1(1, "Task1", 5, 10);
    queue.enqueue(t1);

    // Advance by non-positive units
    int comp1 = queue.advanceAll(0);
    assert(comp1 == 0);
    assert(queue.tasks[0].isPending());
    assert(queue.totalRemaining() == 10);

    int comp2 = queue.advanceAll(-5);
    assert(comp2 == 0);
    assert(queue.tasks[0].isPending());
    assert(queue.totalRemaining() == 10);

    // Task restart inside queue directly
    queue.tasks[0].advance(4);
    assert(queue.tasks[0].isRunning());
    queue.tasks[0].restart();
    assert(queue.tasks[0].isPending());
    assert(queue.tasks[0].remaining() == 10);

    std::cout << "Test test_edge_cases: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_enqueue_capacity_and_rejection();
    test_best_and_dequeue_without_policy();
    test_policy_ranking_and_acceptance();
    test_advanceAll_totalRemaining_and_purge();
    test_edge_cases();

    std::cout << "\nAll TaskQueue unit tests passed successfully!" << std::endl;
    return 0;
}