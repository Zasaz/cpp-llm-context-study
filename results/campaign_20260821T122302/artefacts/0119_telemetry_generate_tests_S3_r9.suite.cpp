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

// Helper function for floating point comparison
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_default_constructor_and_initial_state() {
    SamplePipeline pipeline;

    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.filterCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(!pipeline.isSaturated());
    assert(pipeline.channel.channelName() == "unnamed");
    assert(pipeline.channel.capacity == 16);
    assert(pipeline.minimumQuality == 1);
    assert(pipeline.describe() == "empty");
    assert(approxEqual(pipeline.channelMean(), 0.0));
    assert(approxEqual(pipeline.channelPeak(), 0.0));

    std::cout << "Test test_default_constructor_and_initial_state: PASS" << std::endl;
}

void test_parameterized_constructor() {
    SamplePipeline pipeline("Temperature", 5);

    assert(pipeline.channel.channelName() == "Temperature");
    assert(pipeline.channel.capacity == 5);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.filterCount() == 0);
    assert(pipeline.minimumQuality == 1);
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_parameterized_constructor: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline("Voltage", 10);
    PassFilter pFilter;
    ClampFilter cFilter(0.0, 10.0);

    // Adding null filter should be ignored
    int countAfterNull = pipeline.addFilter(nullptr);
    assert(countAfterNull == 0);
    assert(pipeline.filterCount() == 0);

    // Adding valid filters
    int count1 = pipeline.addFilter(&pFilter);
    assert(count1 == 1);
    assert(pipeline.filterCount() == 1);

    int count2 = pipeline.addFilter(&cFilter);
    assert(count2 == 2);
    assert(pipeline.filterCount() == 2);

    // Clearing filters
    int clearedCount = pipeline.clearFilters();
    assert(clearedCount == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipeline;
    ClampFilter clamp(0.0, 100.0);
    SmoothFilter smooth(0.5);

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // Test stage with invalid indices (should return original reading)
    assert(approxEqual(pipeline.stage(150.0, -1), 150.0));
    assert(approxEqual(pipeline.stage(150.0, 2), 150.0));

    // Test individual stage: stage 0 is clamp
    double clamped = pipeline.stage(150.0, 0);
    assert(approxEqual(clamped, 100.0));

    // Reset filters for process pipeline test
    pipeline.clearFilters();
    ClampFilter c(0.0, 50.0);
    SmoothFilter s(0.5); // first value primes to itself
    pipeline.addFilter(&c);
    pipeline.addFilter(&s);

    // First process: reading 80 clamped to 50 -> smooth primed to 50
    double out1 = pipeline.process(80.0);
    assert(approxEqual(out1, 50.0));

    // Second process: reading 10 clamped to 10 -> smooth = 0.5 * 10 + 0.5 * 50 = 30
    double out2 = pipeline.process(10.0);
    assert(approxEqual(out2, 30.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_wouldReject() {
    SamplePipeline pipeline;
    ClampFilter clamp(0.0, 10.0);
    pipeline.addFilter(&clamp);

    assert(!pipeline.wouldReject(5.0));
    assert(pipeline.wouldReject(15.0));
    assert(pipeline.wouldReject(-1.0));

    std::cout << "Test test_wouldReject: PASS" << std::endl;
}

void test_ingest_basic_and_filters() {
    SamplePipeline pipeline("Sensor", 5);
    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);

    // Successful ingest
    int res1 = pipeline.ingest(1, 50.0, 80);
    assert(res1 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 1);
    assert(approxEqual(pipeline.latest().value(), 50.0));

    // Ingest with value transformed by filter (e.g. 150 -> clamped to 100)
    // Note: ClampFilter::rejects rejects > high, so wouldReject(150) is true
    int res2 = pipeline.ingest(2, 150.0, 80);
    assert(res2 == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 1);

    std::cout << "Test test_ingest_basic_and_filters: PASS" << std::endl;
}

void test_quality_requirements_and_rejection() {
    SamplePipeline pipeline("QualityCheck", 10);

    // Initial minimum quality is 1
    assert(pipeline.minimumQuality == 1);

    // Quality 0 should be rejected by minimumQuality check
    int res1 = pipeline.ingest(0, 10.0, 0);
    assert(res1 == -1);
    assert(pipeline.rejectedCount() == 1);

    // Change minimum quality
    int prevMin = pipeline.requireQuality(60);
    assert(prevMin == 1);
    assert(pipeline.minimumQuality == 60);

    // Negative requireQuality should not change setting
    pipeline.requireQuality(-5);
    assert(pipeline.minimumQuality == 60);

    // Ingest with quality 50 (< 60) should fail
    int res2 = pipeline.ingest(1, 20.0, 50);
    assert(res2 == -1);
    assert(pipeline.rejectedCount() == 2);

    // Ingest with quality 70 (>= 60) should succeed
    int res3 = pipeline.ingest(2, 20.0, 70);
    assert(res3 == 1);
    assert(pipeline.acceptedCount() == 1);

    std::cout << "Test test_quality_requirements_and_rejection: PASS" << std::endl;
}

void test_capacity_and_saturation() {
    SamplePipeline pipeline("SmallChannel", 2);

    assert(!pipeline.isSaturated());

    pipeline.ingest(0, 10.0, 80);
    assert(!pipeline.isSaturated());

    pipeline.ingest(1, 20.0, 80);
    assert(pipeline.isSaturated());

    // Ingest beyond capacity
    int resOver = pipeline.ingest(2, 30.0, 80);
    assert(resOver == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.isSaturated());

    std::cout << "Test test_capacity_and_saturation: PASS" << std::endl;
}

void test_ingestBatch() {
    SamplePipeline pipeline("BatchChannel", 5);
    std::vector<double> data = { 10.0, 20.0, 30.0, 40.0 };

    int acceptedCount = pipeline.ingestBatch(data, 80);
    assert(acceptedCount == 4);
    assert(pipeline.acceptedCount() == 4);
    assert(pipeline.channel.count() == 4);
    assert(approxEqual(pipeline.channelMean(), 25.0));
    assert(approxEqual(pipeline.channelPeak(), 40.0));

    std::cout << "Test test_ingestBatch: PASS" << std::endl;
}

void test_normalisation_and_scaling() {
    SamplePipeline pipeline("ScaledChannel", 10);
    pipeline.scale = UnitScale(2.0, 5.0, "Celsius"); // toBase: x*2 + 5, fromBase: (x-5)/2

    double baseVal = pipeline.denormalised(10.0); // 10 * 2 + 5 = 25
    assert(approxEqual(baseVal, 25.0));

    double normVal = pipeline.normalised(25.0); // (25 - 5) / 2 = 10
    assert(approxEqual(normVal, 10.0));

    std::cout << "Test test_normalisation_and_scaling: PASS" << std::endl;
}

void test_flush() {
    SamplePipeline pipeline("FlushChannel", 10);
    pipeline.ingest(1, 10.0, 90);
    pipeline.ingest(2, 20.0, 90);
    pipeline.ingest(3, 30.0, 0); // rejected

    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.validSamples() == 2);

    int flushed = pipeline.flush();
    assert(flushed == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.channel.isEmpty());
    assert(pipeline.channel.channelName() == "FlushChannel");
    assert(pipeline.channel.capacity == 10);

    std::cout << "Test test_flush: PASS" << std::endl;
}

void test_describe() {
    SamplePipeline pipeline("Humidity", 10);
    pipeline.scale = UnitScale(1.0, 0.0, "%");

    assert(pipeline.describe() == "empty");

    pipeline.ingest(1, 45.0, 80);
    assert(pipeline.describe() == "Humidity:%");

    std::cout << "Test test_describe: PASS" << std::endl;
}

int main() {
    std::cout << "Starting SamplePipeline Test Suite..." << std::endl;

    test_default_constructor_and_initial_state();
    test_parameterized_constructor();
    test_filter_management();
    test_process_and_stage();
    test_wouldReject();
    test_ingest_basic_and_filters();
    test_quality_requirements_and_rejection();
    test_capacity_and_saturation();
    test_ingestBatch();
    test_normalisation_and_scaling();
    test_flush();
    test_describe();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}