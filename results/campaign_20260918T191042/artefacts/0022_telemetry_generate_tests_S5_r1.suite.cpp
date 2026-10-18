#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>

#include "SamplePipeline.h"
#include "Channel.h"
#include "Filter.h"
#include "Sample.h"
#include "UnitScale.h"

// Helper for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_constructors_and_defaults() {
    // Default constructor
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.minimumQuality == 1);
    assert(defaultPipeline.channel.channelName() == "unnamed");
    assert(defaultPipeline.channel.capacity == 16);
    assert(!defaultPipeline.isSaturated());

    // Parameterized constructor
    SamplePipeline namedPipeline("sensor_temp", 5);
    assert(namedPipeline.acceptedCount() == 0);
    assert(namedPipeline.rejectedCount() == 0);
    assert(namedPipeline.filterCount() == 0);
    assert(namedPipeline.minimumQuality == 1);
    assert(namedPipeline.channel.channelName() == "sensor_temp");
    assert(namedPipeline.channel.capacity == 5);
    assert(!namedPipeline.isSaturated());

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline("ch", 10);
    PassFilter pass;
    ClampFilter clamp(0.0, 100.0);

    assert(pipeline.filterCount() == 0);

    // Adding null filter should do nothing
    assert(pipeline.addFilter(nullptr) == 0);
    assert(pipeline.filterCount() == 0);

    // Adding valid filters
    assert(pipeline.addFilter(&pass) == 1);
    assert(pipeline.addFilter(&clamp) == 2);
    assert(pipeline.filterCount() == 2);

    // Clearing filters
    int previousCount = pipeline.clearFilters();
    assert(previousCount == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipeline("ch", 10);
    ClampFilter clamp(0.0, 50.0);
    SmoothFilter smooth(0.5); // Initial reading primes it

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // Testing stage execution with out-of-bounds indices
    assert(isClose(pipeline.stage(100.0, -1), 100.0));
    assert(isClose(pipeline.stage(100.0, 2), 100.0));

    // Stage 0 (clamp only, reading 100 clamped to 50)
    assert(isClose(pipeline.stage(100.0, 0), 50.0));

    // Full process: reading 80.0 clamped to 50.0, then smooth primed with 50.0
    double processed1 = pipeline.process(80.0);
    assert(isClose(processed1, 50.0));

    // Second reading through pipeline: 10.0 clamped to 10.0, then smooth: 0.5 * 10 + 0.5 * 50 = 30.0
    double processed2 = pipeline.process(10.0);
    assert(isClose(processed2, 30.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_wouldReject() {
    SamplePipeline pipeline("ch", 10);
    ClampFilter clamp(10.0, 20.0);
    pipeline.addFilter(&clamp);

    assert(pipeline.wouldReject(5.0) == true);
    assert(pipeline.wouldReject(25.0) == true);
    assert(pipeline.wouldReject(15.0) == false);

    // Check with disabled/toggled filter
    PassFilter pass;
    pipeline.clearFilters();
    pipeline.addFilter(&pass);
    assert(pipeline.wouldReject(5.0) == false);
    pass.toggle(); // enabled becomes false
    assert(pipeline.wouldReject(5.0) == true);
    assert(pipeline.wouldReject(0.0) == false); // Filter::rejects is false if reading == 0.0

    std::cout << "Test test_wouldReject: PASS" << std::endl;
}

void test_requireQuality_and_ingest_rejections() {
    SamplePipeline pipeline("ch", 10);
    pipeline.requireQuality(50);
    assert(pipeline.minimumQuality == 50);

    // Passing a negative quality to requireQuality should not change it
    pipeline.requireQuality(-5);
    assert(pipeline.minimumQuality == 50);

    // Reject due to quality < minimumQuality
    int res1 = pipeline.ingest(0, 10.0, 40);
    assert(res1 == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 1);

    // Reject due to filter rejection
    ClampFilter clamp(0.0, 10.0);
    pipeline.addFilter(&clamp);
    int res2 = pipeline.ingest(1, 20.0, 60);
    assert(res2 == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 2);

    // Accept valid sample
    int res3 = pipeline.ingest(2, 5.0, 60);
    assert(res3 == 1); // accepted count = 1
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 1);

    std::cout << "Test test_requireQuality_and_ingest_rejections: PASS" << std::endl;
}

void test_ingest_capacity_saturation() {
    SamplePipeline pipeline("small_ch", 2);

    int res1 = pipeline.ingest(1, 10.0, 80);
    assert(res1 == 1);
    assert(!pipeline.isSaturated());

    int res2 = pipeline.ingest(2, 20.0, 80);
    assert(res2 == 2);
    assert(pipeline.isSaturated());

    // Channel is now full (capacity = 2). Next ingestion should fail due to channel overflow
    int res3 = pipeline.ingest(3, 30.0, 80);
    assert(res3 == -1);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.channel.droppedCount() == 1);

    std::cout << "Test test_ingest_capacity_saturation: PASS" << std::endl;
}

void test_ingestBatch() {
    SamplePipeline pipeline("batch_ch", 5);
    ClampFilter clamp(0.0, 50.0);
    pipeline.addFilter(&clamp);

    std::vector<double> readings = { 10.0, 60.0, 20.0, 70.0, 30.0 };
    // Readings 60.0 and 70.0 will be rejected by clamp
    int taken = pipeline.ingestBatch(readings, 80);
    assert(taken == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 3);

    std::cout << "Test test_ingestBatch: PASS" << std::endl;
}

void test_channel_statistics_and_latest() {
    SamplePipeline pipeline("stats_ch", 10);
    pipeline.ingest(1, 10.0, 90);
    pipeline.ingest(2, 30.0, 90);
    pipeline.ingest(3, 20.0, 90);

    assert(isClose(pipeline.channelMean(), 20.0));
    assert(isClose(pipeline.channelPeak(), 30.0));

    Sample latestSample = pipeline.latest();
    assert(latestSample.position() == 3);
    assert(isClose(latestSample.value(), 20.0));
    assert(latestSample.score() == 90);

    std::cout << "Test test_channel_statistics_and_latest: PASS" << std::endl;
}

void test_scaling_normalised_denormalised() {
    SamplePipeline pipeline("scaled_ch", 10);
    pipeline.scale = UnitScale(2.0, 5.0, "volt");

    // scale.toBase(x) = x * 2.0 + 5.0
    // scale.fromBase(x) = (x - 5.0) / 2.0
    double baseVal = 25.0;
    double norm = pipeline.normalised(baseVal);
    assert(isClose(norm, (25.0 - 5.0) / 2.0)); // 10.0

    double denorm = pipeline.denormalised(10.0);
    assert(isClose(denorm, 10.0 * 2.0 + 5.0)); // 25.0

    std::cout << "Test test_scaling_normalised_denormalised: PASS" << std::endl;
}

void test_flush_and_describe() {
    SamplePipeline pipeline("sensor_A", 4);
    pipeline.scale = UnitScale(1.0, 0.0, "Celsius");

    // Describe when empty
    assert(pipeline.describe() == "empty");

    pipeline.ingest(1, 22.5, 95);
    assert(pipeline.describe() == "sensor_A:Celsius");
    assert(pipeline.acceptedCount() == 1);

    // Flush the pipeline
    int flushedCount = pipeline.flush();
    assert(flushedCount == 1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_flush_and_describe: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running SamplePipeline Test Suite ===" << std::endl;

    test_constructors_and_defaults();
    test_filter_management();
    test_process_and_stage();
    test_wouldReject();
    test_requireQuality_and_ingest_rejections();
    test_ingest_capacity_saturation();
    test_ingestBatch();
    test_channel_statistics_and_latest();
    test_scaling_normalised_denormalised();
    test_flush_and_describe();

    std::cout << "=== All SamplePipeline Tests Passed Successfully! ===" << std::endl;
    return 0;
}