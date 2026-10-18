#ifndef SAMPLEPIPELINE_H
#define SAMPLEPIPELINE_H

#include "Channel.h"
#include "Filter.h"
#include "Sample.h"
#include "UnitScale.h"
#include <string>
#include <vector>

class SamplePipeline {
public:
    Channel channel;
    std::vector<Filter*> filters;
    UnitScale scale;
    int accepted;
    int rejected;
    int minimumQuality;

    SamplePipeline();
    SamplePipeline(const std::string& channelName, int limit);

    int addFilter(Filter* filter);
    int filterCount() const;
    int clearFilters();
    double process(double reading);
    double stage(double reading, int index);
    bool wouldReject(double reading) const;
    int ingest(int sequence, double reading, int quality);
    int ingestBatch(const std::vector<double>& readings, int quality);
    int acceptedCount() const;
    int rejectedCount() const;
    int requireQuality(int minimum);
    double channelMean() const;
    double channelPeak() const;
    double normalised(double reading) const;
    double denormalised(double reading) const;
    int validSamples() const;
    bool isSaturated() const;
    Sample latest() const;
    int flush();
    std::string describe() const;
};

#endif
