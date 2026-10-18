#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "SamplePipeline.h"
#include "Filter.h"
#include "Sample.h"
#include "Channel.h"
#include "UnitScale.h"

static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// 1. Test Default and Parameterized Constructors
void test_constructors() {
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.channel.channelName() == "unnamed");
    assert(defaultPipeline.channel.capacity == 16);
    assert(!defaultPipeline.isSaturated());
    assert(defaultPipeline.describe() == "empty");

    SamplePipeline customPipeline("temperature", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.channel.channelName() == "temperature");
    assert(customPipeline.channel.capacity == 5);
    assert(!customPipeline.isSaturated());
    assert(customPipeline.describe() == "empty");

    std::cout << "Test test_constructors: PASS" << std::endl;
}

// 2. Test Filter Addition, Retrieval, Clearing, and Null Handling
void test_filter_management() {
    SamplePipeline pipeline("sensor", 10);
    assert(pipeline.filterCount() == 0);

    // Adding null filter should not increment count
    assert(pipeline.addFilter(nullptr) == 0);
    assert(pipeline.filterCount() == 0);

    PassFilter passFilter;
    ClampFilter clampFilter(0.0, 100.0);
    SmoothFilter smoothFilter(0.5);

    assert(pipeline.addFilter(&passFilter) == 1);
    assert(pipeline.addFilter(&clampFilter) == 2);
    assert(pipeline.addFilter(&smoothFilter) == 3);
    assert(pipeline.filterCount() == 3);

    // Test clear filters
    int previousCount = pipeline.clearFilters();
    assert(previousCount == 3);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

// 3. Test Multi-stage Process Pipeline and Stage-specific Execution
void test_process_and_stage() {
    SamplePipeline pipeline("math_pipeline", 10);
    ClampFilter clamp(0.0, 50.0);
    SmoothFilter smooth(0.5);

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // Stage out of range should return reading unaltered
    assert(pipeline.stage(75.0, -1) == 75.0);
    assert(pipeline.stage(75.0, 2) == 75.0);

    // Stage 0 is clamp: 75.0 clamped to 50.0
    double clamped = pipeline.stage(75.0, 0);
    assert(approxEqual(clamped, 50.0));

    // Reset smooth filter for clean process test
    SmoothFilter freshSmooth(0.5);
    SamplePipeline processPipeline("flow", 10);
    ClampFilter freshClamp(10.0, 20.0);
    processPipeline.addFilter(&freshClamp);
    processPipeline.addFilter(&freshSmooth);

    // Process 25.0 -> Clamp gives 20.0 -> Smooth unprimed returns 20.0
    double out1 = processPipeline.process(25.0);
    assert(approxEqual(out1, 20.0));

    // Process 0.0 -> Clamp gives 10.0 -> Smooth: 0.5 * 10 + 0.5 * 20 = 15.0
    double out2 = processPipeline.process(0.0);
    assert(approxEqual(out2, 15.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

// 4. Test wouldReject Logic with Various Filters
void test_would_reject() {
    SamplePipeline pipeline("rejection_test", 10);
    assert(!pipeline.wouldReject(100.0));

    ClampFilter clamp(0.0, 10.0);
    pipeline.addFilter(&clamp);

    assert(!pipeline.wouldReject(5.0));
    assert(pipeline.wouldReject(-1.0));
    assert(pipeline.wouldReject(15.0));

    // Disabled filter rejection
    pipeline.clearFilters();
    PassFilter pass;
    pass.toggle(); // Disabled
    pipeline.addFilter(&pass);
    assert(pipeline.wouldReject(42.0));
    assert(!pipeline.wouldReject(0.0)); // Filter::rejects allows 0.0 even when disabled

    std::cout << "Test test_would_reject: PASS" << std::endl;
}

// 5. Test Quality Requirements Configuration
void test_quality_requirements() {
    SamplePipeline pipeline("quality_test", 10);
    assert(pipeline.minimumQuality == 1);

    int oldQuality = pipeline.requireQuality(50);
    assert(oldQuality == 1);
    assert(pipeline.minimumQuality == 50);

    // Negative quality value should not modify minimumQuality
    oldQuality = pipeline.requireQuality(-5);
    assert(oldQuality == 50);
    assert(pipeline.minimumQuality == 50);

    std::cout << "Test test_quality_requirements: PASS" << std::endl;
}

// 6. Test Ingest Flow, Filtering, and Acceptance/Rejection Counter Updates
void test_ingest_and_rejections() {
    SamplePipeline pipeline("telemetry", 2);
    pipeline.requireQuality(20);

    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);

    // 1. Rejected: Quality below threshold
    int res1 = pipeline.ingest(1, 50.0, 10);
    assert(res1 == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 1);

    // 2. Rejected: Filter rejects reading
    int res2 = pipeline.ingest(2, 150.0, 80);
    assert(res2 == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 2);

    // 3. Accepted: Valid reading and quality
    int res3 = pipeline.ingest(3, 40.0, 80);
    assert(res3 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);
    assert(approxEqual(pipeline.latest().value(), 40.0));

    // 4. Accepted: Second valid reading
    int res4 = pipeline.ingest(4, 60.0, 90);
    assert(res4 == 2);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.isSaturated());

    // 5. Rejected: Channel capacity exceeded
    int res5 = pipeline.ingest(5, 50.0, 90);
    assert(res5 == -1);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 3);

    // 6. Rejected: Sample invalidity edge-case (quality 0 when minimumQuality is 0)
    SamplePipeline zeroQualityPipeline("zero_q", 5);
    zeroQualityPipeline.requireQuality(0);
    int res6 = zeroQualityPipeline.ingest(6, 10.0, 0); // Quality 0 sample has isValid() == false
    assert(res6 == -1);
    assert(zeroQualityPipeline.rejectedCount() == 1);

    std::cout << "Test test_ingest_and_rejections: PASS" << std::endl;
}

// 7. Test Ingest Batch
void test_ingest_batch() {
    SamplePipeline pipeline("batch_channel", 3);
    ClampFilter clamp(0.0, 50.0);
    pipeline.addFilter(&clamp);

    std::vector<double> readings = { 10.0, 60.0, 20.0, 30.0 }; // 60.0 rejected by clamp, 30.0 rejected by capacity
    int acceptedBatch = pipeline.ingestBatch(readings, 80);

    assert(acceptedBatch == 2); // Only 10.0 and 20.0 should be accepted
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.channel.count() == 2);

    std::cout << "Test test_ingest_batch: PASS" << std::endl;
}

