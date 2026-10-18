#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>

#include "SamplePipeline.h"
#include "Filter.h"
#include "Sample.h"
#include "Channel.h"
#include "UnitScale.h"

// Helper for floating-point comparison
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) <= epsilon;
}

// 1. Test Default and Parameterized Constructors
void test_constructors() {
    // Default constructor
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.minimumQuality == 1);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.channel.channelName() == "unnamed");
    assert(defaultPipeline.channel.capacity == 16);
    assert(defaultPipeline.channel.isEmpty());

    // Parameterized constructor
    SamplePipeline customPipeline("temperature", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.minimumQuality == 1);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.channel.channelName() == "temperature");
    assert(customPipeline.channel.capacity == 5);
    assert(customPipeline.channel.isEmpty());

    std::cout << "Test test_constructors: PASS" << std::endl;
}

// 2. Test Filter Management (addFilter, filterCount, clearFilters)
void test_filterManagement() {
    SamplePipeline pipeline("sensor", 10);
    assert(pipeline.filterCount() == 0);

    // Adding null filter should be ignored
    int count = pipeline.addFilter(nullptr);
    assert(count == 0);
    assert(pipeline.filterCount() == 0);

    PassFilter pf;
    ClampFilter cf(0.0, 100.0);
    SmoothFilter sf(0.5);

    assert(pipeline.addFilter(&pf) == 1);
    assert(pipeline.addFilter(&cf) == 2);
    assert(pipeline.addFilter(&sf) == 3);
    assert(pipeline.filterCount() == 3);

    // Clear filters
    int clearedCount = pipeline.clearFilters();
    assert(clearedCount == 3);
    assert(pipeline.filterCount() == 0);

    // Clear again on empty
    assert(pipeline.clearFilters() == 0);

    std::cout << "Test test_filterManagement: PASS" << std::endl;
}

// 3. Test Process and Stage Filtering
void test_processAndStage() {
    SamplePipeline pipeline;
    ClampFilter clamp(0.0, 50.0);
    SmoothFilter smooth(0.5); // previous starts at 0, unprimed

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // Test stage with out-of-bounds indices
    assert(approxEqual(pipeline.stage(100.0, -1), 100.0));
    assert(approxEqual(pipeline.stage(100.0, 2), 100.0));

    // Test single stage (clamp)
    // reading 100 clamped to 50.0
    double clampedVal = pipeline.stage(100.0, 0);
    assert(approxEqual(clampedVal, 50.0));

    // Test pipeline process
    // First reading: 100.0 -> clamped to 50.0 -> smooth (primed with 50.0) -> 50.0
    double out1 = pipeline.process(100.0);
    assert(approxEqual(out1, 50.0));

    // Second reading: 20.0 -> clamped to 20.0 -> smooth(0.5*20.0 + 0.5*50.0) = 35.0
    double out2 = pipeline.process(20.0);
    assert(approxEqual(out2, 35.0));

    std::cout << "Test test_processAndStage: PASS" << std::endl;
}

// 4. Test Ingestion Logic (Quality, Filter Rejection, Capacity)
void test_ingestion() {
    SamplePipeline pipeline("volt", 2);
    pipeline.requireQuality(50); // Minimum quality set to 50

    ClampFilter cf(0.0, 10.0);
    pipeline.addFilter(&cf);

    // 1. Rejected due to quality too low
    int res1 = pipeline.ingest(1, 5.0, 40);
    assert(res1 == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // 2. Rejected due to filter rejects (value 15.0 is out of [0, 10])
    int res2 = pipeline.ingest(2, 15.0, 80);
    assert(res2 == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 0);

    // 3. Successfully accepted
    int res3 = pipeline.ingest(3, 8.0, 80);
    assert(res3 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 1);

    // 4. Successfully accepted second item (capacity is 2)
    int res4 = pipeline.ingest(4, 4.0, 90);
    assert(res4 == 2);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.isSaturated());

    // 5. Rejected due to channel capacity full
    int res5 = pipeline.ingest(5, 6.0, 90);
    assert(res5 == -1);
    assert(pipeline.rejectedCount() == 3);
    assert(pipeline.acceptedCount() == 2);

    std::cout << "Test test_ingestion: PASS" << std::endl;
}

