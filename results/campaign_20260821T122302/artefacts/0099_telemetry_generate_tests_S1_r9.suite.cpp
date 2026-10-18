#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>

#include "SamplePipeline.h"
#include "Filter.h"
#include "Sample.h"
#include "Channel.h"
#include "UnitScale.h"

// Helper function for floating point comparisons
bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

void test_SamplePipeline_constructorsAndDefaults() {
    // Default constructor
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.validSamples() == 0);
    assert(!defaultPipeline.isSaturated());
    assert(defaultPipeline.describe() == "empty");
    assert(defaultPipeline.channel.channelName() == "unnamed");
    assert(defaultPipeline.channel.capacity == 16);

    // Parameterized constructor
    SamplePipeline customPipeline("sensor_temp", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.channel.channelName() == "sensor_temp");
    assert(customPipeline.channel.capacity == 5);
    assert(!customPipeline.isSaturated());

    std::cout << "Test test_SamplePipeline_constructorsAndDefaults: PASS" << std::endl;
}

void test_SamplePipeline_filterManagement() {
    SamplePipeline pipeline("ch1", 10);

    ClampFilter clamp(0.0, 10.0);
    PassFilter pass;

    // Add valid filters
    assert(pipeline.addFilter(&clamp) == 1);
    assert(pipeline.filterCount() == 1);
    assert(pipeline.addFilter(&pass) == 2);
    assert(pipeline.filterCount() == 2);

    // Adding nullptr should not change count
    assert(pipeline.addFilter(nullptr) == 2);
    assert(pipeline.filterCount() == 2);

    // Clear filters
    int removed = pipeline.clearFilters();
    assert(removed == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_SamplePipeline_filterManagement: PASS" << std::endl;
}

void test_SamplePipeline_processAndStage() {
    SamplePipeline pipeline("ch1", 10);

    ClampFilter clamp(0.0, 50.0);
    SmoothFilter smooth(0.5);

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // Stage out of range checks
    assert(approxEqual(pipeline.stage(100.0, -1), 100.0));
    assert(approxEqual(pipeline.stage(100.0, 5), 100.0));

    // Stage individual evaluation
    // Stage 0 is clamp: 100.0 -> 50.0
    assert(approxEqual(pipeline.stage(100.0, 0), 50.0));

    // Testing pipeline process (multi-stage)
    // First value: 100.0 -> clamped to 50.0 -> smooth unprimed -> 50.0
    double res1 = pipeline.process(100.0);
    assert(approxEqual(res1, 50.0));

    // Second value: 10.0 -> clamped to 10.0 -> smooth(0.5 * 10 + 0.5 * 50) = 30.0
    double res2 = pipeline.process(10.0);
    assert(approxEqual(res2, 30.0));

    std::cout << "Test test_SamplePipeline_processAndStage: PASS" << std::endl;
}

void test_SamplePipeline_wouldReject() {
    SamplePipeline pipeline("ch1", 10);

    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);

    // ClampFilter rejects values < 0 or > 100
    assert(!pipeline.wouldReject(50.0));
    assert(pipeline.wouldReject(-1.0));
    assert(pipeline.wouldReject(101.0));

    // Disable a generic filter to test Filter::rejects
    Filter genericFilter;
    pipeline.addFilter(&genericFilter);
    assert(!pipeline.wouldReject(50.0));
    genericFilter.toggle(); // now disabled
    assert(pipeline.wouldReject(50.0));
    // Disabled filter rejects any non-zero value, but allows 0.0
    assert(!genericFilter.rejects(0.0));

    std::cout << "Test test_SamplePipeline_wouldReject: PASS" << std::endl;
}

