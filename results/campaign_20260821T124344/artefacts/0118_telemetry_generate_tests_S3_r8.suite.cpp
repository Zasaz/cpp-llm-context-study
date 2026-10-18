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

// Helper function to check approximate double equality
bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default constructor and parameterized constructor
void test_constructors_and_defaults() {
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.validSamples() == 0);
    assert(!defaultPipeline.isSaturated());
    assert(defaultPipeline.describe() == "empty");

    SamplePipeline customPipeline("temperature", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.channel.channelName() == "temperature");
    assert(customPipeline.channel.capacity == 5);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.describe() == "empty");

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

// Test filter management: addFilter, filterCount, clearFilters
void test_filter_management() {
    SamplePipeline pipeline("sensor", 10);

    PassFilter pf;
    ClampFilter cf(0.0, 100.0);
    SmoothFilter sf(0.5);

    assert(pipeline.addFilter(nullptr) == 0);
    assert(pipeline.filterCount() == 0);

    assert(pipeline.addFilter(&pf) == 1);
    assert(pipeline.filterCount() == 1);

    assert(pipeline.addFilter(&cf) == 2);
    assert(pipeline.addFilter(&sf) == 3);
    assert(pipeline.filterCount() == 3);

    int clearedCount = pipeline.clearFilters();
    assert(clearedCount == 3);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

// Test processing and stage execution through filters
void test_process_and_stage() {
    SamplePipeline pipeline("sensor", 10);
    ClampFilter cf(0.0, 50.0);
    SmoothFilter sf(1.0); // Smooth with weight=1.0 takes current reading directly

    pipeline.addFilter(&cf);
    pipeline.addFilter(&sf);

    // Reading within clamp range
    double out1 = pipeline.process(30.0);
    assert(approxEqual(out1, 30.0));

    // Reading above clamp range
    double out2 = pipeline.process(80.0);
    assert(approxEqual(out2, 50.0));

    // Test stage individual filter execution
    assert(approxEqual(pipeline.stage(100.0, 0), 50.0));
    assert(approxEqual(pipeline.stage(-10.0, 0), 0.0));
    // Test stage with out-of-bounds indices (returns original reading)
    assert(approxEqual(pipeline.stage(42.0, -1), 42.0));
    assert(approxEqual(pipeline.stage(42.0, 2), 42.0));
    assert(approxEqual(pipeline.stage(42.0, 99), 42.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

// Test wouldReject behavior
void test_wouldReject() {
    SamplePipeline pipeline("sensor", 10);
    ClampFilter cf(10.0, 20.0);

    assert(!pipeline.wouldReject(15.0));

    pipeline.addFilter(&cf);
    assert(!pipeline.wouldReject(15.0));
    assert(pipeline.wouldReject(5.0));
    assert(pipeline.wouldReject(25.0));

    // Disabled filter rejection behavior
    PassFilter pf;
    pf.toggle(); // enabled becomes false
    SamplePipeline pipeline2("sensor", 10);
    pipeline2.addFilter(&pf);
    assert(pipeline2.wouldReject(1.0));
    assert(!pipeline2.wouldReject(0.0)); // Filter::rejects returns false for 0.0

    std::cout << "Test test_wouldReject: PASS" << std::endl;
}

// Test ingest: acceptance, filter modifications, and quality requirements
void test_ingest_and_quality() {
    SamplePipeline pipeline("voltage", 3);
    ClampFilter cf(0.0, 10.0);
    pipeline.addFilter(&cf);

    // Default minimumQuality is 1. Ingest with quality = 0 should be rejected.
    int res0 = pipeline.ingest(1, 5.0, 0);
    assert(res0 == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 1);

    // Valid ingest
    int res1 = pipeline.ingest(2, 5.0, 50);
    assert(res1 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.validSamples() == 1);

    // Test rejection by filter
    int resReject = pipeline.ingest(3, 15.0, 50); // Clamped filter wouldReject returns true
    assert(resReject == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 1);

    // Change minimumQuality
    int prevMin = pipeline.requireQuality(80);
    assert(prevMin == 1);
    assert(pipeline.minimumQuality == 80);

    // Lower quality than required should be rejected
    int resLowQual = pipeline.ingest(4, 5.0, 79);
    assert(resLowQual == -1);
    assert(pipeline.rejectedCount() == 3);

    // Meeting quality requirement
    int resHighQual = pipeline.ingest(5, 8.0, 80);
    assert(resHighQual == 2);
    assert(pipeline.acceptedCount() == 2);

    // Negative requireQuality minimum should be ignored
    pipeline.requireQuality(-5);
    assert(pipeline.minimumQuality == 80);

    std::cout << "Test test_ingest_and_quality: PASS" << std::endl;
}

// Test batch ingestion
void test_ingest_batch() {
    SamplePipeline pipeline("pressure", 5);
    ClampFilter cf(0.0, 100.0);
    pipeline.addFilter(&cf);

    std::vector<double> batch = {10.0, 20.0, -5.0, 40.0, 150.0, 50.0};
    // -5.0 and 150.0 will be rejected by ClampFilter::rejects
    int ingestedCount = pipeline.ingestBatch(batch, 75);

    // Capacity is 5: 10.0 (ok, 1), 20.0 (ok, 2), -5.0 (rej), 40.0 (ok, 3), 150.0 (rej), 50.0 (ok, 4)
    assert(ingestedCount == 4);
    assert(pipeline.acceptedCount() == 4);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 4);

    std::cout << "Test test_ingest_batch: PASS" << std::endl;
}

// Test capacity saturation and dropped samples handling
void test_saturation_and_overflow() {
    SamplePipeline pipeline("flow", 2);

    assert(pipeline.ingest(1, 10.0, 60) == 1);
    assert(!pipeline.isSaturated());
    assert(pipeline.ingest(2, 20.0, 60) == 2);
    assert(pipeline.isSaturated());

    // Next ingest should fail due to channel capacity reached
    int overflowRes = pipeline.ingest(3, 30.0, 60);
    assert(overflowRes == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.channel.droppedCount() == 1);

    std::cout << "Test test_saturation_and_overflow: PASS" << std::endl;
}

// Test statistical methods and latest sample tracking
void test_channel_stats_and_latest() {
    SamplePipeline pipeline("level", 5);

    assert(approxEqual(pipeline.channelMean(), 0.0));
    assert(approxEqual(pipeline.channelPeak(), 0.0));

    pipeline.ingest(1, 10.0, 70);
    pipeline.ingest(2, 30.0, 80);
    pipeline.ingest(3, 20.0, 90);

    assert(approxEqual(pipeline.channelMean(), 20.0));
    assert(approxEqual(pipeline.channelPeak(), 30.0));

    Sample latestSample = pipeline.latest();
    assert(latestSample.position() == 3);
    assert(approxEqual(latestSample.value(), 20.0));
    assert(latestSample.score() == 90);

    std::cout << "Test test_channel_stats_and_latest: PASS" << std::endl;
}

// Test scale normalisation, denormalisation, and describe output
void test_scale_and_describe() {
    SamplePipeline pipeline("current", 5);
    pipeline.scale = UnitScale(2.0, 5.0, "mA"); // toBase: x * 2 + 5; fromBase: (x - 5) / 2

    assert(pipeline.describe() == "empty");

    pipeline.ingest(1, 15.0, 80);

    // Normalised (fromBase)
    assert(approxEqual(pipeline.normalised(25.0), 10.0));
    // Denormalised (toBase)
    assert(approxEqual(pipeline.denormalised(10.0), 25.0));

    // Describe when channel is not empty
    assert(pipeline.describe() == "current:mA");

    std::cout << "Test test_scale_and_describe: PASS" << std::endl;
}

// Test flushing the pipeline
void test_flush() {
    SamplePipeline pipeline("vibration", 5);
    pipeline.ingest(1, 10.0, 70);
    pipeline.ingest(2, 20.0, 80);
    pipeline.ingest(3, 100.0, 0); // rejected

    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.channel.count() == 2);

    int flushedCount = pipeline.flush();
    assert(flushedCount == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.channel.count() == 0);
    assert(pipeline.channel.channelName() == "vibration");
    assert(pipeline.channel.capacity == 5);
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_flush: PASS" << std::endl;
}

int main() {
    std::cout << "=== Running SamplePipeline Test Suite ===" << std::endl;

    test_constructors_and_defaults();
    test_filter_management();
    test_process_and_stage();
    test_wouldReject();
    test_ingest_and_quality();
    test_ingest_batch();
    test_saturation_and_overflow();
    test_channel_stats_and_latest();
    test_scale_and_describe();
    test_flush();

    std::cout << "=== All SamplePipeline Tests Passed Successfully ===" << std::endl;
    return 0;
}