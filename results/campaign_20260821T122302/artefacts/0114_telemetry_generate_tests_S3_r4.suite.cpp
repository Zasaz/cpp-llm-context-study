#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cmath>

#include "SamplePipeline.h"
#include "Channel.h"
#include "Filter.h"
#include "Sample.h"
#include "UnitScale.h"

// Helper function for floating-point comparisons
bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test Constructors and Default Initializations
void test_SamplePipeline_constructors() {
    // Default constructor
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.minimumQuality == 1);
    assert(defaultPipeline.channel.channelName() == "unnamed");
    assert(defaultPipeline.channel.capacity == 16);
    assert(defaultPipeline.describe() == "empty");

    // Parameterized constructor
    SamplePipeline customPipeline("SensorAlpha", 10);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.minimumQuality == 1);
    assert(customPipeline.channel.channelName() == "SensorAlpha");
    assert(customPipeline.channel.capacity == 10);
    assert(customPipeline.describe() == "empty");

    std::cout << "Test test_SamplePipeline_constructors: PASS" << std::endl;
}

// Test Filter Management (addFilter, filterCount, clearFilters)
void test_SamplePipeline_filterManagement() {
    SamplePipeline pipeline("TestChannel", 5);

    ClampFilter f1(0.0, 100.0);
    PassFilter f2;
    SmoothFilter f3(0.5);

    assert(pipeline.filterCount() == 0);

    // Adding null filter should be a no-op
    assert(pipeline.addFilter(nullptr) == 0);
    assert(pipeline.filterCount() == 0);

    // Adding valid filters
    assert(pipeline.addFilter(&f1) == 1);
    assert(pipeline.addFilter(&f2) == 2);
    assert(pipeline.addFilter(&f3) == 3);
    assert(pipeline.filterCount() == 3);

    // Clearing filters
    int cleared = pipeline.clearFilters();
    assert(cleared == 3);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_SamplePipeline_filterManagement: PASS" << std::endl;
}

// Test Filter Processing and Stages
void test_SamplePipeline_processingAndStages() {
    SamplePipeline pipeline("TestProc", 5);

    ClampFilter clamp(0.0, 50.0);
    SmoothFilter smooth(0.5); // (0.5 * reading) + 0.5 * previous

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // stage() out of bounds checks
    assert(approxEqual(pipeline.stage(75.0, -1), 75.0));
    assert(approxEqual(pipeline.stage(75.0, 2), 75.0));

    // stage() at valid index
    // Clamp applied at index 0
    assert(approxEqual(pipeline.stage(75.0, 0), 50.0));

    // Pipeline process: 80 -> clamped to 50 -> smoothed (first time -> 50)
    double out1 = pipeline.process(80.0);
    assert(approxEqual(out1, 50.0));

    // Next process: 10 -> clamped to 10 -> smoothed (0.5*10 + 0.5*50 = 30)
    double out2 = pipeline.process(10.0);
    assert(approxEqual(out2, 30.0));

    std::cout << "Test test_SamplePipeline_processingAndStages: PASS" << std::endl;
}

// Test Quality Rejection and wouldReject
void test_SamplePipeline_rejection() {
    SamplePipeline pipeline("TestReject", 5);
    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);

    // wouldReject check based on clamp
    assert(pipeline.wouldReject(150.0) == true);
    assert(pipeline.wouldReject(-5.0) == true);
    assert(pipeline.wouldReject(50.0) == false);

    // Filter disabled rejection
    PassFilter pass;
    pass.toggle(); // disabled
    SamplePipeline pipeline2("TestPassReject", 5);
    pipeline2.addFilter(&pass);
    assert(pipeline2.wouldReject(10.0) == true);

    // Quality threshold tests
    SamplePipeline pipelineQuality("QualityChannel", 5);
    int prevReq = pipelineQuality.requireQuality(75);
    assert(prevReq == 1);
    assert(pipelineQuality.minimumQuality == 75);

    // Ingest rejected by quality
    int res = pipelineQuality.ingest(0, 10.0, 50);
    assert(res == -1);
    assert(pipelineQuality.rejectedCount() == 1);
    assert(pipelineQuality.acceptedCount() == 0);

    // Negative quality update should be ignored
    pipelineQuality.requireQuality(-5);
    assert(pipelineQuality.minimumQuality == 75);

    std::cout << "Test test_SamplePipeline_rejection: PASS" << std::endl;
}