// 5. Test Batch Ingestion
void test_ingestBatch() {
    SamplePipeline pipeline("batch", 5);
    pipeline.requireQuality(10);

    ClampFilter cf(-5.0, 5.0);
    pipeline.addFilter(&cf);

    std::vector<double> readings = {-2.0, 10.0, 3.0, -10.0, 0.0};
    // Expected:
    // -2.0 -> accepted
    // 10.0 -> rejected (out of bounds)
    //  3.0 -> accepted
    // -10.0 -> rejected (out of bounds)
    //  0.0 -> accepted
    int acceptedCount = pipeline.ingestBatch(readings, 50);

    assert(acceptedCount == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 3);

    std::cout << "Test test_ingestBatch: PASS" << std::endl;
}

// 6. Test Quality Requirement Configuration
void test_requireQuality() {
    SamplePipeline pipeline;
    assert(pipeline.minimumQuality == 1);

    int prev = pipeline.requireQuality(75);
    assert(prev == 1);
    assert(pipeline.minimumQuality == 75);

    // Negative requirement should be ignored
    prev = pipeline.requireQuality(-5);
    assert(prev == 75);
    assert(pipeline.minimumQuality == 75);

    std::cout << "Test test_requireQuality: PASS" << std::endl;
}

// 7. Test Channel Statistics (channelMean, channelPeak, latest)
void test_channelStatistics() {
    SamplePipeline pipeline("stats", 4);
    assert(approxEqual(pipeline.channelMean(), 0.0));
    assert(approxEqual(pipeline.channelPeak(), 0.0));

    pipeline.ingest(1, 10.0, 100);
    pipeline.ingest(2, 30.0, 100);
    pipeline.ingest(3, 20.0, 100);

    assert(approxEqual(pipeline.channelMean(), 20.0));
    assert(approxEqual(pipeline.channelPeak(), 30.0));

    Sample latest = pipeline.latest();
    assert(latest.position() == 3);
    assert(approxEqual(latest.value(), 20.0));
    assert(latest.score() == 100);

    std::cout << "Test test_channelStatistics: PASS" << std::endl;
}

// 8. Test Normalisation and Denormalisation with UnitScale
void test_scaling() {
    SamplePipeline pipeline;
    // factor = 2.0, offset = 10.0 -> toBase(x) = x * 2.0 + 10.0; fromBase(x) = (x - 10.0) / 2.0
    pipeline.scale = UnitScale(2.0, 10.0, "volt");

    double rawVal = 5.0;
    double denorm = pipeline.denormalised(rawVal); // toBase: 5 * 2 + 10 = 20
    assert(approxEqual(denorm, 20.0));

    double norm = pipeline.normalised(20.0); // fromBase: (20 - 10) / 2 = 5
    assert(approxEqual(norm, 5.0));

    std::cout << "Test test_scaling: PASS" << std::endl;
}

// 9. Test Pipeline Flush
void test_flush() {
    SamplePipeline pipeline("ch1", 3);
    pipeline.ingest(1, 10.0, 100);
    pipeline.ingest(2, 20.0, 100);
    pipeline.ingest(3, 1000.0, 0); // rejected due to quality < 1

    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.channel.count() == 2);

    int flushed = pipeline.flush();
    assert(flushed == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.channel.isEmpty());
    assert(pipeline.channel.capacity == 3);
    assert(pipeline.channel.channelName() == "ch1");

    std::cout << "Test test_flush: PASS" << std::endl;
}

// 10. Test Describe Method
void test_describe() {
    SamplePipeline pipeline("pressure", 5);
    pipeline.scale = UnitScale(1.0, 0.0, "kPa");

    // When empty
    assert(pipeline.describe() == "empty");

    // After adding sample
    pipeline.ingest(1, 101.3, 100);
    assert(pipeline.describe() == "pressure:kPa");

    std::cout << "Test test_describe: PASS" << std::endl;
}

int main() {
    std::cout << "Starting SamplePipeline Test Suite..." << std::endl;

    test_constructors();
    test_filterManagement();
    test_processAndStage();
    test_ingestion();
    test_ingestBatch();
    test_requireQuality();
    test_channelStatistics();
    test_scaling();
    test_flush();
    test_describe();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}