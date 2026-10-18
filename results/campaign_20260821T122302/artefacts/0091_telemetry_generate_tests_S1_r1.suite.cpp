#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>

#include "SamplePipeline.h"
#include "Filter.h"
#include "UnitScale.h"
#include "Sample.h"
#include "Channel.h"

static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

void test_constructors_and_defaults() {
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.channel.channelName() == "unnamed");
    assert(defaultPipeline.channel.capacity == 16);
    assert(!defaultPipeline.isSaturated());
    assert(defaultPipeline.validSamples() == 0);

    SamplePipeline customPipeline("temperature", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.channel.channelName() == "temperature");
    assert(customPipeline.channel.capacity == 5);
    assert(!customPipeline.isSaturated());

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline("sensor", 10);
    assert(pipeline.filterCount() == 0);

    // Adding null pointer should not add to filters
    int countAfterNull = pipeline.addFilter(nullptr);
    assert(countAfterNull == 0);
    assert(pipeline.filterCount() == 0);

    PassFilter pf;
    ClampFilter cf(0.0, 100.0);
    SmoothFilter sf(0.5);

    assert(pipeline.addFilter(&pf) == 1);
    assert(pipeline.addFilter(&cf) == 2);
    assert(pipeline.addFilter(&sf) == 3);
    assert(pipeline.filterCount() == 3);

    // Test clearFilters
    int cleared = pipeline.clearFilters();
    assert(cleared == 3);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipeline("proc", 10);

    ClampFilter clamp(0.0, 50.0);
    PassFilter pass;

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&pass);

    // Process chains through filters: clamped to 50.0, pass returns 50.0
    double processed = pipeline.process(75.0);
    assert(approxEqual(processed, 50.0));

    double underflow = pipeline.process(-10.0);
    assert(approxEqual(underflow, 0.0));

    // Test stage with valid indices
    assert(approxEqual(pipeline.stage(80.0, 0), 50.0));
    assert(approxEqual(pipeline.stage(80.0, 1), 80.0));

    // Test stage with invalid indices (returns original reading)
    assert(approxEqual(pipeline.stage(80.0, -1), 80.0));
    assert(approxEqual(pipeline.stage(80.0, 2), 80.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_wouldReject() {
    SamplePipeline pipeline("reject_test", 10);
    ClampFilter clamp(10.0, 20.0);
    pipeline.addFilter(&clamp);

    assert(!pipeline.wouldReject(15.0));
    assert(pipeline.wouldReject(5.0));
    assert(pipeline.wouldReject(25.0));

    // Clear filters and check rejection with disabled filter
    pipeline.clearFilters();
    Filter baseFilter;
    baseFilter.toggle(); // disables the filter
    pipeline.addFilter(&baseFilter);

    assert(pipeline.wouldReject(1.0));
    assert(!pipeline.wouldReject(0.0));

    std::cout << "Test test_wouldReject: PASS" << std::endl;
}

void test_requireQuality() {
    SamplePipeline pipeline("quality_test", 10);

    // Default minimum quality is 1
    int prev = pipeline.requireQuality(50);
    assert(prev == 1);
    assert(pipeline.minimumQuality == 50);

    // Negative minimum value should be ignored
    int prev2 = pipeline.requireQuality(-5);
    assert(prev2 == 50);
    assert(pipeline.minimumQuality == 50);

    std::cout << "Test test_requireQuality: PASS" << std::endl;
}

void test_ingest_single() {
    SamplePipeline pipeline("voltage", 3);
    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);

    pipeline.requireQuality(20);

    // 1. Rejection due to low quality
    int r1 = pipeline.ingest(1, 50.0, 10);
    assert(r1 == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 1);

    // 2. Rejection due to filter rejection
    int r2 = pipeline.ingest(2, 150.0, 50);
    assert(r2 == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 2);

    // 3. Successful ingestion
    int r3 = pipeline.ingest(3, 50.0, 80);
    assert(r3 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 1);

    // 4. Ingest up to capacity
    assert(pipeline.ingest(4, 60.0, 80) == 2);
    assert(pipeline.ingest(5, 70.0, 80) == 3);
    assert(pipeline.isSaturated());

    // 5. Ingest when full -> rejection
    int r6 = pipeline.ingest(6, 80.0, 80);
    assert(r6 == -1);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 3);

    std::cout << "Test test_ingest_single: PASS" << std::endl;
}

