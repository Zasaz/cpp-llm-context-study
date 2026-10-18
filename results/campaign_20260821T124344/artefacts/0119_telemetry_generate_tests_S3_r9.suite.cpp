#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>

#include "SamplePipeline.h"
#include "Channel.h"
#include "Filter.h"
#include "Sample.h"
#include "UnitScale.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) <= epsilon;
}

void test_constructors_and_defaults() {
    // Default constructor
    SamplePipeline p1;
    assert(p1.acceptedCount() == 0);
    assert(p1.rejectedCount() == 0);
    assert(p1.filterCount() == 0);
    assert(p1.minimumQuality == 1);
    assert(p1.channel.channelName() == "unnamed");
    assert(p1.channel.capacity == 16);
    assert(p1.validSamples() == 0);
    assert(!p1.isSaturated());
    assert(p1.describe() == "empty");

    // Parameterized constructor
    SamplePipeline p2("temperature", 5);
    assert(p2.acceptedCount() == 0);
    assert(p2.rejectedCount() == 0);
    assert(p2.filterCount() == 0);
    assert(p2.minimumQuality == 1);
    assert(p2.channel.channelName() == "temperature");
    assert(p2.channel.capacity == 5);
    assert(!p2.isSaturated());

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline("sensor", 10);
    PassFilter pass;
    ClampFilter clamp(0.0, 100.0);

    // Null filter pointer should not be added
    assert(pipeline.addFilter(nullptr) == 0);
    assert(pipeline.filterCount() == 0);

    // Adding valid filters
    assert(pipeline.addFilter(&pass) == 1);
    assert(pipeline.filterCount() == 1);

    assert(pipeline.addFilter(&clamp) == 2);
    assert(pipeline.filterCount() == 2);

    // Clear filters
    int cleared = pipeline.clearFilters();
    assert(cleared == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipeline("proc_test", 10);
    ClampFilter clamp(0.0, 50.0);
    SmoothFilter smooth(0.5); // weight 0.5

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // Testing stage execution on individual stages
    // Stage 0 is Clamp [0.0, 50.0]
    assert(isClose(pipeline.stage(70.0, 0), 50.0));
    assert(isClose(pipeline.stage(-10.0, 0), 0.0));
    assert(isClose(pipeline.stage(25.0, 0), 25.0));

    // Out of bound stage indices should return input unmodified
    assert(isClose(pipeline.stage(99.0, -1), 99.0));
    assert(isClose(pipeline.stage(99.0, 2), 99.0));

    // Reset smooth filter for clean process pipeline testing
    pipeline.clearFilters();
    ClampFilter c1(0.0, 10.0);
    pipeline.addFilter(&c1);

    // Reading 15 -> Clamp -> 10
    double processed = pipeline.process(15.0);
    assert(isClose(processed, 10.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_wouldReject() {
    SamplePipeline pipeline("reject_test", 10);
    ClampFilter clamp(10.0, 20.0);
    pipeline.addFilter(&clamp);

    assert(!pipeline.wouldReject(15.0));
    assert(pipeline.wouldReject(5.0));   // below clamp range
    assert(pipeline.wouldReject(25.0));  // above clamp range

    PassFilter pass;
    pipeline.addFilter(&pass);
    // Disabled filter rejects non-zero
    pass.toggle(); // enabled becomes false
    assert(pipeline.wouldReject(15.0));

    std::cout << "Test test_wouldReject: PASS" << std::endl;
}

void test_ingest_single_and_quality() {
    SamplePipeline pipeline("voltage", 3);
    ClampFilter clamp(0.0, 10.0);
    pipeline.addFilter(&clamp);

    pipeline.requireQuality(50);

    // Ingest with quality lower than required minimum (rejected)
    int res1 = pipeline.ingest(0, 5.0, 40);
    assert(res1 == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 1);

    // Ingest with reading that filter would reject (e.g. 15.0 for clamp(0, 10))
    int res2 = pipeline.ingest(1, 15.0, 60);
    assert(res2 == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 2);

    // Successful ingest
    int res3 = pipeline.ingest(2, 8.0, 70);
    assert(res3 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 1);

    Sample latestSample = pipeline.latest();
    assert(latestSample.position() == 2);
    assert(isClose(latestSample.value(), 8.0));
    assert(latestSample.score() == 70);

    // Test quality requirement update validation
    int prevMin = pipeline.requireQuality(80);
    assert(prevMin == 50);
    assert(pipeline.minimumQuality == 80);

    // Negative quality value should not update minimum quality
    pipeline.requireQuality(-5);
    assert(pipeline.minimumQuality == 80);

    std::cout << "Test test_ingest_single_and_quality: PASS" << std::endl;
}

void test_capacity_and_saturation() {
    SamplePipeline pipeline("buffer", 2);
    assert(!pipeline.isSaturated());

    assert(pipeline.ingest(0, 1.0, 50) == 1);
    assert(!pipeline.isSaturated());

    assert(pipeline.ingest(1, 2.0, 50) == 2);
    assert(pipeline.isSaturated());

    // Ingest when full -> rejected and channel drops it
    assert(pipeline.ingest(2, 3.0, 50) == -1);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.isSaturated());

    std::cout << "Test test_capacity_and_saturation: PASS" << std::endl;
}

void test_ingestBatch() {
    SamplePipeline pipeline("batch_channel", 10);
    ClampFilter clamp(0.0, 50.0);
    pipeline.addFilter(&clamp);

    std::vector<double> readings = { 10.0, 20.0, 60.0, 30.0, -5.0 }; // 60 and -5 will be rejected
    int taken = pipeline.ingestBatch(readings, 80);

    assert(taken == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 3);

    std::cout << "Test test_ingestBatch: PASS" << std::endl;
}

void test_channel_statistics() {
    SamplePipeline pipeline("stats", 10);
    pipeline.ingest(0, 10.0, 80);
    pipeline.ingest(1, 20.0, 80);
    pipeline.ingest(2, 30.0, 80);

    assert(isClose(pipeline.channelMean(), 20.0));
    assert(isClose(pipeline.channelPeak(), 30.0));

    std::cout << "Test test_channel_statistics: PASS" << std::endl;
}

void test_scaling_and_normalization() {
    SamplePipeline pipeline("scaled", 10);
    pipeline.scale = UnitScale(2.0, 5.0, "mV"); // toBase: x * 2 + 5, fromBase: (x - 5) / 2

    // normalised calls scale.fromBase
    assert(isClose(pipeline.normalised(25.0), 10.0));

    // denormalised calls scale.toBase
    assert(isClose(pipeline.denormalised(10.0), 25.0));

    std::cout << "Test test_scaling_and_normalization: PASS" << std::endl;
}

void test_flush_and_describe() {
    SamplePipeline pipeline("humidity", 5);
    pipeline.scale = UnitScale(1.0, 0.0, "%");

    assert(pipeline.describe() == "empty");

    pipeline.ingest(0, 45.0, 90);
    pipeline.ingest(1, 55.0, 90);
    assert(pipeline.describe() == "humidity:%");

    int flushed = pipeline.flush();
    assert(flushed == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.channel.isEmpty());
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_flush_and_describe: PASS" << std::endl;
}

int main() {
    test_constructors_and_defaults();
    test_filter_management();
    test_process_and_stage();
    test_wouldReject();
    test_ingest_single_and_quality();
    test_capacity_and_saturation();
    test_ingestBatch();
    test_channel_statistics();
    test_scaling_and_normalization();
    test_flush_and_describe();

    std::cout << "\nAll SamplePipeline unit tests passed successfully!" << std::endl;
    return 0;
}