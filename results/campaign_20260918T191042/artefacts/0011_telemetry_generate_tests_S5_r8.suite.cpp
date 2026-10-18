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

// Test constructors and initial state
void test_SamplePipeline_constructors() {
    // Default constructor
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.minimumQuality == 1);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.validSamples() == 0);
    assert(!defaultPipeline.isSaturated());
    assert(defaultPipeline.describe() == "empty");
    assert(defaultPipeline.channel.channelName() == "unnamed");
    assert(defaultPipeline.channel.capacity == 16);

    // Parameterized constructor
    SamplePipeline customPipeline("SensorA", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.minimumQuality == 1);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.validSamples() == 0);
    assert(!customPipeline.isSaturated());
    assert(customPipeline.describe() == "empty");
    assert(customPipeline.channel.channelName() == "SensorA");
    assert(customPipeline.channel.capacity == 5);

    std::cout << "Test test_SamplePipeline_constructors: PASS" << std::endl;
}

// Test filter management: addFilter, filterCount, clearFilters
void test_SamplePipeline_filterManagement() {
    SamplePipeline pipeline("FilterTest", 10);
    assert(pipeline.filterCount() == 0);

    // Adding null filter should be ignored
    int count = pipeline.addFilter(nullptr);
    assert(count == 0);
    assert(pipeline.filterCount() == 0);

    PassFilter pass;
    ClampFilter clamp(0.0, 100.0);
    SmoothFilter smooth(0.5);

    pipeline.addFilter(&pass);
    assert(pipeline.filterCount() == 1);

    pipeline.addFilter(&clamp);
    assert(pipeline.filterCount() == 2);

    pipeline.addFilter(&smooth);
    assert(pipeline.filterCount() == 3);

    // Clear filters
    int cleared = pipeline.clearFilters();
    assert(cleared == 3);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_SamplePipeline_filterManagement: PASS" << std::endl;
}

// Test processing data through filter chain and staging individual filters
void test_SamplePipeline_processAndStage() {
    SamplePipeline pipeline("ProcTest", 10);

    ClampFilter clamp(0.0, 10.0);
    SmoothFilter smooth(0.5); // smoothing weight 0.5

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // Test process() sequentially:
    // First reading: 15.0 -> clamped to 10.0 -> smooth (first reading primes to 10.0) -> 10.0
    double out1 = pipeline.process(15.0);
    assert(isClose(out1, 10.0));

    // Second reading: -5.0 -> clamped to 0.0 -> smooth (0.5 * 0.0 + 0.5 * 10.0 = 5.0) -> 5.0
    double out2 = pipeline.process(-5.0);
    assert(isClose(out2, 5.0));

    // Test stage()
    // Invalid index returns raw reading
    assert(isClose(pipeline.stage(25.0, -1), 25.0));
    assert(isClose(pipeline.stage(25.0, 2), 25.0));

    // Stage 0 is ClampFilter [0, 10]
    assert(isClose(pipeline.stage(25.0, 0), 10.0));
    assert(isClose(pipeline.stage(-8.0, 0), 0.0));

    std::cout << "Test test_SamplePipeline_processAndStage: PASS" << std::endl;
}

// Test rejection logic using wouldReject and minimumQuality checks
void test_SamplePipeline_rejection() {
    SamplePipeline pipeline("RejectTest", 5);
    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);

    assert(!pipeline.wouldReject(50.0));
    assert(pipeline.wouldReject(-5.0));
    assert(pipeline.wouldReject(105.0));

    // Quality threshold tests
    pipeline.requireQuality(50);
    assert(pipeline.minimumQuality == 50);

    // Rejected by quality
    int res1 = pipeline.ingest(1, 50.0, 40);
    assert(res1 == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Rejected by filter bound
    int res2 = pipeline.ingest(2, 120.0, 80);
    assert(res2 == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 0);

    // Accepted
    int res3 = pipeline.ingest(3, 80.0, 60);
    assert(res3 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);

    std::cout << "Test test_SamplePipeline_rejection: PASS" << std::endl;
}

