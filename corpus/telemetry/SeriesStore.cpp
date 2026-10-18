#include "SeriesStore.h"

SeriesStore::SeriesStore() {
    depth = 8;
    unknownWrites = 0;
}

SeriesStore::SeriesStore(int bufferDepth) {
    depth = bufferDepth > 0 ? bufferDepth : 1;
    unknownWrites = 0;
}

int SeriesStore::declare(const std::string& series) {
    int existing = indexOf(series);
    if (existing >= 0) {
        return existing;
    }
    names.push_back(series);
    buffers.push_back(RingBuffer(depth));
    return seriesCount() - 1;
}

int SeriesStore::indexOf(const std::string& series) const {
    for (unsigned int i = 0; i < names.size(); ++i) {
        if (names[i] == series) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

bool SeriesStore::knows(const std::string& series) const {
    return indexOf(series) >= 0;
}

int SeriesStore::seriesCount() const {
    return static_cast<int>(names.size());
}

int SeriesStore::write(const std::string& series, double value) {
    int index = indexOf(series);
    if (index < 0) {
        unknownWrites = unknownWrites + 1;
        return -1;
    }
    return buffers[static_cast<unsigned int>(index)].push(value);
}

double SeriesStore::latest(const std::string& series) const {
    int index = indexOf(series);
    if (index < 0) {
        return 0.0;
    }
    return buffers[static_cast<unsigned int>(index)].newest();
}

double SeriesStore::earliest(const std::string& series) const {
    int index = indexOf(series);
    if (index < 0) {
        return 0.0;
    }
    return buffers[static_cast<unsigned int>(index)].oldest();
}

double SeriesStore::averageOf(const std::string& series) const {
    int index = indexOf(series);
    if (index < 0) {
        return 0.0;
    }
    return buffers[static_cast<unsigned int>(index)].average();
}

int SeriesStore::samplesIn(const std::string& series) const {
    int index = indexOf(series);
    if (index < 0) {
        return 0;
    }
    return buffers[static_cast<unsigned int>(index)].occupancy();
}

int SeriesStore::totalSamples() const {
    int total = 0;
    for (unsigned int i = 0; i < buffers.size(); ++i) {
        total = total + buffers[i].occupancy();
    }
    return total;
}

int SeriesStore::rejectedWrites() const {
    return unknownWrites;
}

int SeriesStore::drop(const std::string& series) {
    int index = indexOf(series);
    if (index < 0) {
        return -1;
    }
    names.erase(names.begin() + index);
    buffers.erase(buffers.begin() + index);
    return seriesCount();
}