void test_SamplePipeline_requireQualityAndIngestRejection() {
    SamplePipeline pipeline("ch1", 10);
    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);

    // Default min quality is 1
    int prevMin = pipeline.requireQuality(50);
    assert(prevMin == 1);

    // Quality below threshold (< 50)
    int res = pipeline.ingest(0, 75.0, 49);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Filter rejection (out of clamp bounds)
    res = pipeline.ingest(1, 150.0, 80);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 0);

    // Negative quality setting ignored
    pipeline.requireQuality(-5);
    // Min quality remains 50
    assert(pipeline.ingest(2, 75.0, 40) == -1);
    assert(pipeline.rejectedCount() == 3);

    // Valid ingest
    res = pipeline.ingest(3, 75.0, 80);
    assert(res == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 3);

    std::cout << "Test test_SamplePipeline_requireQualityAndIngestRejection: PASS" << std::endl;
}

void test_SamplePipeline_ingestAndBatch() {
    SamplePipeline pipeline("pressure", 5);
    pipeline.requireQuality(10);

    std::vector<double> batch = {10.0, 20.0, 30.0};
    int taken = pipeline.ingestBatch(batch, 80);
    assert(taken == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.validSamples() == 3);
    assert(approxEqual(pipeline.channelMean(), 20.0));
    assert(approxEqual(pipeline.channelPeak(), 30.0));

    // Ingest until saturated
    assert(!pipeline.isSaturated());
    assert(pipeline.ingest(3, 40.0, 80) == 4);
    assert(pipeline.ingest(4, 50.0, 80) == 5);
    assert(pipeline.isSaturated());

    // Next ingest should fail due to channel capacity limit
    int overflowRes = pipeline.ingest(5, 60.0, 80);
    assert(overflowRes == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 5);

    // Latest sample check
    Sample latestSample = pipeline.latest();
    assert(latestSample.position() == 4);
    assert(approxEqual(latestSample.value(), 50.0));
    assert(latestSample.score() == 80);

    std::cout << "Test test_SamplePipeline_ingestAndBatch: PASS" << std::endl;
}

void test_SamplePipeline_scalingAndNormalization() {
    SamplePipeline pipeline("voltage", 10);
    pipeline.scale = UnitScale(2.0, 10.0, "V"); // toBase: x * 2 + 10, fromBase: (x - 10) / 2

    // normalised (fromBase)
    double norm = pipeline.normalised(30.0);
    assert(approxEqual(norm, 10.0)); // (30 - 10) / 2 = 10

    // denormalised (toBase)
    double denorm = pipeline.denormalised(10.0);
    assert(approxEqual(denorm, 30.0)); // 10 * 2 + 10 = 30

    std::cout << "Test test_SamplePipeline_scalingAndNormalization: PASS" << std::endl;
}

void test_SamplePipeline_flushAndDescribe() {
    SamplePipeline pipeline("temp", 5);
    pipeline.scale = UnitScale(1.0, 0.0, "C");

    // Initially empty
    assert(pipeline.describe() == "empty");

    pipeline.ingest(1, 25.0, 90);
    assert(pipeline.acceptedCount() == 1);
    assert(!pipeline.channel.isEmpty());
    assert(pipeline.describe() == "temp:C");

    // Flush
    int flushedCount = pipeline.flush();
    assert(flushedCount == 1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.channel.isEmpty());
    assert(pipeline.describe() == "empty");
    // Verify channel capacity is preserved
    assert(pipeline.channel.capacity == 5);
    assert(pipeline.channel.channelName() == "temp");

    std::cout << "Test test_SamplePipeline_flushAndDescribe: PASS" << std::endl;
}

int main() {
    std::cout << "Starting SamplePipeline Test Suite..." << std::endl;

    test_SamplePipeline_constructorsAndDefaults();
    test_SamplePipeline_filterManagement();
    test_SamplePipeline_processAndStage();
    test_SamplePipeline_wouldReject();
    test_SamplePipeline_requireQualityAndIngestRejection();
    test_SamplePipeline_ingestAndBatch();
    test_SamplePipeline_scalingAndNormalization();
    test_SamplePipeline_flushAndDescribe();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}