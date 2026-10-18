#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cmath>

#include "SamplePipeline.h"

// Concrete mock Filter implementation for testing pipeline filtering and rejection behavior.
class TestFilter : public Filter {
public:
    double multiplier;
    double offset;
    double rejectThreshold;
    bool enableRejection;

    TestFilter(double mult = 1.0, double off = 0.0, double rejectAbove = 1000.0, bool rejection = false)
        : multiplier(mult), offset(off), rejectThreshold(rejectAbove), enableRejection(rejection) {}

    virtual double apply(double value) const {
        return (value * multiplier) + offset;
    }

    virtual bool rejects(double value) const {
        if (!enableRejection) return false;
        return value > rejectThreshold;
    }
};

void test_constructors_and_defaults() {
    SamplePipeline defaultPipe;
    assert(defaultPipe.acceptedCount() == 0);
    assert(defaultPipe.rejectedCount() == 0);
    assert(defaultPipe.filterCount() == 0);
    assert(defaultPipe.validSamples() == 0);
    assert(!defaultPipe.isSaturated());
    assert(defaultPipe.describe() == "empty");

    SamplePipeline paramPipe("Voltage", 5);
    assert(paramPipe.acceptedCount() == 0);
    assert(paramPipe.rejectedCount() == 0);
    assert(paramPipe.filterCount() == 0);
    assert(paramPipe.validSamples() == 0);
    assert(!paramPipe.isSaturated());
    assert(paramPipe.describe() == "empty");

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_filter_management_and_stages() {
    SamplePipeline pipe("Temperature", 10);

    TestFilter f1(2.0, 0.0); // value * 2
    TestFilter f2(1.0, 5.0); // value + 5

    assert(pipe.filterCount() == 0);
    assert(pipe.addFilter(0) == 0); // Adding null filter should be ignored
    assert(pipe.addFilter(&f1) == 1);
    assert(pipe.addFilter(&f2) == 2);
    assert(pipe.filterCount() == 2);

    // Test stage processing individually
    // Index out of bounds should return original reading
    assert(pipe.stage(10.0, -1) == 10.0);
    assert(pipe.stage(10.0, 2) == 10.0);
    assert(pipe.stage(10.0, 0) == 20.0); // 10 * 2
    assert(pipe.stage(10.0, 1) == 15.0); // 10 + 5

    // Full pipeline process: (10.0 * 2.0) + 5.0 = 25.0
    assert(pipe.process(10.0) == 25.0);

    // Clear filters
    int cleared = pipe.clearFilters();
    assert(cleared == 2);
    assert(pipe.filterCount() == 0);
    assert(pipe.process(10.0) == 10.0);

    std::cout << "Test test_filter_management_and_stages: PASS" << std::endl;
}

void test_ingest_quality_and_rejection() {
    SamplePipeline pipe("Sensor", 5);

    // Default minimum quality is 1
    // Ingest with quality < minimumQuality (e.g. 0) should fail
    int res1 = pipe.ingest(0, 12.5, 0);
    assert(res1 == -1);
    assert(pipe.rejectedCount() == 1);
    assert(pipe.acceptedCount() == 0);

    // Ingest with quality >= 1 should succeed
    int res2 = pipe.ingest(1, 12.5, 1);
    assert(res2 == 1); // returns accepted count
    assert(pipe.acceptedCount() == 1);
    assert(pipe.rejectedCount() == 1);

    // Test changing minimum quality requirement
    int prevMin = pipe.requireQuality(3);
    assert(prevMin == 1);
    // Passing negative minimum should be ignored
    assert(pipe.requireQuality(-5) == 3);

    // Quality 2 should now be rejected
    assert(pipe.ingest(2, 15.0, 2) == -1);
    assert(pipe.rejectedCount() == 2);

    // Quality 3 should be accepted
    assert(pipe.ingest(3, 15.0, 3) == 2);
    assert(pipe.acceptedCount() == 2);

    // Add filter that rejects values > 50.0
    TestFilter rejectFilter(1.0, 0.0, 50.0, true);
    pipe.addFilter(&rejectFilter);

    assert(pipe.wouldReject(100.0) == true);
    assert(pipe.wouldReject(30.0) == false);

    assert(pipe.ingest(4, 100.0, 5) == -1);
    assert(pipe.rejectedCount() == 3);

    std::cout << "Test test_ingest_quality_and_rejection: PASS" << std::endl;
}

void test_ingest_batch_and_saturation() {
    SamplePipeline pipe("BatchChannel", 3);

    std::vector<double> readings;
    readings.push_back(10.0);
    readings.push_back(20.0);
    readings.push_back(30.0);
    readings.push_back(40.0); // Exceeds capacity 3

    int taken = pipe.ingestBatch(readings, 1);
    assert(taken == 3);
    assert(pipe.acceptedCount() == 3);
    assert(pipe.rejectedCount() == 1); // The 4th item overflowed the channel capacity
    assert(pipe.isSaturated());
    assert(pipe.validSamples() == 3);

    std::cout << "Test test_ingest_batch_and_saturation: PASS" << std::endl;
}

void test_channel_statistics_and_latest() {
    SamplePipeline pipe("StatsChannel", 5);

    TestFilter gain(2.0, 0.0); // doubles reading
    pipe.addFilter(&gain);

    pipe.ingest(1, 10.0, 1); // Processed to 20.0
    pipe.ingest(2, 20.0, 1); // Processed to 40.0
    pipe.ingest(3, 30.0, 1); // Processed to 60.0

    assert(std::fabs(pipe.channelMean() - 40.0) < 1e-6);
    assert(std::fabs(pipe.channelPeak() - 60.0) < 1e-6);

    Sample newest = pipe.latest();
    assert(newest.sequence == 3);
    assert(std::fabs(newest.reading - 60.0) < 1e-6);

    std::cout << "Test test_channel_statistics_and_latest: PASS" << std::endl;
}

void test_scale_normalisation() {
    SamplePipeline pipe("ScaleChannel", 5);
    // Tests that normalised and denormalised delegates properly to the internal scale
    double baseVal = 10.0;
    double normVal = pipe.normalised(baseVal);
    double denormVal = pipe.denormalised(normVal);
    assert(std::fabs(baseVal - denormVal) < 1e-6);

    std::cout << "Test test_scale_normalisation: PASS" << std::endl;
}

void test_flush_and_describe() {
    SamplePipeline pipe("Pressure", 4);

    assert(pipe.describe() == "empty");

    pipe.ingest(1, 101.3, 1);
    assert(pipe.describe() != "empty");
    assert(pipe.acceptedCount() == 1);

    int flushedCount = pipe.flush();
    assert(flushedCount == 1);
    assert(pipe.acceptedCount() == 0);
    assert(pipe.rejectedCount() == 0);
    assert(pipe.validSamples() == 0);
    assert(pipe.describe() == "empty");

    std::cout << "Test test_flush_and_describe: PASS" << std::endl;
}

int main() {
    test_constructors_and_defaults();
    test_filter_management_and_stages();
    test_ingest_quality_and_rejection();
    test_ingest_batch_and_saturation();
    test_channel_statistics_and_latest();
    test_scale_normalisation();
    test_flush_and_describe();

    std::cout << "\nAll SamplePipeline unit tests passed successfully!" << std::endl;
    return 0;
}