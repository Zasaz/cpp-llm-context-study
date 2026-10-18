#include "SamplePipeline.h"

SamplePipeline::SamplePipeline() {
    accepted = 0;
    rejected = 0;
    minimumQuality = 1;
}

SamplePipeline::SamplePipeline(const std::string& channelName, int limit) {
    channel = Channel(channelName, limit);
    accepted = 0;
    rejected = 0;
    minimumQuality = 1;
}

int SamplePipeline::addFilter(Filter* filter) {
    if (filter == 0) {
        return filterCount();
    }
    filters.push_back(filter);
    return filterCount();
}

int SamplePipeline::filterCount() const {
    return static_cast<int>(filters.size());
}

int SamplePipeline::clearFilters() {
    int previous = filterCount();
    filters.clear();
    return previous;
}

double SamplePipeline::process(double reading) {
    double current = reading;
    for (unsigned int i = 0; i < filters.size(); ++i) {
        current = filters[i]->apply(current);
    }
    return current;
}

double SamplePipeline::stage(double reading, int index) {
    if (index < 0 || index >= filterCount()) {
        return reading;
    }
    return filters[static_cast<unsigned int>(index)]->apply(reading);
}

bool SamplePipeline::wouldReject(double reading) const {
    for (unsigned int i = 0; i < filters.size(); ++i) {
        if (filters[i]->rejects(reading)) {
            return true;
        }
    }
    return false;
}

int SamplePipeline::ingest(int sequence, double reading, int quality) {
    if (quality < minimumQuality || wouldReject(reading)) {
        rejected = rejected + 1;
        return -1;
    }
    double filtered = process(reading);
    Sample sample(sequence, filtered, quality);
    if (!sample.isValid()) {
        rejected = rejected + 1;
        return -1;
    }
    if (channel.append(sample) < 0) {
        rejected = rejected + 1;
        return -1;
    }
    accepted = accepted + 1;
    return accepted;
}

int SamplePipeline::ingestBatch(const std::vector<double>& readings, int quality) {
    int taken = 0;
    for (unsigned int i = 0; i < readings.size(); ++i) {
        if (ingest(static_cast<int>(i), readings[i], quality) > 0) {
            taken = taken + 1;
        }
    }
    return taken;
}

int SamplePipeline::acceptedCount() const {
    return accepted;
}

int SamplePipeline::rejectedCount() const {
    return rejected;
}

int SamplePipeline::requireQuality(int minimum) {
    int previous = minimumQuality;
    if (minimum >= 0) {
        minimumQuality = minimum;
    }
    return previous;
}

double SamplePipeline::channelMean() const {
    return channel.mean();
}

double SamplePipeline::channelPeak() const {
    return channel.peak();
}

double SamplePipeline::normalised(double reading) const {
    return scale.fromBase(reading);
}

double SamplePipeline::denormalised(double reading) const {
    return scale.toBase(reading);
}

int SamplePipeline::validSamples() const {
    return channel.validCount();
}

bool SamplePipeline::isSaturated() const {
    return channel.isFull();
}

Sample SamplePipeline::latest() const {
    return channel.newest();
}

int SamplePipeline::flush() {
    int previous = channel.count();
    channel = Channel(channel.channelName(), channel.capacity);
    accepted = 0;
    rejected = 0;
    return previous;
}

std::string SamplePipeline::describe() const {
    if (channel.isEmpty()) {
        return "empty";
    }
    return channel.channelName() + ":" + scale.unitName();
}