// 8. Test Channel Metrics and Statistics
void test_channel_metrics() {
    SamplePipeline pipeline("stats", 5);
    pipeline.ingest(0, 10.0, 80);
    pipeline.ingest(1, 30.0, 80);
    pipeline.ingest(2, 20.0, 40); // Degraded (quality < 50) but still valid (isValid: !flagged && quality > 0)

    assert(approxEqual(pipeline.channelMean(), 20.0));
    assert(approxEqual(pipeline.channelPeak(), 30.0));
    assert(pipeline.validSamples() == 3);
    assert(!pipeline.isSaturated());

    Sample newest = pipeline.latest();
    assert(newest.position() == 2);
    assert(approxEqual(newest.value(), 20.0));
    assert(newest.score() == 40);

    std::cout << "Test test_channel_metrics: PASS" << std::endl;
}

// 9. Test Normalisation and Denormalisation with UnitScale
void test_unit_scale_operations() {
    SamplePipeline pipeline("pressure", 10);
    // Factor 2.0, Offset 10.0 -> toBase(x) = x * 2 + 10, fromBase(x) = (x - 10) / 2
    pipeline.scale = UnitScale(2.0, 10.0, "bar");

    assert(approxEqual(pipeline.denormalised(5.0), 20.0)); // 5 * 2 + 10 = 20
    assert(approxEqual(pipeline.normalised(20.0), 5.0));  // (20 - 10) / 2 = 5

    std::cout << "Test test_unit_scale_operations: PASS" << std::endl;
}

// 10. Test Flush and Describe
void test_flush_and_describe() {
    SamplePipeline pipeline("voltage", 4);
    pipeline.scale = UnitScale(1.0, 0.0, "mV");

    assert(pipeline.describe() == "empty");

    pipeline.ingest(1, 12.5, 75);
    pipeline.ingest(2, 13.5, 80);
    assert(pipeline.describe() == "voltage:mV");
    assert(pipeline.acceptedCount() == 2);

    int flushedCount = pipeline.flush();
    assert(flushedCount == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.channel.count() == 0);
    assert(pipeline.channel.capacity == 4);
    assert(pipeline.channel.channelName() == "voltage");
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_flush_and_describe: PASS" << std::endl;
}

int main() {
    std::cout << "Running SamplePipeline unit tests..." << std::endl;

    test_constructors();
    test_filter_management();
    test_process_and_stage();
    test_would_reject();
    test_quality_requirements();
    test_ingest_and_rejections();
    test_ingest_batch();
    test_channel_metrics();
    test_unit_scale_operations();
    test_flush_and_describe();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}