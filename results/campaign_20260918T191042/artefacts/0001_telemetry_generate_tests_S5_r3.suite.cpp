#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>

#include "SamplePipeline.h"
#include "Channel.h"
#include "Filter.h"
#include "Sample.h"
#include "UnitScale.h"

// Helper function for floating point comparisons
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) <= epsilon;
}

void test_constructors_and_defaults() {
    // Test default constructor
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.minimumQuality == 1);
    assert(defaultPipeline.channel.channelName() == "unnamed");
    assert(defaultPipeline.channel.capacity == 16);
    assert(defaultPipeline.validSamples() == 0);
    assert(!defaultPipeline.isSaturated());

    // Test parameterized constructor
    SamplePipeline customPipeline("temp_sensor", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.minimumQuality == 1);
    assert(customPipeline.channel.channelName() == "temp_sensor");
    assert(customPipeline.channel.capacity == 5);
    assert(!customPipeline.isSaturated());

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline("ch1", 10);
    PassFilter pFilter;
    ClampFilter cFilter(0.0, 100.0);

    // Adding null filter should be ignored
    assert(pipeline.addFilter(nullptr) == 0);
    assert(pipeline.filterCount() == 0);

    // Adding valid filters
    assert(pipeline.addFilter(&pFilter) == 1);
    assert(pipeline.filterCount() == 1);

    assert(pipeline.addFilter(&cFilter) == 2);
    assert(pipeline.filterCount() == 2);

    // Clearing filters
    int cleared = pipeline.clearFilters();
    assert(cleared == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_filter_processing_and_staging() {
    SamplePipeline pipeline("ch1", 10);
    ClampFilter cFilter(0.0, 50.0);
    SmoothFilter sFilter(0.5); // previous starts at 0, unprimed

    pipeline.addFilter(&cFilter);
    pipeline.addFilter(&sFilter);

    // Test staging (applying single filter individually)
    // Stage 0: clamp
    double clamped = pipeline.stage(100.0, 0);
    assert(approxEqual(clamped, 50.0));

    // Invalid index returns raw reading unchanged
    assert(approxEqual(pipeline.stage(100.0, -1), 100.0));
    assert(approxEqual(pipeline.stage(100.0, 2), 100.0));

    // Test pipeline processing (passes through cFilter then sFilter)
    // First value 80.0 -> clamped to 50.0 -> sFilter primes with 50.0 -> result 50.0
    double processed1 = pipeline.process(80.0);
    assert(approxEqual(processed1, 50.0));

    // Second value 0.0 -> clamped to 0.0 -> sFilter computes 0.5 * 0.0 + 0.5 * 50.0 = 25.0
    double processed2 = pipeline.process(0.0);
    assert(approxEqual(processed2, 25.0));

    std::cout << "Test test_filter_processing_and_staging: PASS" << std::endl;
}

void test_rejection_rules() {
    SamplePipeline pipeline("ch1", 10);
    ClampFilter cFilter(10.0, 20.0);
    pipeline.addFilter(&cFilter);

    // wouldReject checks filter rejects logic
    assert(pipeline.wouldReject(5.0) == true);
    assert(pipeline.wouldReject(25.0) == true);
    assert(pipeline.wouldReject(15.0) == false);

    // Test rejection due to wouldReject
    int res1 = pipeline.ingest(1, 25.0, 80);
    assert(res1 == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Test rejection due to quality < minimumQuality
    pipeline.requireQuality(50);
    int res2 = pipeline.ingest(2, 15.0, 40);
    assert(res2 == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 0);

    // Test successful ingest
    int res3 = pipeline.ingest(3, 15.0, 60);
    assert(res3 == 1); // returns accepted count
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);

    std::cout << "Test test_rejection_rules: PASS" << std::endl;
}

void test_require_quality() {
    SamplePipeline pipeline("ch1", 10);
    assert(pipeline.minimumQuality == 1);

    int prev = pipeline.requireQuality(75);
    assert(prev == 1);
    assert(pipeline.minimumQuality == 75);

    // Negative requirement should be ignored
    prev = pipeline.requireQuality(-5);
    assert(prev == 75);
    assert(pipeline.minimumQuality == 75);

    std::cout << "Test test_require_quality: PASS" << std::endl;
}

void test_ingest_batch_and_capacity_limits() {
    // Capacity 3 pipeline
    SamplePipeline pipeline("ch_batch", 3);
    PassFilter pFilter;
    pipeline.addFilter(&pFilter);

    std::vector<double> batch = { 10.0, 20.0, 30.0, 40.0, 50.0 };
    int taken = pipeline.ingestBatch(batch, 80);

    // Only 3 should be accepted because capacity is 3
    assert(taken == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 2); // 40.0 and 50.0 rejected due to channel full
    assert(pipeline.isSaturated() == true);
    assert(pipeline.validSamples() == 3);

    // Latest sample should be sequence 2 (reading 30.0)
    Sample latest = pipeline.latest();
    assert(latest.position() == 2);
    assert(approxEqual(latest.value(), 30.0));

    std::cout << "Test test_ingest_batch_and_capacity_limits: PASS" << std::endl;
}

void test_channel_statistics() {
    SamplePipeline pipeline("stats_ch", 10);

    assert(approxEqual(pipeline.channelMean(), 0.0));
    assert(approxEqual(pipeline.channelPeak(), 0.0));

    pipeline.ingest(0, 10.0, 90);
    pipeline.ingest(1, 30.0, 90);
    pipeline.ingest(2, 20.0, 90);

    assert(approxEqual(pipeline.channelMean(), 20.0));
    assert(approxEqual(pipeline.channelPeak(), 30.0));

    std::cout << "Test test_channel_statistics: PASS" << std::endl;
}

void test_scaling_and_normalization() {
    SamplePipeline pipeline("scaled_ch", 10);
    pipeline.scale = UnitScale(2.0, 5.0, "mV"); // toBase: x * 2 + 5, fromBase: (x - 5) / 2

    double baseVal = pipeline.denormalised(10.0); // 10 * 2 + 5 = 25
    assert(approxEqual(baseVal, 25.0));

    double normVal = pipeline.normalised(25.0); // (25 - 5) / 2 = 10
    assert(approxEqual(normVal, 10.0));

    std::cout << "Test test_scaling_and_normalization: PASS" << std::endl;
}

void test_flush_and_describe() {
    SamplePipeline pipeline("sensorA", 4);
    pipeline.scale = UnitScale(1.0, 0.0, "Celsius");

    assert(pipeline.describe() == "empty");

    pipeline.ingest(0, 22.5, 95);
    pipeline.ingest(1, 23.0, 95);

    assert(pipeline.describe() == "sensorA:Celsius");
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.validSamples() == 2);

    int flushedCount = pipeline.flush();
    assert(flushedCount == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.channel.isEmpty());
    assert(pipeline.describe() == "empty");
    assert(pipeline.channel.capacity == 4);
    assert(pipeline.channel.channelName() == "sensorA");

    std::cout << "Test test_flush_and_describe: PASS" << std::endl;
}

int main() {
    std::cout << "Starting SamplePipeline Test Suite..." << std::endl;

    test_constructors_and_defaults();
    test_filter_management();
    test_filter_processing_and_staging();
    test_rejection_rules();
    test_require_quality();
    test_ingest_batch_and_capacity_limits();
    test_channel_statistics();
    test_scaling_and_normalization();
    test_flush_and_describe();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}