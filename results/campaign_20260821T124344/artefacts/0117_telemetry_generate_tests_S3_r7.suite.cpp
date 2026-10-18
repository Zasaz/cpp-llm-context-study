#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>

#include "SamplePipeline.h"
#include "Channel.h"
#include "Filter.h"
#include "Sample.h"
#include "UnitScale.h"

// Helper function for floating point comparisons
bool double_equals(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_SamplePipeline_constructors() {
    // Default constructor
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.minimumQuality == 1);
    assert(defaultPipeline.channel.channelName() == "unnamed");
    assert(defaultPipeline.channel.capacity == 16);
    assert(defaultPipeline.validSamples() == 0);
    assert(!defaultPipeline.isSaturated());

    // Parameterized constructor
    SamplePipeline customPipeline("SensorAlpha", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.minimumQuality == 1);
    assert(customPipeline.channel.channelName() == "SensorAlpha");
    assert(customPipeline.channel.capacity == 5);

    std::cout << "Test test_SamplePipeline_constructors: PASS" << std::endl;
}

void test_SamplePipeline_filterManagement() {
    SamplePipeline pipeline("Pipeline", 10);
    assert(pipeline.filterCount() == 0);

    // Adding null filter should be ignored
    int countAfterNull = pipeline.addFilter(nullptr);
    assert(countAfterNull == 0);
    assert(pipeline.filterCount() == 0);

    PassFilter pf;
    ClampFilter cf(0.0, 10.0);
    SmoothFilter sf(0.5);

    assert(pipeline.addFilter(&pf) == 1);
    assert(pipeline.addFilter(&cf) == 2);
    assert(pipeline.addFilter(&sf) == 3);
    assert(pipeline.filterCount() == 3);

    // Clear filters
    int previousCount = pipeline.clearFilters();
    assert(previousCount == 3);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_SamplePipeline_filterManagement: PASS" << std::endl;
}

void test_SamplePipeline_processAndStage() {
    SamplePipeline pipeline("ProcessTest", 10);
    ClampFilter clamp(0.0, 100.0);
    SmoothFilter smooth(0.5);

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // Test stage with invalid indices
    assert(double_equals(pipeline.stage(50.0, -1), 50.0));
    assert(double_equals(pipeline.stage(50.0, 2), 50.0));

    // Test stage execution for individual filter (Clamp)
    assert(double_equals(pipeline.stage(150.0, 0), 100.0));
    assert(double_equals(pipeline.stage(-20.0, 0), 0.0));

    // Reset smooth filter state and test stage (Smooth)
    // First smooth apply primes it to 20.0
    assert(double_equals(pipeline.stage(20.0, 1), 20.0));
    // Second smooth apply: 0.5 * 40.0 + 0.5 * 20.0 = 30.0
    assert(double_equals(pipeline.stage(40.0, 1), 30.0));

    // Test full process chain
    pipeline.clearFilters();
    ClampFilter clampChain(0.0, 50.0);
    pipeline.addFilter(&clampChain);

    double result = pipeline.process(75.0);
    assert(double_equals(result, 50.0));

    result = pipeline.process(-10.0);
    assert(double_equals(result, 0.0));

    result = pipeline.process(25.0);
    assert(double_equals(result, 25.0));

    std::cout << "Test test_SamplePipeline_processAndStage: PASS" << std::endl;
}

void test_SamplePipeline_wouldReject() {
    SamplePipeline pipeline("RejectTest", 10);
    ClampFilter clamp(10.0, 20.0);
    pipeline.addFilter(&clamp);

    assert(pipeline.wouldReject(5.0) == true);
    assert(pipeline.wouldReject(25.0) == true);
    assert(pipeline.wouldReject(15.0) == false);

    // Test disabled filter rejection behavior
    PassFilter pass;
    pass.toggle(); // enabled becomes false
    SamplePipeline disabledPipeline("DisabledTest", 10);
    disabledPipeline.addFilter(&pass);
    assert(disabledPipeline.wouldReject(1.0) == true);
    assert(disabledPipeline.wouldReject(0.0) == false);

    std::cout << "Test test_SamplePipeline_wouldReject: PASS" << std::endl;
}

