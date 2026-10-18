#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>

#include "SamplePipeline.h"
#include "Filter.h"
#include "Channel.h"
#include "Sample.h"
#include "UnitScale.h"

// Helper function for floating point comparisons
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// 1. Test Default and Parameterized Constructors
void test_constructors() {
    // Default constructor
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.channel.channelName() == "unnamed");
    assert(defaultPipeline.channel.capacity == 16);
    assert(defaultPipeline.minimumQuality == 1);
    assert(defaultPipeline.describe() == "empty");

    // Parameterized constructor
    SamplePipeline customPipeline("temperature", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.channel.channelName() == "temperature");
    assert(customPipeline.channel.capacity == 5);
    assert(customPipeline.minimumQuality == 1);
    assert(customPipeline.describe() == "empty");

    std::cout << "Test test_constructors: PASS" << std::endl;
}

// 2. Test Filter Management (addFilter, filterCount, clearFilters)
void test_filterManagement() {
    SamplePipeline pipeline("sensor", 10);
    PassFilter pass;
    ClampFilter clamp(0.0, 100.0);

    assert(pipeline.filterCount() == 0);
    assert(pipeline.addFilter(nullptr) == 0);
    assert(pipeline.filterCount() == 0);

    assert(pipeline.addFilter(&pass) == 1);
    assert(pipeline.filterCount() == 1);

    assert(pipeline.addFilter(&clamp) == 2);
    assert(pipeline.filterCount() == 2);

    int cleared = pipeline.clearFilters();
    assert(cleared == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filterManagement: PASS" << std::endl;
}

// 3. Test Filter Processing & Staging (process, stage, wouldReject)
void test_processAndStage() {
    SamplePipeline pipeline("voltage", 10);
    ClampFilter clamp(0.0, 10.0);
    SmoothFilter smooth(0.5); // primed with first, then: 0.5 * reading + 0.5 * previous

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // stage() out of bounds checks
    assert(approxEqual(pipeline.stage(15.0, -1), 15.0));
    assert(approxEqual(pipeline.stage(15.0, 2), 15.0));

    // stage(15.0, 0) clamps 15.0 to 10.0
    assert(approxEqual(pipeline.stage(15.0, 0), 10.0));

    // Testing wouldReject
    assert(!pipeline.wouldReject(5.0));
    assert(pipeline.wouldReject(15.0));  // clamp rejects > 10.0
    assert(pipeline.wouldReject(-5.0));  // clamp rejects < 0.0

    // Full process chain:
    // 1st input: 12.0 -> clamp gives 10.0 -> smooth (unprimed) gives 10.0
    double out1 = pipeline.process(12.0);
    assert(approxEqual(out1, 10.0));

    // 2nd input: -5.0 -> clamp gives 0.0 -> smooth: 0.5 * 0.0 + 0.5 * 10.0 = 5.0
    double out2 = pipeline.process(-5.0);
    assert(approxEqual(out2, 5.0));

    std::cout << "Test test_processAndStage: PASS" << std::endl;
}

// 4. Test Ingestion and Quality Validation
void test_ingestAndQuality() {
    SamplePipeline pipeline("humidity", 5);
    pipeline.requireQuality(50);

    // Test quality threshold rejection
    int resLowQuality = pipeline.ingest(1, 45.0, 40); // quality < 50
    assert(resLowQuality == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 1);

    // Test successful ingestion
    int resOk = pipeline.ingest(2, 45.0, 50);
    assert(resOk == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.validSamples() == 1);

    // Test quality setting modification
    int prevMin = pipeline.requireQuality(80);
    assert(prevMin == 50);
    assert(pipeline.minimumQuality == 80);

    // Negative minimum quality ignored
    pipeline.requireQuality(-10);
    assert(pipeline.minimumQuality == 80);

    // Ingest with filter rejection
    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);
    int resFilterReject = pipeline.ingest(3, 120.0, 90);
    assert(resFilterReject == -1);
    assert(pipeline.rejectedCount() == 2);

    std::cout << "Test test_ingestAndQuality: PASS" << std::endl;
}

// 5. Test Batch Ingestion and Capacity Saturation
void test_ingestBatchAndSaturation() {
    SamplePipeline pipeline("pressure", 3);

    std::vector<double> batch = { 101.3, 101.5, 101.2, 102.0 };
    // Pipeline capacity is 3. 4th reading should be rejected due to channel full.
    int taken = pipeline.ingestBatch(batch, 80);
    assert(taken == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.isSaturated());
    assert(pipeline.validSamples() == 3);

    Sample latest = pipeline.latest();
    assert(latest.position() == 2);
    assert(approxEqual(latest.value(), 101.2));

    std::cout << "Test test_ingestBatchAndSaturation: PASS" << std::endl;
}

// 6. Test Channel Analytics (mean, peak)
void test_channelAnalytics() {
    SamplePipeline pipeline("metrics", 10);
    pipeline.ingest(0, 10.0, 100);
    pipeline.ingest(1, 30.0, 100);
    pipeline.ingest(2, 20.0, 100);

    assert(approxEqual(pipeline.channelMean(), 20.0));
    assert(approxEqual(pipeline.channelPeak(), 30.0));

    std::cout << "Test test_channelAnalytics: PASS" << std::endl;
}

// 7. Test Unit Scaling and Normalisation
void test_scalingAndNormalisation() {
    SamplePipeline pipeline("distance", 10);
    // Factor: 1000.0, offset: 0.0 -> e.g. km to meters
    pipeline.scale = UnitScale(1000.0, 0.0, "km");

    // normalised calls scale.fromBase() -> (reading - 0) / 1000.0
    assert(approxEqual(pipeline.normalised(5000.0), 5.0));

    // denormalised calls scale.toBase() -> reading * 1000.0 + 0
    assert(approxEqual(pipeline.denormalised(5.0), 5000.0));

    std::cout << "Test test_scalingAndNormalisation: PASS" << std::endl;
}

// 8. Test Pipeline Flush and Describe
void test_flushAndDescribe() {
    SamplePipeline pipeline("sound", 5);
    pipeline.scale = UnitScale(1.0, 0.0, "dB");

    assert(pipeline.describe() == "empty");

    pipeline.ingest(0, 42.0, 90);
    assert(pipeline.describe() == "sound:dB");

    int flushedCount = pipeline.flush();
    assert(flushedCount == 1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_flushAndDescribe: PASS" << std::endl;
}

// Main entry point running all unit tests
int main() {
    std::cout << "=== Running SamplePipeline Test Suite ===" << std::endl;

    test_constructors();
    test_filterManagement();
    test_processAndStage();
    test_ingestAndQuality();
    test_ingestBatchAndSaturation();
    test_channelAnalytics();
    test_scalingAndNormalisation();
    test_flushAndDescribe();

    std::cout << "=== All SamplePipeline Tests Passed Successfully! ===" << std::endl;
    return 0;
}