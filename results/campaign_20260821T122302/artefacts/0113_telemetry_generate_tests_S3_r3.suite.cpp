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

static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) <= epsilon;
}

void test_constructors_and_initial_state() {
    // Default constructor
    SamplePipeline defaultPipe;
    assert(defaultPipe.acceptedCount() == 0);
    assert(defaultPipe.rejectedCount() == 0);
    assert(defaultPipe.minimumQuality == 1);
    assert(defaultPipe.filterCount() == 0);
    assert(defaultPipe.validSamples() == 0);
    assert(!defaultPipe.isSaturated());
    assert(defaultPipe.describe() == "empty");
    assert(defaultPipe.channel.channelName() == "unnamed");
    assert(defaultPipe.channel.capacity == 16);

    // Parameterized constructor
    SamplePipeline customPipe("voltage", 5);
    assert(customPipe.acceptedCount() == 0);
    assert(customPipe.rejectedCount() == 0);
    assert(customPipe.minimumQuality == 1);
    assert(customPipe.filterCount() == 0);
    assert(customPipe.validSamples() == 0);
    assert(!customPipe.isSaturated());
    assert(customPipe.describe() == "empty");
    assert(customPipe.channel.channelName() == "voltage");
    assert(customPipe.channel.capacity == 5);

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipe("temp", 10);
    assert(pipe.filterCount() == 0);

    // Adding null filter should not change count
    int countAfterNull = pipe.addFilter(nullptr);
    assert(countAfterNull == 0);
    assert(pipe.filterCount() == 0);

    PassFilter pass;
    ClampFilter clamp(0.0, 100.0);
    SmoothFilter smooth(0.5);

    assert(pipe.addFilter(&pass) == 1);
    assert(pipe.addFilter(&clamp) == 2);
    assert(pipe.addFilter(&smooth) == 3);
    assert(pipe.filterCount() == 3);

    // Clear filters
    int previousCount = pipe.clearFilters();
    assert(previousCount == 3);
    assert(pipe.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipe("sensor", 10);

    ClampFilter clamp(10.0, 50.0);
    SmoothFilter smooth(0.5);

    pipe.addFilter(&clamp);
    pipe.addFilter(&smooth);

    // Test stage with valid index
    // Stage 0 (clamp only): 5.0 -> clamped to 10.0
    double stage0Result = pipe.stage(5.0, 0);
    assert(approxEqual(stage0Result, 10.0));

    // Test stage with invalid indices (returns reading unchanged)
    assert(approxEqual(pipe.stage(100.0, -1), 100.0));
    assert(approxEqual(pipe.stage(100.0, 2), 100.0));

    // Clear and re-test full process pipeline
    pipe.clearFilters();
    ClampFilter clamp2(0.0, 100.0);
    SmoothFilter smooth2(0.5); // first reading primes it
    pipe.addFilter(&clamp2);
    pipe.addFilter(&smooth2);

    // First process: input 150 -> clamped to 100 -> smooth primed with 100
    double out1 = pipe.process(150.0);
    assert(approxEqual(out1, 100.0));

    // Second process: input 50 -> clamped to 50 -> smooth: 0.5 * 50 + 0.5 * 100 = 75
    double out2 = pipe.process(50.0);
    assert(approxEqual(out2, 75.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_wouldReject() {
    SamplePipeline pipe("detector", 10);
    ClampFilter clamp(10.0, 20.0);

    assert(!pipe.wouldReject(100.0)); // No filters yet

    pipe.addFilter(&clamp);
    assert(pipe.wouldReject(5.0));
    assert(pipe.wouldReject(25.0));
    assert(!pipe.wouldReject(15.0));
    assert(!pipe.wouldReject(10.0));
    assert(!pipe.wouldReject(20.0));

    // Generic Filter rejects if disabled and reading != 0.0
    Filter genFilter;
    genFilter.toggle(); // disables it
    SamplePipeline pipe2("detector2", 10);
    pipe2.addFilter(&genFilter);
    assert(pipe2.wouldReject(1.0));
    assert(!pipe2.wouldReject(0.0));

    std::cout << "Test test_wouldReject: PASS" << std::endl;
}

void test_ingest_success_and_filtering() {
    SamplePipeline pipe("pressure", 5);
    ClampFilter clamp(0.0, 100.0);
    pipe.addFilter(&clamp);

    // Ingest valid sample with filtering
    int accepted1 = pipe.ingest(101, 150.0, 80);
    assert(accepted1 == 1);
    assert(pipe.acceptedCount() == 1);
    assert(pipe.rejectedCount() == 0);
    assert(pipe.validSamples() == 1);

    Sample latest = pipe.latest();
    assert(latest.position() == 101);
    assert(approxEqual(latest.value(), 100.0)); // Clamped from 150.0
    assert(latest.score() == 80);

    int accepted2 = pipe.ingest(102, 50.0, 90);
    assert(accepted2 == 2);
    assert(pipe.acceptedCount() == 2);
    assert(pipe.latest().position() == 102);
    assert(approxEqual(pipe.latest().value(), 50.0));

    std::cout << "Test test_ingest_success_and_filtering: PASS" << std::endl;
}

void test_ingest_rejections() {
    SamplePipeline pipe("flow", 2);

    // 1. Rejection due to quality < minimumQuality
    pipe.requireQuality(50);
    int res1 = pipe.ingest(1, 10.0, 49);
    assert(res1 == -1);
    assert(pipe.rejectedCount() == 1);
    assert(pipe.acceptedCount() == 0);

    // 2. Rejection due to filter rejection
    ClampFilter clamp(0.0, 10.0);
    pipe.addFilter(&clamp);
    int res2 = pipe.ingest(2, 20.0, 60); // 20.0 rejected by clamp
    assert(res2 == -1);
    assert(pipe.rejectedCount() == 2);
    assert(pipe.acceptedCount() == 0);

    // 3. Successful ingestion up to capacity
    int res3 = pipe.ingest(3, 5.0, 60);
    assert(res3 == 1);
    int res4 = pipe.ingest(4, 6.0, 60);
    assert(res4 == 2);
    assert(pipe.isSaturated());

    // 4. Rejection due to channel capacity full
    int res5 = pipe.ingest(5, 7.0, 60);
    assert(res5 == -1);
    assert(pipe.rejectedCount() == 3);
    assert(pipe.acceptedCount() == 2);

    std::cout << "Test test_ingest_rejections: PASS" << std::endl;
}

void test_ingest_batch() {
    SamplePipeline pipe("batch_channel", 4);
    ClampFilter clamp(0.0, 50.0);
    pipe.addFilter(&clamp);

    // Vector has 5 items: 10 (ok), 20 (ok), 60 (rejected by clamp), 30 (ok), 40 (rejected due to full capacity 4: already 3 accepted)
    // Wait, capacity is 4.
    // Index 0: 10.0 -> accepted (1/4)
    // Index 1: 20.0 -> accepted (2/4)
    // Index 2: 60.0 -> rejected by filter
    // Index 3: 30.0 -> accepted (3/4)
    // Index 4: 40.0 -> accepted (4/4)
    // Index 5: 50.0 -> rejected by channel full
    std::vector<double> readings = {10.0, 20.0, 60.0, 30.0, 40.0, 50.0};
    int taken = pipe.ingestBatch(readings, 75);

    assert(taken == 4);
    assert(pipe.acceptedCount() == 4);
    assert(pipe.rejectedCount() == 2); // 60.0 rejected by filter, 50.0 rejected by capacity
    assert(pipe.isSaturated());

    std::cout << "Test test_ingest_batch: PASS" << std::endl;
}

void test_quality_threshold_update() {
    SamplePipeline pipe("qual", 10);
    assert(pipe.minimumQuality == 1);

    int prev = pipe.requireQuality(50);
    assert(prev == 1);
    assert(pipe.minimumQuality == 50);

    // Negative requirement should be ignored
    prev = pipe.requireQuality(-5);
    assert(prev == 50);
    assert(pipe.minimumQuality == 50);

    // Quality 0 should be accepted as setting
    prev = pipe.requireQuality(0);
    assert(prev == 50);
    assert(pipe.minimumQuality == 0);

    std::cout << "Test test_quality_threshold_update: PASS" << std::endl;
}

void test_channel_queries_and_stats() {
    SamplePipeline pipe("stats", 5);

    // Empty channel
    assert(approxEqual(pipe.channelMean(), 0.0));
    assert(approxEqual(pipe.channelPeak(), 0.0));
    assert(pipe.validSamples() == 0);

    // Ingest data
    pipe.ingest(1, 10.0, 80);
    pipe.ingest(2, 30.0, 80);
    pipe.ingest(3, 20.0, 80);

    assert(approxEqual(pipe.channelMean(), 20.0));
    assert(approxEqual(pipe.channelPeak(), 30.0));
    assert(pipe.validSamples() == 3);

    Sample latest = pipe.latest();
    assert(latest.position() == 3);
    assert(approxEqual(latest.value(), 20.0));

    std::cout << "Test test_channel_queries_and_stats: PASS" << std::endl;
}

void test_scaling_and_normalization() {
    SamplePipeline pipe("scaled", 10);
    // Custom unit scale: factor = 2.0, offset = 10.0, unit = "mV"
    pipe.scale = UnitScale(2.0, 10.0, "mV");

    // normalised calls scale.fromBase(reading): (reading - 10) / 2
    double norm = pipe.normalised(30.0);
    assert(approxEqual(norm, 10.0));

    // denormalised calls scale.toBase(reading): reading * 2 + 10
    double denorm = pipe.denormalised(10.0);
    assert(approxEqual(denorm, 30.0));

    std::cout << "Test test_scaling_and_normalization: PASS" << std::endl;
}

void test_flush_and_describe() {
    SamplePipeline pipe("temp_stream", 5);
    pipe.scale = UnitScale(1.0, 0.0, "Celsius");

    // When empty
    assert(pipe.describe() == "empty");

    // After ingestion
    pipe.ingest(1, 23.5, 90);
    assert(pipe.describe() == "temp_stream:Celsius");
    assert(pipe.acceptedCount() == 1);
    assert(pipe.channel.count() == 1);

    // Flush
    int flushedCount = pipe.flush();
    assert(flushedCount == 1);
    assert(pipe.acceptedCount() == 0);
    assert(pipe.rejectedCount() == 0);
    assert(pipe.channel.count() == 0);
    assert(pipe.channel.channelName() == "temp_stream");
    assert(pipe.channel.capacity == 5);
    assert(pipe.describe() == "empty");

    std::cout << "Test test_flush_and_describe: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_filter_management();
    test_process_and_stage();
    test_wouldReject();
    test_ingest_success_and_filtering();
    test_ingest_rejections();
    test_ingest_batch();
    test_quality_threshold_update();
    test_channel_queries_and_stats();
    test_scaling_and_normalization();
    test_flush_and_describe();

    std::cout << "\nAll SamplePipeline unit tests passed successfully!" << std::endl;
    return 0;
}