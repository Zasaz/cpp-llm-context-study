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

// Helper for floating point comparisons
bool isClose(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_constructors_and_defaults() {
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.minimumQuality == 1);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.channel.channelName() == "unnamed");
    assert(defaultPipeline.channel.capacity == 16);
    assert(!defaultPipeline.isSaturated());
    assert(defaultPipeline.describe() == "empty");

    SamplePipeline customPipeline("Temperature", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.minimumQuality == 1);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.channel.channelName() == "Temperature");
    assert(customPipeline.channel.capacity == 5);
    assert(!customPipeline.isSaturated());
    assert(customPipeline.describe() == "empty");

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline("Sensor", 10);
    PassFilter pFilter;
    ClampFilter cFilter(0.0, 100.0);

    assert(pipeline.filterCount() == 0);

    // Adding null filter should not increase count
    int countAfterNull = pipeline.addFilter(nullptr);
    assert(countAfterNull == 0);
    assert(pipeline.filterCount() == 0);

    // Adding valid filters
    assert(pipeline.addFilter(&pFilter) == 1);
    assert(pipeline.filterCount() == 1);
    assert(pipeline.addFilter(&cFilter) == 2);
    assert(pipeline.filterCount() == 2);

    // Clear filters
    int cleared = pipeline.clearFilters();
    assert(cleared == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipeline("ProcessTest", 10);
    ClampFilter clamp(0.0, 50.0);
    SmoothFilter smooth(0.5);

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // Test stage execution
    // Stage 0 is clamp
    double stagedVal = pipeline.stage(60.0, 0);
    assert(isClose(stagedVal, 50.0));

    // Stage out of range returns input directly
    assert(isClose(pipeline.stage(60.0, -1), 60.0));
    assert(isClose(pipeline.stage(60.0, 2), 60.0));

    // Reset pipeline for full process test
    pipeline.clearFilters();
    ClampFilter clamp2(10.0, 20.0);
    SmoothFilter smooth2(0.5); // unprimed: first reading stays as-is
    pipeline.addFilter(&clamp2);
    pipeline.addFilter(&smooth2);

    // 25.0 -> clamped to 20.0 -> smooth unprimed -> 20.0
    double out1 = pipeline.process(25.0);
    assert(isClose(out1, 20.0));

    // 5.0 -> clamped to 10.0 -> smooth primed (0.5 * 10.0 + 0.5 * 20.0) -> 15.0
    double out2 = pipeline.process(5.0);
    assert(isClose(out2, 15.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_wouldReject() {
    SamplePipeline pipeline("RejectTest", 10);
    ClampFilter clamp(0.0, 10.0);
    pipeline.addFilter(&clamp);

    assert(!pipeline.wouldReject(5.0));
    assert(pipeline.wouldReject(-1.0));
    assert(pipeline.wouldReject(11.0));

    std::cout << "Test test_wouldReject: PASS" << std::endl;
}

void test_ingest_and_quality() {
    SamplePipeline pipeline("IngestTest", 3);
    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);

    // Initial quality requirement is 1
    // Rejection due to quality < minimumQuality
    int res1 = pipeline.ingest(0, 50.0, 0);
    assert(res1 == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Rejection due to wouldReject (reading > 100.0)
    int res2 = pipeline.ingest(1, 150.0, 50);
    assert(res2 == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 0);

    // Successful ingest
    int res3 = pipeline.ingest(2, 50.0, 80);
    assert(res3 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 1);
    assert(pipeline.latest().value() == 50.0);
    assert(pipeline.latest().position() == 2);

    // Test requireQuality update
    int prevMin = pipeline.requireQuality(70);
    assert(prevMin == 1);
    assert(pipeline.minimumQuality == 70);

    // Passing negative minimum should be ignored
    pipeline.requireQuality(-5);
    assert(pipeline.minimumQuality == 70);

    // Ingest with quality 60 fails now
    int res4 = pipeline.ingest(3, 50.0, 60);
    assert(res4 == -1);
    assert(pipeline.rejectedCount() == 3);

    std::cout << "Test test_ingest_and_quality: PASS" << std::endl;
}

void test_ingestBatch_and_saturation() {
    SamplePipeline pipeline("BatchTest", 3);

    std::vector<double> readings;
    readings.push_back(10.0);
    readings.push_back(20.0);
    readings.push_back(30.0);
    readings.push_back(40.0); // Should be dropped due to capacity 3

    int acceptedInBatch = pipeline.ingestBatch(readings, 80);
    assert(acceptedInBatch == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 1); // 4th sample exceeded channel capacity
    assert(pipeline.isSaturated());
    assert(isClose(pipeline.channelPeak(), 30.0));
    assert(isClose(pipeline.channelMean(), 20.0));

    std::cout << "Test test_ingestBatch_and_saturation: PASS" << std::endl;
}

void test_normalisation_and_scale() {
    SamplePipeline pipeline("ScaleTest", 5);
    pipeline.scale = UnitScale(2.0, 5.0, "volt");

    // scale: toBase(x) = x * 2.0 + 5.0
    //        fromBase(x) = (x - 5.0) / 2.0
    double norm = pipeline.normalised(15.0); // fromBase: (15 - 5) / 2 = 5
    assert(isClose(norm, 5.0));

    double denorm = pipeline.denormalised(5.0); // toBase: 5 * 2 + 5 = 15
    assert(isClose(denorm, 15.0));

    std::cout << "Test test_normalisation_and_scale: PASS" << std::endl;
}

void test_flush_and_describe() {
    SamplePipeline pipeline("SensX", 5);
    pipeline.scale = UnitScale(1.0, 0.0, "mV");

    assert(pipeline.describe() == "empty");

    pipeline.ingest(1, 10.0, 50);
    pipeline.ingest(2, 20.0, 50);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.describe() == "SensX:mV");

    int flushedCount = pipeline.flush();
    assert(flushedCount == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.channel.isEmpty());
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_flush_and_describe: PASS" << std::endl;
}

int main() {
    std::cout << "Starting SamplePipeline Test Suite..." << std::endl;

    test_constructors_and_defaults();
    test_filter_management();
    test_process_and_stage();
    test_wouldReject();
    test_ingest_and_quality();
    test_ingestBatch_and_saturation();
    test_normalisation_and_scale();
    test_flush_and_describe();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}