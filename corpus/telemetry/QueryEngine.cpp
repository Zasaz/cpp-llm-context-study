#include "QueryEngine.h"

QueryEngine::QueryEngine() {
    queries = 0;
}

QueryEngine::QueryEngine(int bufferDepth) {
    store = SeriesStore(bufferDepth);
    queries = 0;
}

int QueryEngine::define(const std::string& series) {
    return store.declare(series);
}

int QueryEngine::feed(const std::string& series, double value) {
    return store.write(series, value);
}

double QueryEngine::last(const std::string& series) {
    queries = queries + 1;
    return store.latest(series);
}

double QueryEngine::first(const std::string& series) {
    queries = queries + 1;
    return store.earliest(series);
}

double QueryEngine::average(const std::string& series) {
    queries = queries + 1;
    return store.averageOf(series);
}

double QueryEngine::delta(const std::string& series) {
    queries = queries + 1;
    return store.latest(series) - store.earliest(series);
}

double QueryEngine::sumOver(const std::vector<std::string>& series) {
    double total = 0.0;
    for (unsigned int i = 0; i < series.size(); ++i) {
        total = total + average(series[i]);
    }
    return total;
}

double QueryEngine::maximumOf(const std::vector<std::string>& series) {
    if (series.empty()) {
        return 0.0;
    }
    double best = last(series[0]);
    for (unsigned int i = 1; i < series.size(); ++i) {
        double current = last(series[i]);
        if (current > best) {
            best = current;
        }
    }
    return best;
}

int QueryEngine::queryCount() const {
    return queries;
}

int QueryEngine::knownSeries() const {
    return store.seriesCount();
}

bool QueryEngine::exists(const std::string& series) const {
    return store.knows(series);
}
