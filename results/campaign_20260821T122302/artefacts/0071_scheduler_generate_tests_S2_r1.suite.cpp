#include <iostream>
#include <cassert>
#include <vector>
#include "TaskQueue.h"

// Custom Policy implementation for testing custom filtering and ranking logic
class FilterEvenWeightPolicy : public Policy {
public:
    bool accepts(const Task& task) const {
        return (task.weight() % 2 == 0);
    }

    int rank(const Task& task) const {
        // Invert weight ranking so lower weights have higher rank
        return 1000 - task.weight();
    }
};

class RejectAllPolicy : public Policy {
public:
    bool accepts(const Task& /*task*/) const {
        return false;
    }

    int rank(const Task& task) const {
        return task.weight();
    }
};

void test_constructors_and_initial_state() {
    // Test default constructor
    TaskQueue defaultQueue;
    assert(defaultQueue.size() == 0);
    assert(defaultQueue.isEmpty());
    assert(!defaultQueue.isFull());
    assert(defaultQueue.rejectedCount() == 0);
    assert(defaultQueue.totalRemaining() == 0);
    assert(defaultQueue.pendingCount() == 0);

    // Test custom positive capacity constructor
    TaskQueue customQueue(3);
    assert(customQueue.size() == 0);
    assert(customQueue.isEmpty());
    assert(!customQueue.isFull());
    assert(customQueue.rejectedCount() == 0);

    // Test non-positive capacity fallback to 1
    TaskQueue zeroCapacityQueue(0);
    assert(zeroCapacityQueue.size() == 0);
    assert(!zeroCapacityQueue.isFull());

    TaskQueue negativeCapacityQueue(-5);
    assert(negativeCapacityQueue.size() == 0);
    assert(!negativeCapacityQueue.isFull());

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity_limits() {
    TaskQueue queue(2);

    Task t1;
    Task t2;
    Task t3;

    // Enqueue first task
    int res1 = queue.enqueue(t1);
    assert(res1 == 1);
    assert(queue.size() == 1);
    assert(!queue.isEmpty());
    assert(!queue.isFull());
    assert(queue.rejectedCount() == 0);

    // Enqueue second task (fills queue)
    int res2 = queue.enqueue(t2);
    assert(res2 == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());
    assert(queue.rejectedCount() == 0);

    // Enqueue third task (should be rejected due to capacity)
    int res3 = queue.enqueue(t3);
    assert(res3 == -1);
    assert(queue.size() == 2);
    assert(queue.isFull());
    assert(queue.rejectedCount() == 1);

    // Try enqueuing again when full
    int res4 = queue.enqueue(t3);
    assert(res4 == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_and_capacity_limits: PASS" << std::endl;
}

void test_best_and_dequeue_with_default_policy() {
    // Default queue uses weight() to prioritize tasks
    TaskQueue queue(5);

    // On empty queue
    assert(queue.bestIndex() == -1);
    Task emptyBest = queue.best();
    Task emptyDequeued = queue.dequeue();
    assert(queue.size() == 0);

    Task lowWeight;
    Task highWeight;
    Task midWeight;

    // Advance/setup tasks if Task methods permit, otherwise default instances
    queue.enqueue(lowWeight);
    queue.enqueue(highWeight);
    queue.enqueue(midWeight);

    assert(queue.size() == 3);
    int chosenIndex = queue.bestIndex();
    assert(chosenIndex >= 0 && chosenIndex < queue.size());

    Task chosen = queue.best();
    assert(queue.size() == 3); // best() should not remove

    Task dequeued = queue.dequeue();
    assert(queue.size() == 2);

    // Drain remaining tasks
    queue.dequeue();
    queue.dequeue();
    assert(queue.isEmpty());
    assert(queue.dequeue().remaining() == Task().remaining());

    std::cout << "Test test_best_and_dequeue_with_default_policy: PASS" << std::endl;
}

void test_policy_filtering_and_ranking() {
    TaskQueue queue(5);
    FilterEvenWeightPolicy evenPolicy;

    int curSize = queue.usePolicy(&evenPolicy);
    assert(curSize == 0);

    RejectAllPolicy rejectPolicy;
    TaskQueue rejectQueue(3);
    rejectQueue.usePolicy(&rejectPolicy);

    Task t1;
    int res = rejectQueue.enqueue(t1);
    assert(res == -1);
    assert(rejectQueue.rejectedCount() == 1);
    assert(rejectQueue.isEmpty());

    // Switch policy back to null (0)
    rejectQueue.usePolicy(0);
    int res2 = rejectQueue.enqueue(t1);
    assert(res2 == 1);
    assert(rejectQueue.size() == 1);

    std::cout << "Test test_policy_filtering_and_ranking: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue queue(4);
    Task t1;
    Task t2;

    queue.enqueue(t1);
    queue.enqueue(t2);

    assert(queue.size() == 2);
    int remainingBefore = queue.totalRemaining();
    assert(remainingBefore >= 0);

    // Advance tasks
    int completedCount = queue.advanceAll(100);
    assert(completedCount >= 0);

    int pending = queue.pendingCount();
    assert(pending >= 0 && pending <= queue.size());

    // Purge completed tasks
    int purged = queue.purgeComplete();
    assert(purged == completedCount);
    assert(queue.size() == 2 - purged);

    // Purge again on clean queue
    int purgedAgain = queue.purgeComplete();
    assert(purgedAgain == 0);

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

void test_edge_cases_and_drain() {
    TaskQueue queue(1);
    assert(queue.isEmpty());

    // Operations on empty queue
    assert(queue.advanceAll(5) == 0);
    assert(queue.purgeComplete() == 0);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);
    assert(queue.bestIndex() == -1);

    Task t;
    queue.enqueue(t);
    assert(queue.isFull());
    assert(queue.size() == 1);

    Task dequeued = queue.dequeue();
    assert(queue.isEmpty());
    assert(queue.size() == 0);

    std::cout << "Test test_edge_cases_and_drain: PASS" << std::endl;
}

int main() {
    std::cout << "Starting TaskQueue test suite..." << std::endl;

    test_constructors_and_initial_state();
    test_enqueue_and_capacity_limits();
    test_best_and_dequeue_with_default_policy();
    test_policy_filtering_and_ranking();
    test_advanceAll_and_purgeComplete();
    test_edge_cases_and_drain();

    std::cout << "All TaskQueue tests passed successfully!" << std::endl;
    return 0;
}