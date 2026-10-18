#include <iostream>
#include <cassert>
#include <vector>
#include "TaskQueue.h"
#include "Task.h"
#include "Policy.h"

// Concrete mock policy for testing TaskQueue's policy interaction
class MockPolicy : public Policy {
public:
    bool acceptAll;
    int rankMultiplier;

    MockPolicy(bool accept = true, int multiplier = 1) 
        : acceptAll(accept), rankMultiplier(multiplier) {}

    virtual bool accepts(const Task& task) const {
        return acceptAll;
    }

    virtual int rank(const Task& task) const {
        return task.weight() * rankMultiplier;
    }
};

// Concrete policy for selective filtering and custom ranking
class SelectivePolicy : public Policy {
public:
    int minWeightAllowed;

    SelectivePolicy(int minWeight) : minWeightAllowed(minWeight) {}

    virtual bool accepts(const Task& task) const {
        return task.weight() >= minWeightAllowed;
    }

    virtual int rank(const Task& task) const {
        // Inverse rank: smaller remaining time gives higher rank score
        return 1000 - task.remaining();
    }
};

void test_constructors_and_initial_state() {
    // Default constructor
    TaskQueue qDefault;
    assert(qDefault.size() == 0);
    assert(qDefault.isEmpty() == true);
    assert(qDefault.isFull() == false);
    assert(qDefault.capacity == 8);
    assert(qDefault.rejectedCount() == 0);
    assert(qDefault.policy == 0);

    // Custom limit constructor
    TaskQueue qCustom(5);
    assert(qCustom.size() == 0);
    assert(qCustom.isEmpty() == true);
    assert(qCustom.isFull() == false);
    assert(qCustom.capacity == 5);
    assert(qCustom.rejectedCount() == 0);

    // Custom limit <= 0 should default capacity to 1
    TaskQueue qZero(0);
    assert(qZero.capacity == 1);
    TaskQueue qNegative(-5);
    assert(qNegative.capacity == 1);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_enqueue_and_capacity() {
    TaskQueue q(2);
    Task t1;
    Task t2;
    Task t3;

    int res1 = q.enqueue(t1);
    assert(res1 == 1);
    assert(q.size() == 1);
    assert(!q.isEmpty());
    assert(!q.isFull());
    assert(q.rejectedCount() == 0);

    int res2 = q.enqueue(t2);
    assert(res2 == 2);
    assert(q.size() == 2);
    assert(q.isFull());
    assert(q.rejectedCount() == 0);

    // Enqueue when full -> should fail and increment rejected
    int res3 = q.enqueue(t3);
    assert(res3 == -1);
    assert(q.size() == 2);
    assert(q.rejectedCount() == 1);

    std::cout << "Test test_enqueue_and_capacity: PASS" << std::endl;
}

void test_policy_filtering() {
    TaskQueue q(5);
    SelectivePolicy selectivePolicy(10); // Accepts tasks with weight >= 10
    q.usePolicy(&selectivePolicy);

    Task taskAcceptable;
    // Assuming task construction or default weights; adjust via advance or basic state
    // Let's create tasks and test with MockPolicy controlling acceptance
    MockPolicy rejectPolicy(false, 1);
    q.usePolicy(&rejectPolicy);

    int res = q.enqueue(Task());
    assert(res == -1);
    assert(q.rejectedCount() == 1);
    assert(q.size() == 0);

    MockPolicy acceptPolicy(true, 1);
    q.usePolicy(&acceptPolicy);
    res = q.enqueue(Task());
    assert(res == 1);
    assert(q.rejectedCount() == 1);
    assert(q.size() == 1);

    std::cout << "Test test_policy_filtering: PASS" << std::endl;
}

void test_bestIndex_and_best_empty() {
    TaskQueue q;
    assert(q.bestIndex() == -1);
    Task b = q.best();
    // Default task returned
    Task d = q.dequeue();
    assert(q.size() == 0);

    std::cout << "Test test_bestIndex_and_best_empty: PASS" << std::endl;
}

void test_ranking_and_dequeue() {
    TaskQueue q(5);

    Task t1;
    Task t2;
    Task t3;

    q.enqueue(t1);
    q.enqueue(t2);
    q.enqueue(t3);

    // Without policy, bestIndex chooses the first maximal task.weight()
    int chosenIndex = q.bestIndex();
    assert(chosenIndex >= 0 && chosenIndex < q.size());

    int initialSize = q.size();
    Task dequeued = q.dequeue();
    assert(q.size() == initialSize - 1);

    // Test with a policy ranking
    MockPolicy policy(true, 2);
    q.usePolicy(&policy);
    int policyBestIdx = q.bestIndex();
    assert(policyBestIdx >= 0 && policyBestIdx < q.size());

    std::cout << "Test test_ranking_and_dequeue: PASS" << std::endl;
}

void test_advanceAll_and_purgeComplete() {
    TaskQueue q(5);
    Task t1;
    Task t2;
    q.enqueue(t1);
    q.enqueue(t2);

    int totalRemBefore = q.totalRemaining();
    assert(totalRemBefore >= 0);

    int pendingBefore = q.pendingCount();
    assert(pendingBefore >= 0);

    // Advance by some units
    int completedCount = q.advanceAll(1000);
    assert(completedCount >= 0);

    // Purge completed tasks
    int purgedCount = q.purgeComplete();
    assert(purgedCount == completedCount);
    assert(q.size() == 2 - purgedCount);

    std::cout << "Test test_advanceAll_and_purgeComplete: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_enqueue_and_capacity();
    test_policy_filtering();
    test_bestIndex_and_best_empty();
    test_ranking_and_dequeue();
    test_advanceAll_and_purgeComplete();

    std::cout << "\nAll TaskQueue tests passed successfully!" << std::endl;
    return 0;
}