#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cmath>

#include "SamplePipeline.h"
#include "Filter.h"
#include "Channel.h"
#include "Sample.h"
#include "UnitScale.h"

// Helper mock/derived filter for testing filter interactions
class OffsetFilter : public Filter {
public:
    double offset;
    double rejectAbove;

    OffsetFilter(double off = 0.0, double maxVal = 1e9) 
        : offset(off), rejectAbove(maxVal) {}

    double apply(double value) const {
        return value + offset;
    }

    bool rejects(double value) const {
        return value > rejectAbove;
    }
};

class ScaleFilter : public Filter {
public:
    double factor;

    ScaleFilter(double f = 1.0) : factor(f) {}

    double apply(double value) const {
        return value * factor;
    }

    bool rejects(double) const {
        return false;
    }
};

void test_constructors_and_defaults() {
    // Default constructor
    SamplePipeline defaultPipe;
    assert(defaultPipe.acceptedCount() == 0);
    assert(defaultPipe.rejectedCount() == 0);
    assert(defaultPipe.filterCount() == 0);
    assert(defaultPipe.minimumQuality == 1);
    assert(defaultPipe.describe() == "empty");

    // Parameterized constructor
    SamplePipeline namedPipe("Voltage", 10);
    assert(namedPipe.acceptedCount() == 0);
    assert(namedPipe.rejectedCount() == 0);
    assert(namedPipe.filterCount() == 0);
    assert(namedPipe.minimumQuality == 1);
    assert(namedPipe.channel.channelName() == "Voltage");
    assert(namedPipe.channel.capacity == 10);
    assert(namedPipe.describe() == "empty");

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipe("Test", 5);
    OffsetFilter f1(5.0);
    ScaleFilter f2(2.0);

    // Adding null filter should not change count
    assert(pipe.addFilter(nullptr) == 0);
    assert(pipe.filterCount() == 0);

    // Adding filters
    assert(pipe.addFilter(&f1) == 1);
    assert(pipe.addFilter(&f2) == 2);
    assert(pipe.filterCount() == 2);

    // Stage processing
    assert(pipe.stage(10.0, -1) == 10.0); // Out of bounds negative
    assert(pipe.stage(10.0, 2) == 10.0);  // Out of bounds positive
    assert(pipe.stage(10.0, 0) == 15.0);  // Filter 1: 10 + 5
    assert(pipe.stage(10.0, 1) == 20.0);  // Filter 2: 10 * 2

    // Chained processing: (10 + 5) * 2 = 30
    assert(pipe.process(10.0) == 30.0);

    // Clearing filters
    assert(pipe.clearFilters() == 2);
    assert(pipe.filterCount() == 0);
    assert(pipe.process(10.0) == 10.0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_rejection_and_quality() {
    SamplePipeline pipe("QualityTest", 5);
    OffsetFilter rejectingFilter(0.0, 50.0); // Rejects values > 50.0
    pipe.addFilter(&rejectingFilter);

    // Test requireQuality
    assert(pipe.requireQuality(5) == 1);
    assert(pipe.minimumQuality == 5);
    // Invalid minimum quality (negative) shouldn't update
    assert(pipe.requireQuality(-1) == 5);
    assert(pipe.minimumQuality == 5);

    // Test rejection due to quality
    int result = pipe.ingest(1, 20.0, 3); // quality 3 < 5
    assert(result == -1);
    assert(pipe.rejectedCount() == 1);
    assert(pipe.acceptedCount() == 0);

    // Test rejection due to filter
    assert(pipe.wouldReject(60.0) == true);
    assert(pipe.wouldReject(40.0) == false);

    result = pipe.ingest(2, 60.0, 5); // quality ok, but rejected by filter
    assert(result == -1);
    assert(pipe.rejectedCount() == 2);
    assert(pipe.acceptedCount() == 0);

    // Successful ingest
    result = pipe.ingest(3, 40.0, 5);
    assert(result == 1);
    assert(pipe.acceptedCount() == 1);
    assert(pipe.rejectedCount() == 2);

    std::cout << "Test test_rejection_and_quality: PASS" << std::endl;
}

void test_ingest_and_channel_state() {
    SamplePipeline pipe("Sensor", 2);

    // Ingest until saturation
    assert(pipe.isSaturated() == false);
    assert(pipe.validSamples() == 0);

    assert(pipe.ingest(100, 10.0, 1) == 1);
    assert(pipe.validSamples() == 1);
    assert(pipe.isSaturated() == false);

    assert(pipe.ingest(101, 20.0, 1) == 2);
    assert(pipe.validSamples() == 2);
    assert(pipe.isSaturated() == true);

    // Ingesting into a full channel should fail and increment rejection count
    int res = pipe.ingest(102, 30.0, 1);
    assert(res == -1);
    assert(pipe.rejectedCount() == 1);
    assert(pipe.acceptedCount() == 2);

    // Channel stats
    assert(pipe.channelPeak() == 20.0);
    assert(pipe.channelMean() == 15.0);

    Sample latestSample = pipe.latest();
    assert(latestSample.sequence == 101);
    assert(latestSample.value == 20.0);

    std::cout << "Test test_ingest_and_channel_state: PASS" << std::endl;
}

void test_batch_ingest() {
    SamplePipeline pipe("BatchPipe", 10);
    OffsetFilter limitFilter(0.0, 100.0);
    pipe.addFilter(&limitFilter);
    pipe.requireQuality(2);

    std::vector<double> readings = { 10.0, 50.0, 150.0, 20.0 }; // 150.0 should be rejected

    int taken = pipe.ingestBatch(readings, 2);
    assert(taken == 3);
    assert(pipe.acceptedCount() == 3);
    assert(pipe.rejectedCount() == 1);
    assert(pipe.validSamples() == 3);

    std::cout << "Test test_batch_ingest: PASS" << std::endl;
}

void test_unit_scale_and_describe() {
    SamplePipeline pipe("Temperature", 5);
    pipe.scale = UnitScale("Celsius", 1.0, 0.0);

    // Empty channel description
    assert(pipe.describe() == "empty");

    // Ingest a sample
    pipe.ingest(1, 25.0, 1);
    assert(pipe.describe() == "Temperature:Celsius");

    // Scaling conversions
    assert(pipe.normalised(25.0) == pipe.scale.fromBase(25.0));
    assert(pipe.denormalised(25.0) == pipe.scale.toBase(25.0));

    std::cout << "Test test_unit_scale_and_describe: PASS" << std::endl;
}

void test_flush() {
    SamplePipeline pipe("Pressure", 3);
    pipe.ingest(1, 10.0, 1);
    pipe.ingest(2, 20.0, 1);
    pipe.requireQuality(10);
    pipe.ingest(3, 30.0, 1); // Rejected

    assert(pipe.acceptedCount() == 2);
    assert(pipe.rejectedCount() == 1);
    assert(pipe.validSamples() == 2);

    int flushed = pipe.flush();
    assert(flushed == 2);
    assert(pipe.acceptedCount() == 0);
    assert(pipe.rejectedCount() == 0);
    assert(pipe.validSamples() == 0);
    assert(pipe.describe() == "empty");
    assert(pipe.channel.channelName() == "Pressure");
    assert(pipe.channel.capacity == 3);

    std::cout << "Test test_flush: PASS" << std::endl;
}

int main() {
    std::cout << "Starting SamplePipeline unit tests..." << std::endl;

    test_constructors_and_defaults();
    test_filter_management();
    test_rejection_and_quality();
    test_ingest_and_channel_state();
    test_batch_ingest();
    test_unit_scale_and_describe();
    test_flush();

    std::cout << "All SamplePipeline unit tests passed successfully!" << std::endl;
    return 0;
}