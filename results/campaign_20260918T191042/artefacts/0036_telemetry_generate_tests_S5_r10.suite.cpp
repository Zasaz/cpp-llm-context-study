#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "Channel.h"
#include "Filter.h"
#include "Sample.h"
#include "SamplePipeline.h"
#include "UnitScale.h"

// Helper function for floating point comparisons
bool isNear(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// 1. Test Default and Parameterized Constructors & Initial State
void test_constructors_and_initial_state() {
    SamplePipeline defaultPipe;
    assert(defaultPipe.acceptedCount() == 0);
    assert(defaultPipe.rejectedCount() == 0);
    assert(defaultPipe.filterCount() == 0);
    assert(defaultPipe.minimumQuality == 1);
    assert(defaultPipe.channel.channelName() == "unnamed");
    assert(defaultPipe.channel.capacity == 16);
    assert(defaultPipe.describe() == "empty");
    assert(!defaultPipe.isSaturated());
    assert(defaultPipe.validSamples() == 0);

    SamplePipeline paramPipe("Voltage", 5);
    assert(paramPipe.acceptedCount() == 0);
    assert(paramPipe.rejectedCount() == 0);
    assert(paramPipe.filterCount() == 0);
    assert(paramPipe.minimumQuality == 1);
    assert(paramPipe.channel.channelName() == "Voltage");
    assert(paramPipe.channel.capacity == 5);
    assert(paramPipe.describe() == "empty");

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

// 2. Test Filter Management: Add, Null Safety, Count, Clear
void test_filter_management() {
    SamplePipeline pipeline("TestChannel", 10);
    PassFilter pf;
    ClampFilter cf(0.0, 10.0);

    assert(pipeline.filterCount() == 0);

    // Adding null filter should do nothing and return count
    int countAfterNull = pipeline.addFilter(nullptr);
    assert(countAfterNull == 0);
    assert(pipeline.filterCount() == 0);

    // Adding valid filters
    assert(pipeline.addFilter(&pf) == 1);
    assert(pipeline.addFilter(&cf) == 2);
    assert(pipeline.filterCount() == 2);

    // Clear filters
    int cleared = pipeline.clearFilters();
    assert(cleared == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

// 3. Test Process and Stage Filtering Execution
void test_process_and_stage() {
    SamplePipeline pipeline("FilterTest", 10);
    ClampFilter clamp(0.0, 50.0);
    SmoothFilter smooth(0.5);

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // Testing stage() with invalid and valid indices
    assert(isNear(pipeline.stage(100.0, -1), 100.0)); // Out of bounds low
    assert(isNear(pipeline.stage(100.0, 5), 100.0));  // Out of bounds high
    assert(isNear(pipeline.stage(100.0, 0), 50.0));   // Stage 0 is clamp: 100 clamped to 50.0

    // Smooth filter in stage 1 is now primed with stage call
    assert(isNear(pipeline.stage(20.0, 1), 20.0));    // Stage 1 primes with 20.0

    // Reset filters for predictable process() testing
    pipeline.clearFilters();
    ClampFilter cFilter(10.0, 30.0);
    PassFilter pFilter;
    pipeline.addFilter(&cFilter);
    pipeline.addFilter(&pFilter);

    // 5.0 clamped to 10.0, passed through pFilter -> 10.0
    assert(isNear(pipeline.process(5.0), 10.0));
    // 25.0 clamped to 25.0, passed through pFilter -> 25.0
    assert(isNear(pipeline.process(25.0), 25.0));
    // 40.0 clamped to 30.0, passed through pFilter -> 30.0
    assert(isNear(pipeline.process(40.0), 30.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

// 4. Test WouldReject Functionality
void test_would_reject() {
    SamplePipeline pipeline("RejectTest", 10);
    ClampFilter cf(5.0, 15.0);
    pipeline.addFilter(&cf);

    assert(pipeline.wouldReject(3.0));   // Below low bound
    assert(pipeline.wouldReject(20.0));  // Above high bound
    assert(!pipeline.wouldReject(10.0)); // Within bounds

    // Disabled generic filter test
    Filter genericFilter;
    genericFilter.toggle(); // enabled becomes false
    SamplePipeline pipe2("ToggleTest", 10);
    pipe2.addFilter(&genericFilter);
    assert(pipe2.wouldReject(5.0));  // Rejects non-zero when disabled
    assert(!pipe2.wouldReject(0.0)); // 0.0 is not rejected even when disabled

    std::cout << "Test test_would_reject: PASS" << std::endl;
}

// 5. Test Sample Ingest, Acceptance, and Rejection criteria
void test_ingest_and_quality_control() {
    SamplePipeline pipeline("IngestTest", 3);
    ClampFilter cf(0.0, 100.0);
    pipeline.addFilter(&cf);

    // Require minimum quality of 50
    int oldQuality = pipeline.requireQuality(50);
    assert(oldQuality == 1);
    assert(pipeline.minimumQuality == 50);

    // Negative requireQuality ignored
    pipeline.requireQuality(-5);
    assert(pipeline.minimumQuality == 50);

    // Ingest rejected due to low quality
    int res1 = pipeline.ingest(1, 20.0, 40);
    assert(res1 == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Ingest rejected due to filter rejection
    int res2 = pipeline.ingest(2, 150.0, 80);
    assert(res2 == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 0);

    // Successful ingest
    int res3 = pipeline.ingest(3, 25.0, 75);
    assert(res3 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 1);

    // Ingest another
    int res4 = pipeline.ingest(4, 35.0, 85);
    assert(res4 == 2);
    assert(pipeline.acceptedCount() == 2);

    // Third ingest fills capacity (capacity is 3)
    int res5 = pipeline.ingest(5, 45.0, 90);
    assert(res5 == 3);
    assert(pipeline.isSaturated());

    // Ingest beyond capacity: channel is full, ingest returns -1
    int res6 = pipeline.ingest(6, 20.0, 95);
    assert(res6 == -1);
    assert(pipeline.rejectedCount() == 3);

    std::cout << "Test test_ingest_and_quality_control: PASS" << std::endl;
}

// 6. Test Batch Ingestion
void test_ingest_batch() {
    SamplePipeline pipeline("BatchPipe", 4);
    pipeline.requireQuality(20);

    std::vector<double> batch = { 10.0, 20.0, 30.0, 40.0, 50.0 }; // 5 items, capacity is 4
    int taken = pipeline.ingestBatch(batch, 50);

    assert(taken == 4);
    assert(pipeline.acceptedCount() == 4);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.isSaturated());

    std::cout << "Test test_ingest_batch: PASS" << std::endl;
}

// 7. Test Channel Statistics (Mean, Peak, Latest)
void test_channel_statistics_and_latest() {
    SamplePipeline pipeline("StatsPipe", 5);

    // When empty
    assert(isNear(pipeline.channelMean(), 0.0));
    assert(isNear(pipeline.channelPeak(), 0.0));

    pipeline.ingest(1, 10.0, 70);
    pipeline.ingest(2, 40.0, 80);
    pipeline.ingest(3, 25.0, 90);

    assert(isNear(pipeline.channelMean(), (10.0 + 40.0 + 25.0) / 3.0));
    assert(isNear(pipeline.channelPeak(), 40.0));

    Sample latestSample = pipeline.latest();
    assert(latestSample.position() == 3);
    assert(isNear(latestSample.value(), 25.0));
    assert(latestSample.score() == 90);

    std::cout << "Test test_channel_statistics_and_latest: PASS" << std::endl;
}

// 8. Test Normalisation and Denormalisation with UnitScale
void test_normalisation_and_scale() {
    SamplePipeline pipeline("ScalePipe", 5);
    // scale: reading * factor + offset
    pipeline.scale = UnitScale(2.0, 5.0, "mV");

    // normalised (fromBase): (reading - 5.0) / 2.0
    assert(isNear(pipeline.normalised(25.0), 10.0));

    // denormalised (toBase): reading * 2.0 + 5.0
    assert(isNear(pipeline.denormalised(10.0), 25.0));

    std::cout << "Test test_normalisation_and_scale: PASS" << std::endl;
}

// 9. Test Flush and Describe
void test_flush_and_describe() {
    SamplePipeline pipeline("SensorA", 5);
    pipeline.scale = UnitScale(1.0, 0.0, "degC");

    assert(pipeline.describe() == "empty");

    pipeline.ingest(1, 22.5, 90);
    assert(pipeline.describe() == "SensorA:degC");

    // Flush channel
    int previousCount = pipeline.flush();
    assert(previousCount == 1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.channel.isEmpty());
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_flush_and_describe: PASS" << std::endl;
}

int main() {
    std::cout << "Starting SamplePipeline Test Suite..." << std::endl;

    test_constructors_and_initial_state();
    test_filter_management();
    test_process_and_stage();
    test_would_reject();
    test_ingest_and_quality_control();
    test_ingest_batch();
    test_channel_statistics_and_latest();
    test_normalisation_and_scale();
    test_flush_and_describe();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}