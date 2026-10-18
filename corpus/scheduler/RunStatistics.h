#ifndef RUNSTATISTICS_H
#define RUNSTATISTICS_H

#include <vector>

class RunStatistics {
public:
    std::vector<int> samples;
    int ignored;

    RunStatistics();

    int record(int value);
    int observations() const;
    int total() const;
    double mean() const;
    int minimum() const;
    int maximum() const;
    int ignoredCount() const;
    int discard();
};

#endif
