#include "RunStatistics.h"

RunStatistics::RunStatistics() {
    ignored = 0;
}

int RunStatistics::record(int value) {
    if (value < 0) {
        ignored = ignored + 1;
        return observations();
    }
    samples.push_back(value);
    return observations();
}

int RunStatistics::observations() const {
    return static_cast<int>(samples.size());
}

int RunStatistics::total() const {
    int sum = 0;
    for (unsigned int i = 0; i < samples.size(); ++i) {
        sum = sum + samples[i];
    }
    return sum;
}

double RunStatistics::mean() const {
    int n = observations();
    if (n == 0) {
        return 0.0;
    }
    return static_cast<double>(total()) / static_cast<double>(n);
}

int RunStatistics::minimum() const {
    if (samples.empty()) {
        return 0;
    }
    int best = samples[0];
    for (unsigned int i = 1; i < samples.size(); ++i) {
        if (samples[i] < best) {
            best = samples[i];
        }
    }
    return best;
}

int RunStatistics::maximum() const {
    if (samples.empty()) {
        return 0;
    }
    int best = samples[0];
    for (unsigned int i = 1; i < samples.size(); ++i) {
        if (samples[i] > best) {
            best = samples[i];
        }
    }
    return best;
}

int RunStatistics::ignoredCount() const {
    return ignored;
}

int RunStatistics::discard() {
    int previous = observations();
    samples.clear();
    ignored = 0;
    return previous;
}