void test_SamplePipeline_ingest_and_quality() {
    SamplePipeline pipeline("IngestTest", 3);
    assert(pipeline.requireQuality(50) == 1); // Previous minimum was 1
    assert(pipeline.minimumQuality == 50);

    // Negative requirement should not change minimumQuality
    pipeline.requireQuality(-5);
    assert(pipeline.minimumQuality == 50);

    // Reject due to low quality
    int res1 = pipeline.ingest(1, 10.0, 40);
    assert(res1 == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 1);

    // Ingest valid sample
    int res2 = pipeline.ingest(2, 15.0, 60);
    assert(res2 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 1);
    assert(double_equals(pipeline.latest().value(), 15.0));
    assert(pipeline.latest().position() == 2);

    // Filter rejection check
    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);
    int res3 = pipeline.ingest(3, 150.0, 80); // clamp rejects > 100.0
    assert(res3 == -1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);

    // Fill capacity
    pipeline.ingest(4, 20.0, 70);
    pipeline.ingest(5, 30.0, 90);
    assert(pipeline.isSaturated());
    assert(pipeline.acceptedCount() == 3);

    // Reject because channel capacity is full
    int resOverflow = pipeline.ingest(6, 25.0, 80);
    assert(resOverflow == -1);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 3);

    std::cout << "Test test_SamplePipeline_ingest_and_quality: PASS" << std::endl;
}

void test_SamplePipeline_ingestBatch() {
    SamplePipeline pipeline("BatchTest", 5);
    ClampFilter clamp(0.0, 50.0);
    pipeline.addFilter(&clamp);

    std::vector<double> batch = { 10.0, 20.0, 60.0, 30.0, 40.0, 50.0 };
    // 60.0 will be rejected by clamp
    // The rest (10, 20, 30, 40) take 4 slots
    // 50.0 will be rejected due to capacity limit (capacity is 5, but after 4 valid samples it takes the 5th)
    int acceptedBatch = pipeline.ingestBatch(batch, 80);
    assert(acceptedBatch == 4);
    assert(pipeline.acceptedCount() == 4);
    assert(pipeline.rejectedCount() == 2); // 60.0 rejected, and 6th element would overflow or not reached

    std::cout << "Test test_SamplePipeline_ingestBatch: PASS" << std::endl;
}

void test_SamplePipeline_statistics_and_scaling() {
    SamplePipeline pipeline("StatsTest", 10);
    pipeline.scale = UnitScale(2.0, 5.0, "mV"); // base = reading * 2 + 5

    pipeline.ingest(0, 10.0, 100);
    pipeline.ingest(1, 20.0, 100);
    pipeline.ingest(2, 30.0, 100);

    assert(double_equals(pipeline.channelMean(), 20.0));
    assert(double_equals(pipeline.channelPeak(), 30.0));
    assert(pipeline.validSamples() == 3);

    // Normalised / Denormalised
    // fromBase(25.0) -> (25 - 5) / 2 = 10
    assert(double_equals(pipeline.normalised(25.0), 10.0));
    // toBase(10.0) -> 10 * 2 + 5 = 25
    assert(double_equals(pipeline.denormalised(10.0), 25.0));

    std::cout << "Test test_SamplePipeline_statistics_and_scaling: PASS" << std::endl;
}

void test_SamplePipeline_flush_and_describe() {
    SamplePipeline pipeline("DescribeTest", 5);
    pipeline.scale = UnitScale(1.0, 0.0, "V");

    // Empty describe
    assert(pipeline.describe() == "empty");

    pipeline.ingest(0, 12.5, 90);
    assert(pipeline.describe() == "DescribeTest:V");

    // Flush
    int flushedCount = pipeline.flush();
    assert(flushedCount == 1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.channel.isEmpty());
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_SamplePipeline_flush_and_describe: PASS" << std::endl;
}

int main() {
    std::cout << "Starting SamplePipeline Test Suite..." << std::endl;

    test_SamplePipeline_constructors();
    test_SamplePipeline_filterManagement();
    test_SamplePipeline_processAndStage();
    test_SamplePipeline_wouldReject();
    test_SamplePipeline_ingest_and_quality();
    test_SamplePipeline_ingestBatch();
    test_SamplePipeline_statistics_and_scaling();
    test_SamplePipeline_flush_and_describe();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}