// Test ingesting batches and channel saturation handling
void test_SamplePipeline_ingestAndBatch() {
    SamplePipeline pipeline("BatchTest", 3);
    pipeline.requireQuality(20);

    std::vector<double> batch = { 10.0, 20.0, 30.0, 40.0 };
    // Pipeline capacity is 3. The 4th sample should overflow the channel and be rejected.
    int taken = pipeline.ingestBatch(batch, 50);
    assert(taken == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 1); // 4th rejected due to channel full
    assert(pipeline.isSaturated());
    assert(pipeline.validSamples() == 3);

    // Channel statistics
    assert(isClose(pipeline.channelPeak(), 30.0));
    assert(isClose(pipeline.channelMean(), 20.0));

    // Latest sample inspection
    Sample latestSample = pipeline.latest();
    assert(latestSample.position() == 2);
    assert(isClose(latestSample.value(), 30.0));
    assert(latestSample.score() == 50);

    std::cout << "Test test_SamplePipeline_ingestAndBatch: PASS" << std::endl;
}

// Test UnitScale normalization and pipeline scaling features
void test_SamplePipeline_scalingAndDescription() {
    SamplePipeline pipeline("Voltage", 10);
    pipeline.scale = UnitScale(2.0, 5.0, "V"); // toBase: x*2 + 5; fromBase: (x - 5)/2

    // Normalised: from base scale
    assert(isClose(pipeline.normalised(15.0), 5.0)); // (15 - 5) / 2 = 5
    // Denormalised: to base scale
    assert(isClose(pipeline.denormalised(5.0), 15.0)); // 5 * 2 + 5 = 15

    // Description when empty
    assert(pipeline.describe() == "empty");

    // Ingest a sample
    pipeline.ingest(0, 12.0, 90);
    assert(pipeline.describe() == "Voltage:V");

    std::cout << "Test test_SamplePipeline_scalingAndDescription: PASS" << std::endl;
}

// Test requireQuality behavior with valid and invalid values
void test_SamplePipeline_requireQuality() {
    SamplePipeline pipeline("QualityTest", 10);
    assert(pipeline.minimumQuality == 1);

    int prev = pipeline.requireQuality(75);
    assert(prev == 1);
    assert(pipeline.minimumQuality == 75);

    // Passing negative minimum should be ignored
    prev = pipeline.requireQuality(-10);
    assert(prev == 75);
    assert(pipeline.minimumQuality == 75);

    std::cout << "Test test_SamplePipeline_requireQuality: PASS" << std::endl;
}

// Test flush function to reset pipeline data and statistics
void test_SamplePipeline_flush() {
    SamplePipeline pipeline("ResetTest", 5);
    pipeline.ingest(1, 10.0, 50);
    pipeline.ingest(2, 20.0, 50);
    pipeline.ingest(3, 30.0, 0); // Quality 0 rejected since minQuality is 1

    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.validSamples() == 2);

    int flushedSamples = pipeline.flush();
    assert(flushedSamples == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(!pipeline.isSaturated());
    assert(pipeline.describe() == "empty");
    assert(pipeline.channel.channelName() == "ResetTest");
    assert(pipeline.channel.capacity == 5);

    std::cout << "Test test_SamplePipeline_flush: PASS" << std::endl;
}

int main() {
    std::cout << "Starting SamplePipeline Test Suite..." << std::endl;

    test_SamplePipeline_constructors();
    test_SamplePipeline_filterManagement();
    test_SamplePipeline_processAndStage();
    test_SamplePipeline_rejection();
    test_SamplePipeline_ingestAndBatch();
    test_SamplePipeline_scalingAndDescription();
    test_SamplePipeline_requireQuality();
    test_SamplePipeline_flush();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}