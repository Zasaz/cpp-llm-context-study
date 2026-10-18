#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cmath>

#include "SamplePipeline.h"
#include "Filter.h"
#include "Sample.h"
#include "UnitScale.h"
#include "Channel.h"

// Helper function for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_pipeline_constructors() {
    // Default constructor
    SamplePipeline defaultPipe;
    assert(defaultPipe.channel.channelName() == "unnamed");
    assert(defaultPipe.acceptedCount() == 0);
    assert(defaultPipe.rejectedCount() == 0);
    assert(defaultPipe.filterCount() == 0);
    assert(defaultPipe.minimumQuality == 1);
    assert(defaultPipe.describe() == "empty");

    // Parameterized constructor
    SamplePipeline customPipe("temperature", 5);
    assert(customPipe.channel.channelName() == "temperature");
    assert(customPipe.channel.capacity == 5);
    assert(customPipe.acceptedCount() == 0);
    assert(customPipe.rejectedCount() == 0);
    assert(customPipe.filterCount() == 0);
    assert(customPipe.minimumQuality == 1);

    std::cout << "Test test_pipeline_constructors: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipe("sensor", 10);

    PassFilter pf;
    ClampFilter cf(0.0, 100.0);

    // Adding null filter should not change filter count
    int countNull = pipe.addFilter(nullptr);
    assert(countNull == 0);
    assert(pipe.filterCount() == 0);

    // Adding valid filters
    assert(pipe.addFilter(&pf) == 1);
    assert(pipe.addFilter(&cf) == 2);
    assert(pipe.filterCount() == 2);

    // Clear filters
    int cleared = pipe.clearFilters();
    assert(cleared == 2);
    assert(pipe.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipe("voltage", 10);
    ClampFilter cf(0.0, 50.0);
    SmoothFilter sf(0.5);

    pipe.addFilter(&cf);
    pipe.addFilter(&sf);

    // Stage tests
    // Index out of bounds should return raw reading
    assert(isClose(pipe.stage(100.0, -1), 100.0));
    assert(isClose(pipe.stage(100.0, 2), 100.0));
    // Index 0: ClampFilter clamps 100.0 to 50.0
    assert(isClose(pipe.stage(100.0, 0), 50.0));

    // Pipeline process: clamp 100.0 -> 50.0, then smooth first value -> 50.0 (primed)
    double out1 = pipe.process(100.0);
    assert(isClose(out1, 50.0));

    // Next process: 0.0 -> clamp -> 0.0, smooth(0.0) -> 0.5 * 0.0 + 0.5 * 50.0 = 25.0
    double out2 = pipe.process(0.0);
    assert(isClose(out2, 25.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_wouldReject() {
    SamplePipeline pipe("pressure", 10);
    ClampFilter cf(10.0, 20.0);
    pipe.addFilter(&cf);

    // ClampFilter rejects if reading < 10.0 or reading > 20.0
    assert(pipe.wouldReject(5.0) == true);
    assert(pipe.wouldReject(25.0) == true);
    assert(pipe.wouldReject(15.0) == false);

    // Multiple filters check
    PassFilter pf;
    pipe.addFilter(&pf);
    assert(pipe.wouldReject(15.0) == false);

    // Disabling a generic filter causes it to reject non-zero readings
    pf.toggle(); // enabled = false
    assert(pf.rejects(15.0) == true);
    assert(pipe.wouldReject(15.0) == true);

    std::cout << "Test test_wouldReject: PASS" << std::endl;
}

void test_ingest_and_quality() {
    SamplePipeline pipe("flow", 5);

    // Require quality configuration
    assert(pipe.requireQuality(50) == 1); // Previous was 1
    assert(pipe.requireQuality(-5) == 50); // Negative does not modify
    assert(pipe.minimumQuality == 50);

    // Ingest with quality < minimumQuality (rejected)
    int res1 = pipe.ingest(1, 10.0, 40);
    assert(res1 == -1);
    assert(pipe.acceptedCount() == 0);
    assert(pipe.rejectedCount() == 1);

    // Ingest with quality >= minimumQuality (accepted)
    int res2 = pipe.ingest(2, 10.0, 60);
    assert(res2 == 1); // returns accepted count
    assert(pipe.acceptedCount() == 1);
    assert(pipe.rejectedCount() == 1);
    assert(pipe.validSamples() == 1);

    // Filter rejection during ingest
    ClampFilter cf(0.0, 100.0);
    pipe.addFilter(&cf);
    int res3 = pipe.ingest(3, 150.0, 70); // 150.0 rejected by cf
    assert(res3 == -1);
    assert(pipe.rejectedCount() == 2);
    assert(pipe.acceptedCount() == 1);

    std::cout << "Test test_ingest_and_quality: PASS" << std::endl;
}

void test_ingestBatch_and_saturation() {
    SamplePipeline pipe("batch_channel", 3);

    std::vector<double> batch = {1.0, 2.0, 3.0, 4.0};
    int taken = pipe.ingestBatch(batch, 80);

    // Only 3 should fit due to capacity = 3
    assert(taken == 3);
    assert(pipe.acceptedCount() == 3);
    assert(pipe.rejectedCount() == 1);
    assert(pipe.isSaturated() == true);
    assert(pipe.channel.droppedCount() == 1);

    std::cout << "Test test_ingestBatch_and_saturation: PASS" << std::endl;
}

void test_channel_statistics_and_latest() {
    SamplePipeline pipe("stats", 10);
    pipe.ingest(101, 10.0, 80);
    pipe.ingest(102, 30.0, 80);
    pipe.ingest(103, 20.0, 80);

    assert(isClose(pipe.channelMean(), 20.0));
    assert(isClose(pipe.channelPeak(), 30.0));

    Sample lat = pipe.latest();
    assert(lat.position() == 103);
    assert(isClose(lat.value(), 20.0));
    assert(lat.score() == 80);

    std::cout << "Test test_channel_statistics_and_latest: PASS" << std::endl;
}

void test_scaling_and_units() {
    SamplePipeline pipe("speed", 10);
    pipe.scale = UnitScale(2.0, 5.0, "m/s");

    // Base value -> normalised (fromBase): (y - 5.0) / 2.0
    assert(isClose(pipe.normalised(25.0), 10.0));

    // Normalised value -> denormalised (toBase): x * 2.0 + 5.0
    assert(isClose(pipe.denormalised(10.0), 25.0));

    // Test describe when empty vs non-empty
    assert(pipe.describe() == "empty");
    pipe.ingest(1, 10.0, 90);
    assert(pipe.describe() == "speed:m/s");

    std::cout << "Test test_scaling_and_units: PASS" << std::endl;
}

void test_flush() {
    SamplePipeline pipe("buffer", 5);
    pipe.ingest(1, 10.0, 90);
    pipe.ingest(2, 20.0, 90);
    pipe.ingest(3, 30.0, 0); // rejected due to quality < minimumQuality (1)

    assert(pipe.acceptedCount() == 2);
    assert(pipe.rejectedCount() == 1);
    assert(pipe.channel.count() == 2);

    int flushed = pipe.flush();
    assert(flushed == 2);
    assert(pipe.acceptedCount() == 0);
    assert(pipe.rejectedCount() == 0);
    assert(pipe.channel.count() == 0);
    assert(pipe.describe() == "empty");
    assert(pipe.channel.channelName() == "buffer");
    assert(pipe.channel.capacity == 5);

    std::cout << "Test test_flush: PASS" << std::endl;
}

int main() {
    test_pipeline_constructors();
    test_filter_management();
    test_process_and_stage();
    test_wouldReject();
    test_ingest_and_quality();
    test_ingestBatch_and_saturation();
    test_channel_statistics_and_latest();
    test_scaling_and_units();
    test_flush();

    std::cout << "\nAll SamplePipeline unit tests passed successfully!" << std::endl;
    return 0;
}