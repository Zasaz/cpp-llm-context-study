#ifndef SERIESSTORE_H
#define SERIESSTORE_H

#include "RingBuffer.h"
#include <string>
#include <vector>

class SeriesStore {
public:
    std::vector<std::string> names;
    std::vector<RingBuffer> buffers;
    int depth;
    int unknownWrites;

    SeriesStore();
    SeriesStore(int bufferDepth);

    int declare(const std::string& series);
    int indexOf(const std::string& series) const;
    bool knows(const std::string& series) const;
    int seriesCount() const;
    int write(const std::string& series, double value);
    double latest(const std::string& series) const;
    double earliest(const std::string& series) const;
    double averageOf(const std::string& series) const;
    int samplesIn(const std::string& series) const;
    int totalSamples() const;
    int rejectedWrites() const;
    int drop(const std::string& series);
};

#endif
