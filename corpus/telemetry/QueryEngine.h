#ifndef QUERYENGINE_H
#define QUERYENGINE_H

#include "SeriesStore.h"
#include <string>
#include <vector>

class QueryEngine {
public:
    SeriesStore store;
    int queries;

    QueryEngine();
    QueryEngine(int bufferDepth);

    int define(const std::string& series);
    int feed(const std::string& series, double value);
    double last(const std::string& series);
    double first(const std::string& series);
    double average(const std::string& series);
    double delta(const std::string& series);
    double sumOver(const std::vector<std::string>& series);
    double maximumOf(const std::vector<std::string>& series);
    int queryCount() const;
    int knownSeries() const;
    bool exists(const std::string& series) const;
};

#endif