void test_ingestBatch() {
    SamplePipeline pipeline("batch_pipe", 5);
    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);

    std::vector<double> readings = {10.0, 20.0, 150.0, 30.0, 40.0, 50.0};
    // 10.0 -> OK (1)
    // 20.0 -> OK (2)
    // 150.0 -> Rejected by clamp filter
    // 30.0 -> OK (3)
    // 40.0 -> OK (4)
    // 50.0 -> OK (5 - pipeline capacity reached)
    int acceptedBatch = pipeline.ingestBatch(readings, 75);
    assert(acceptedBatch == 5);
    assert(pipeline.acceptedCount() == 5);
    assert(pipeline.rejectedCount() == 1); // 150.0 rejected

    std::cout << "Test test_ingestBatch: PASS" << std::endl;
}

void test_channel_statistics_and_latest() {
    SamplePipeline pipeline("stats", 10);
    pipeline.ingest(101, 10.0, 90);
    pipeline.ingest(102, 30.0, 90);
    pipeline.ingest(103, 20.0, 90);

    assert(approxEqual(pipeline.channelMean(), 20.0));
    assert(approxEqual(pipeline.channelPeak(), 30.0));

    Sample latestSample = pipeline.latest();
    assert(latestSample.position() == 103);
    assert(approxEqual(latestSample.value(), 20.0));
    assert(latestSample.score() == 90);

    std::cout << "Test test_channel_statistics_and_latest: PASS" << std::endl;
}

void test_normalised_and_denormalised() {
    SamplePipeline pipeline("scaled", 10);
    pipeline.scale = UnitScale(2.0, 10.0, "mV");

    // scale: base = reading * 2 + 10 => fromBase = (reading - 10) / 2
    double baseVal = 50.0;
    double norm = pipeline.normalised(baseVal); // (50 - 10) / 2 = 20
    assert(approxEqual(norm, 20.0));

    double denorm = pipeline.denormalised(20.0); // 20 * 2 + 10 = 50
    assert(approxEqual(denorm, 50.0));

    std::cout << "Test test_normalised_and_denormalised: PASS" << std::endl;
}

void test_flush() {
    SamplePipeline pipeline("flushable", 4);
    pipeline.ingest(1, 10.0, 80);
    pipeline.ingest(2, 20.0, 80);
    assert(pipeline.acceptedCount() == 2);

    int flushedCount = pipeline.flush();
    assert(flushedCount == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(!pipeline.isSaturated());
    assert(pipeline.channel.channelName() == "flushable");
    assert(pipeline.channel.capacity == 4);

    std::cout << "Test test_flush: PASS" << std::endl;
}

void test_describe() {
    SamplePipeline pipeline("pressure", 10);
    pipeline.scale = UnitScale(1.0, 0.0, "kPa");

    // When empty
    assert(pipeline.describe() == "empty");

    // When not empty
    pipeline.ingest(1, 101.3, 100);
    assert(pipeline.describe() == "pressure:kPa");

    std::cout << "Test test_describe: PASS" << std::endl;
}

int main() {
    test_constructors_and_defaults();
    test_filter_management();
    test_process_and_stage();
    test_wouldReject();
    test_requireQuality();
    test_ingest_single();
    test_ingestBatch();
    test_channel_statistics_and_latest();
    test_normalised_and_denormalised();
    test_flush();
    test_describe();

    std::cout << "\nAll SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}