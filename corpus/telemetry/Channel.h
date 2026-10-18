#ifndef CHANNEL_H
#define CHANNEL_H

#include "Sample.h"
#include "UnitScale.h"
#include <string>
#include <vector>

class Channel {
public:
    std::string name;
    std::vector<Sample> samples;
    UnitScale scale;
    int capacity;
    int dropped;

    Channel();
    Channel(const std::string& channelName, int limit);

    int append(const Sample& sample);
    int count() const;
    int validCount() const;
    int droppedCount() const;
    bool isFull() const;
    bool isEmpty() const;
    double sum() const;
    double mean() const;
    double peak() const;
    double trough() const;
    double range() const;
    double meanInBase() const;
    Sample sampleAt(int position) const;
    Sample newest() const;
    int purgeDegraded();
    std::string channelName() const;
    int rescale(const UnitScale& replacement);
};

#endif