// Test Ingest, IngestBatch, Capacity and Saturation
void test_SamplePipeline_ingestionAndCapacity() {
    SamplePipeline pipeline("CapacityChannel", 3);
    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);

    assert(pipeline.isSaturated() == false);

    // Ingest sample 1
    int res1 = pipeline.ingest(1, 10.0, 80);
    assert(res1 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 1);

    // Ingest sample 2 rejected by clamp filter
    int res2 = pipeline.ingest(2, 200.0, 80);
    assert(res2 == -1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 1);

    // Ingest sample 3
    int res3 = pipeline.ingest(3, 20.0, 90);
    assert(res3 == 2);
    assert(pipeline.acceptedCount() == 2);

    // Ingest sample 4
    int res4 = pipeline.ingest(4, 30.0, 90);
    assert(res4 == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.isSaturated() == true);

    // Ingest sample 5 - channel is full, should reject and count as rejected
    int res5 = pipeline.ingest(5, 40.0, 90);
    assert(res5 == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 3);

    // Test Batch Ingestion with a fresh pipeline
    SamplePipeline batchPipeline("BatchChannel", 10);
    std::vector<double> readings = { 5.0, 10.0, 15.0, 20.0 };
    int ingestedCount = batchPipeline.ingestBatch(readings, 85);
    assert(ingestedCount == 4);
    assert(batchPipeline.acceptedCount() == 4);
    assert(batchPipeline.validSamples() == 4);

    std::cout << "Test test_SamplePipeline_ingestionAndCapacity: PASS" << std::endl;
}

// Test Channel Statistics via SamplePipeline
void test_SamplePipeline_channelStats() {
    SamplePipeline pipeline("StatsChannel", 10);

    // Empty channel stats
    assert(approxEqual(pipeline.channelMean(), 0.0));
    assert(approxEqual(pipeline.channelPeak(), 0.0));

    pipeline.ingest(0, 10.0, 80);
    pipeline.ingest(1, 40.0, 80);
    pipeline.ingest(2, 25.0, 80);

    assert(approxEqual(pipeline.channelPeak(), 40.0));
    assert(approxEqual(pipeline.channelMean(), 25.0)); // (10 + 40 + 25) / 3 = 25.0

    Sample latestSample = pipeline.latest();
    assert(latestSample.position() == 2);
    assert(approxEqual(latestSample.value(), 25.0));
    assert(latestSample.score() == 80);

    std::cout << "Test test_SamplePipeline_channelStats: PASS" << std::endl;
}

// Test Normalisation, Denormalisation, and Describe
void test_SamplePipeline_scaleAndDescribe() {
    SamplePipeline pipeline("Pressure", 5);
    // Custom scale: scaleFactor = 2.0, offset = 10.0, unit = "bar"
    pipeline.scale = UnitScale(2.0, 10.0, "bar");

    // describe on empty pipeline
    assert(pipeline.describe() == "empty");

    // toBase: x * 2.0 + 10.0; fromBase: (x - 10.0) / 2.0
    assert(approxEqual(pipeline.denormalised(5.0), 20.0)); // 5.0 * 2 + 10 = 20
    assert(approxEqual(pipeline.normalised(20.0), 5.0));  // (20 - 10) / 2 = 5

    // Ingest a sample to make it non-empty
    pipeline.ingest(0, 50.0, 80);
    assert(pipeline.describe() == "Pressure:bar");

    std::cout << "Test test_SamplePipeline_scaleAndDescribe: PASS" << std::endl;
}

// Test Flush Functionality
void test_SamplePipeline_flush() {
    SamplePipeline pipeline("FlushChannel", 4);
    pipeline.ingest(0, 10.0, 80);
    pipeline.ingest(1, 20.0, 80);
    pipeline.ingest(2, 30.0, -1); // rejected due to quality

    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.validSamples() == 2);

    int flushedCount = pipeline.flush();
    assert(flushedCount == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.channel.isEmpty() == true);
    assert(pipeline.channel.channelName() == "FlushChannel");
    assert(pipeline.channel.capacity == 4);

    std::cout << "Test test_SamplePipeline_flush: PASS" << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "Starting SamplePipeline Test Suite" << std::endl;
    std::cout << "========================================" << std::endl;

    test_SamplePipeline_constructors();
    test_SamplePipeline_filterManagement();
    test_SamplePipeline_processingAndStages();
    test_SamplePipeline_rejection();
    test_SamplePipeline_ingestionAndCapacity();
    test_SamplePipeline_channelStats();
    test_SamplePipeline_scaleAndDescribe();
    test_SamplePipeline_flush();

    std::cout << "========================================" << std::endl;
    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}