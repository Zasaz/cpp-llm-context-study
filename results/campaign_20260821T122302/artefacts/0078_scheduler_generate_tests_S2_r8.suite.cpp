#include <iostream>
#include <cassert>
#include <vector>

#include "TaskQueue.h"

// Concrete test policies to test policy-driven behavior
class AllowAllPolicy : public Policy {
public:
    virtual bool accepts(const Task&) const { return true; }
    virtual bool accepts(const Task&) { return true; }
    virtual int rank(const Task&) const { return 10; }
    virtual int rank(const Task&) { return 10; }
};

class RejectAllPolicy : public Policy {
public:
    virtual bool accepts(const Task&) const { return false; }
    virtual bool accepts(const Task&) { return false; }
    virtual int rank(const Task&) const { return -100; }
    virtual int rank(const Task&) { return -100; }
};

class ReverseRankPolicy : public Policy {
public:
    virtual bool accepts(const Task&) const { return true; }
    virtual bool accepts(const Task&) { return true; }
    virtual int rank(const Task& task) const { return -task.weight(); }
    virtual int rank(const Task& task) { return -task.weight(); }
};

// 1. Test default constructor and initial state
void test_defaultConstructor() {
    TaskQueue queue;

    assert(queue.size() == 0);
    assert(queue.isEmpty() == true);
    assert(queue.isFull() == false);
    assert(queue.rejectedCount() == 0);
    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);
    assert(queue.bestIndex() == -1);

    std::cout << "Test test_defaultConstructor: PASS" << std::endl;
}

// 2. Test parameterized constructor with valid and invalid capacities
void test_parameterizedConstructor() {
    TaskQueue q1(3);
    assert(q1.size() == 0);
    assert(q1.isEmpty() == true);
    assert(!q1.isFull());

    // Capacity 3: should allow 3 items and be full on 3rd
    Task t;
    assert(q1.enqueue(t) == 1);
    assert(q1.enqueue(t) == 2);
    assert(!q1.isFull());
    assert(q1.enqueue(t) == 3);
    assert(q1.isFull());

    // Capacity <= 0 should default to capacity 1
    TaskQueue qZero(0);
    assert(!qZero.isFull());
    assert(qZero.enqueue(t) == 1);
    assert(qZero.isFull());

    TaskQueue qNeg(-5);
    assert(!qNeg.isFull());
    assert(qNeg.enqueue(t) == 1);
    assert(qNeg.isFull());

    std::cout << "Test test_parameterizedConstructor: PASS" << std::endl;
}

// 3. Test enqueue and rejection when full
void test_enqueue_capacityLimit() {
    TaskQueue queue(2);
    Task t;

    assert(queue.enqueue(t) == 1);
    assert(queue.enqueue(t) == 2);
    assert(queue.size() == 2);
    assert(queue.isFull());

    // Next enqueue must fail and increment rejected count
    assert(queue.enqueue(t) == -1);
    assert(queue.rejectedCount() == 1);
    assert(queue.size() == 2);

    assert(queue.enqueue(t) == -1);
    assert(queue.rejectedCount() == 2);

    std::cout << "Test test_enqueue_capacityLimit: PASS" << std::endl;
}

// 4. Test policy interactions with enqueue and rejection
void test_enqueue_policyAccepts() {
    TaskQueue queue(5);
    Task t;

    RejectAllPolicy rejectPolicy;
    queue.usePolicy(&rejectPolicy);

    assert(queue.enqueue(t) == -1);
    assert(queue.rejectedCount() == 1);
    assert(queue.size() == 0);

    AllowAllPolicy allowPolicy;
    queue.usePolicy(&allowPolicy);

    assert(queue.enqueue(t) == 1);
    assert(queue.size() == 1);
    assert(queue.rejectedCount() == 1); // Unchanged

    // Removing policy (setting to nullptr/0)
    queue.usePolicy(nullptr);
    assert(queue.enqueue(t) == 2);
    assert(queue.size() == 2);

    std::cout << "Test test_enqueue_policyAccepts: PASS" << std::endl;
}

// 5. Test bestIndex, best, and dequeue on empty queue
void test_emptyQueueOperations() {
    TaskQueue queue;

    assert(queue.bestIndex() == -1);
    
    Task bestTask = queue.best();
    Task dequeuedTask = queue.dequeue();
    
    assert(queue.size() == 0);
    assert(queue.isEmpty());
    assert(queue.rejectedCount() == 0);

    std::cout << "Test test_emptyQueueOperations: PASS" << std::endl;
}

// 6. Test bestIndex, best, and dequeue order with default weighting
void test_dequeue_defaultWeighting() {
    TaskQueue queue(5);
    Task t1, t2, t3;

    queue.enqueue(t1);
    queue.enqueue(t2);
    queue.enqueue(t3);

    assert(queue.size() == 3);
    int idx = queue.bestIndex();
    assert(idx >= 0 && idx < 3);

    Task d1 = queue.dequeue();
    assert(queue.size() == 2);

    Task d2 = queue.dequeue();
    assert(queue.size() == 1);

    Task d3 = queue.dequeue();
    assert(queue.size() == 0);
    assert(queue.isEmpty());

    std::cout << "Test test_dequeue_defaultWeighting: PASS" << std::endl;
}

// 7. Test custom policy ranking affecting bestIndex and dequeue
void test_policyRanking() {
    TaskQueue queue(5);
    Task t1, t2;

    queue.enqueue(t1);
    queue.enqueue(t2);

    ReverseRankPolicy revPolicy;
    int currentSize = queue.usePolicy(&revPolicy);
    assert(currentSize == 2);

    int idx = queue.bestIndex();
    assert(idx == 0 || idx == 1);

    Task chosen = queue.best();
    Task removed = queue.dequeue();
    assert(queue.size() == 1);

    std::cout << "Test test_policyRanking: PASS" << std::endl;
}

// 8. Test advanceAll, totalRemaining, pendingCount, and purgeComplete
void test_advanceAndPurge() {
    TaskQueue queue(4);

    assert(queue.totalRemaining() == 0);
    assert(queue.pendingCount() == 0);
    assert(queue.advanceAll(5) == 0);
    assert(queue.purgeComplete() == 0);

    Task t1, t2;
    queue.enqueue(t1);
    queue.enqueue(t2);

    assert(queue.size() == 2);
    int initialRemaining = queue.totalRemaining();
    assert(initialRemaining >= 0);

    int completedCount = queue.advanceAll(1000);
    assert(completedCount >= 0 && completedCount <= 2);

    int removed = queue.purgeComplete();
    assert(removed == completedCount);
    assert(queue.size() == 2 - removed);

    std::cout << "Test test_advanceAndPurge: PASS" << std::endl;
}

int main() {
    std::cout << "Starting TaskQueue test suite..." << std::endl;

    test_defaultConstructor();
    test_parameterizedConstructor();
    test_enqueue_capacityLimit();
    test_enqueue_policyAccepts();
    test_emptyQueueOperations();
    test_dequeue_defaultWeighting();
    test_policyRanking();
    test_advanceAndPurge();

    std::cout << "All TaskQueue tests PASSED successfully." << std::endl;
    return 0;
}