#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "SamplePipeline.h"
#include "Filter.h"
#include "Channel.h"
#include "Sample.h"
#include "UnitScale.h"

// Concrete implementation of Filter for testing purposes
class OffsetFilter : public Filter {
public:
    double offset;
    double rejectThreshold;

    OffsetFilter(double offsetValue, double rejectAbove = 1000000.0)
        : offset(offsetValue), rejectThreshold(rejectAbove) {}

    double apply(double value) const {
        return value + offset;
    }

    bool rejects(double value) const {
        return value > rejectThreshold;
    }
};

class ScaleFilter : public Filter {
public:
    double factor;

    ScaleFilter(double factorValue) : factor(factorValue) {}

    double apply(double value) const {
        return value * factor;
    }

    bool rejects(double /*value*/) const {
        return false;
    }
};

void test_constructors_and_defaults() {
    SamplePipeline pipelineDefault;
    assert(pipelineDefault.acceptedCount() == 0);
    assert(pipelineDefault.rejectedCount() == 0);
    assert(pipelineDefault.minimumQuality == 1);
    assert(pipelineDefault.filterCount() == 0);
    assert(pipelineDefault.describe() == "empty");

    SamplePipeline pipelineParam("Voltage", 10);
    assert(pipelineParam.acceptedCount() == 0);
    assert(pipelineParam.rejectedCount() == 0);
    assert(pipelineParam.minimumQuality == 1);
    assert(pipelineParam.filterCount() == 0);
    assert(pipelineParam.channel.channelName() == "Voltage");
    assert(pipelineParam.describe() == "empty");

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline("Sensor", 5);
    OffsetFilter f1(2.0);
    ScaleFilter f2(3.0);

    // Adding null filter should not alter count
    assert(pipeline.addFilter(nullptr) == 0);
    assert(pipeline.filterCount() == 0);

    // Adding valid filters
    assert(pipeline.addFilter(&f1) == 1);
    assert(pipeline.addFilter(&f2) == 2);
    assert(pipeline.filterCount() == 2);

    // Process chaining: (10 + 2) * 3 = 36
    double processed = pipeline.process(10.0);
    assert(std::fabs(processed - 36.0) < 1e-6);

    // Stage processing
    assert(std::fabs(pipeline.stage(10.0, 0) - 12.0) < 1e-6); // f1 only: 10 + 2
    assert(std::fabs(pipeline.stage(10.0, 1) - 30.0) < 1e-6); // f2 only: 10 * 3
    assert(std::fabs(pipeline.stage(10.0, -1) - 10.0) < 1e-6); // Out of bounds
    assert(std::fabs(pipeline.stage(10.0, 2) - 10.0) < 1e-6);  // Out of bounds

    // Clear filters
    int prevCount = pipeline.clearFilters();
    assert(prevCount == 2);
    assert(pipeline.filterCount() == 0);
    assert(std::fabs(pipeline.process(10.0) - 10.0) < 1e-6);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_rejection_and_quality_threshold() {
    SamplePipeline pipeline("Temp", 5);
    OffsetFilter f1(0.0, 100.0); // Rejects values > 100.0
    pipeline.addFilter(&f1);

    // wouldReject check
    assert(!pipeline.wouldReject(50.0));
    assert(pipeline.wouldReject(150.0));

    // Quality requirement check
    int prevQuality = pipeline.requireQuality(5);
    assert(prevQuality == 1);
    assert(pipeline.minimumQuality == 5);

    // Passing negative minimum quality should not change it
    pipeline.requireQuality(-1);
    assert(pipeline.minimumQuality == 5);

    // Ingest with lower quality than required -> should reject
    int res = pipeline.ingest(1, 20.0, 4);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Ingest with reading that filter rejects -> should reject
    res = pipeline.ingest(2, 120.0, 5);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 0);

    // Successful ingest
    res = pipeline.ingest(3, 25.0, 5);
    assert(res == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);

    std::cout << "Test test_rejection_and_quality_threshold: PASS" << std::endl;
}

void test_batch_ingest_and_channel_metrics() {
    SamplePipeline pipeline("Pressure", 10);
    OffsetFilter f1(5.0); // Adds 5 to each value
    pipeline.addFilter(&f1);

    std::vector<double> batch = {10.0, 20.0, 30.0};
    int taken = pipeline.ingestBatch(batch, 1);
    assert(taken == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.validSamples() == 3);
    assert(!pipeline.isSaturated());

    // Filtered readings in channel: 15.0, 25.0, 35.0
    // Mean: (15 + 25 + 35) / 3 = 25.0
    assert(std::fabs(pipeline.channelMean() - 25.0) < 1e-6);
    // Peak: 35.0
    assert(std::fabs(pipeline.channelPeak() - 35.0) < 1e-6);

    // Check latest sample
    Sample last = pipeline.latest();
    assert(std::fabs(last.value - 35.0) < 1e-6);
    assert(last.sequence == 2);

    std::cout << "Test test_batch_ingest_and_channel_metrics: PASS" << std::endl;
}

void test_capacity_saturation_and_flush() {
    SamplePipeline pipeline("Current", 2); // Capacity = 2

    assert(pipeline.ingest(1, 1.0, 1) == 1);
    assert(pipeline.ingest(2, 2.0, 1) == 2);
    assert(pipeline.isSaturated());

    // Ingest into a full channel should fail and increment rejected count
    int res = pipeline.ingest(3, 3.0, 1);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 2);

    // Flush
    int flushedCount = pipeline.flush();
    assert(flushedCount == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(!pipeline.isSaturated());
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_capacity_saturation_and_flush: PASS" << std::endl;
}

void test_scaling_and_description() {
    SamplePipeline pipeline("Frequency", 5);

    // Test scale passthrough methods
    double reading = 50.0;
    double norm = pipeline.normalised(reading);
    double denorm = pipeline.denormalised(norm);
    assert(std::fabs(denorm - reading) < 1e-6);

    // Ingest sample to make channel non-empty
    pipeline.ingest(1, 60.0, 1);
    std::string desc = pipeline.describe();
    assert(desc != "empty");
    assert(desc.find("Frequency:") != std::string::npos);

    std::cout << "Test test_scaling_and_description: PASS" << std::endl;
}

int main() {
    test_constructors_and_defaults();
    test_filter_management();
    test_rejection_and_quality_threshold();
    test_batch_ingest_and_channel_metrics();
    test_capacity_saturation_and_flush();
    test_scaling_and_description();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}