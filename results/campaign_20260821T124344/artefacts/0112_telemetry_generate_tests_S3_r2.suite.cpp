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
bool double_equals(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Test default and parameterized constructors and initial state
void test_SamplePipeline_constructors() {
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.minimumQuality == 1);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.channel.channelName() == "unnamed");
    assert(defaultPipeline.channel.capacity == 16);
    assert(defaultPipeline.describe() == "empty");

    SamplePipeline customPipeline("temperature", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.minimumQuality == 1);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.channel.channelName() == "temperature");
    assert(customPipeline.channel.capacity == 5);
    assert(customPipeline.describe() == "empty");

    std::cout << "Test test_SamplePipeline_constructors: PASS" << std::endl;
}

// Test filter management: addFilter, filterCount, clearFilters
void test_SamplePipeline_filterManagement() {
    SamplePipeline pipeline("ch1", 10);
    assert(pipeline.filterCount() == 0);

    PassFilter pf;
    ClampFilter cf(0.0, 10.0);

    // Null pointer check
    int count = pipeline.addFilter(nullptr);
    assert(count == 0);
    assert(pipeline.filterCount() == 0);

    // Add valid filters
    count = pipeline.addFilter(&pf);
    assert(count == 1);
    assert(pipeline.filterCount() == 1);

    count = pipeline.addFilter(&cf);
    assert(count == 2);
    assert(pipeline.filterCount() == 2);

    // Clear filters
    int cleared = pipeline.clearFilters();
    assert(cleared == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_SamplePipeline_filterManagement: PASS" << std::endl;
}

// Test process and stage methods through filter chains
void test_SamplePipeline_processAndStage() {
    SamplePipeline pipeline("ch1", 10);

    ClampFilter clamp(0.0, 50.0);
    SmoothFilter smooth(0.5); // previous = reading on first apply; subsequent = 0.5 * reading + 0.5 * prev

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // Stage out of range
    assert(double_equals(pipeline.stage(100.0, -1), 100.0));
    assert(double_equals(pipeline.stage(100.0, 2), 100.0));

    // Test stage at index 0 (clamp only, without affecting smooth filter)
    assert(double_equals(pipeline.stage(100.0, 0), 50.0));
    assert(double_equals(pipeline.stage(-10.0, 0), 0.0));

    // Test full process: 1st reading = 100.0 -> clamped to 50.0 -> smooth primed with 50.0
    double out1 = pipeline.process(100.0);
    assert(double_equals(out1, 50.0));

    // 2nd reading = 0.0 -> clamped to 0.0 -> smooth calculates: 0.5 * 0.0 + 0.5 * 50.0 = 25.0
    double out2 = pipeline.process(0.0);
    assert(double_equals(out2, 25.0));

    std::cout << "Test test_SamplePipeline_processAndStage: PASS" << std::endl;
}

// Test wouldReject check based on filters
void test_SamplePipeline_wouldReject() {
    SamplePipeline pipeline("ch1", 10);
    ClampFilter clamp(10.0, 20.0);
    pipeline.addFilter(&clamp);

    assert(!pipeline.wouldReject(15.0));
    assert(pipeline.wouldReject(5.0));
    assert(pipeline.wouldReject(25.0));

    // Test filter rejection when filter is disabled
    PassFilter pass;
    pass.toggle(); // enabled = false
    SamplePipeline passPipeline("ch2", 10);
    passPipeline.addFilter(&pass);

    assert(passPipeline.wouldReject(10.0));
    // When reading is 0.0, Filter::rejects returns false even if disabled (!enabled && reading != 0.0)
    assert(!passPipeline.wouldReject(0.0));

    std::cout << "Test test_SamplePipeline_wouldReject: PASS" << std::endl;
}

// Test ingest with various conditions (quality, reject, capacity)
void test_SamplePipeline_ingest() {
    SamplePipeline pipeline("ch1", 2);
    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);
    pipeline.requireQuality(50);

    // Ingest with low quality (below minimumQuality)
    int res = pipeline.ingest(1, 50.0, 40);
    assert(res == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 1);

    // Ingest with rejected reading (out of clamp range)
    res = pipeline.ingest(2, 150.0, 80);
    assert(res == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 2);

    // Valid ingest 1
    res = pipeline.ingest(3, 50.0, 80);
    assert(res == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.channel.count() == 1);

    // Valid ingest 2
    res = pipeline.ingest(4, 75.0, 90);
    assert(res == 2);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.channel.count() == 2);
    assert(pipeline.isSaturated());

    // Ingest when channel capacity is full (capacity = 2)
    res = pipeline.ingest(5, 60.0, 80);
    assert(res == -1);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 3);

    std::cout << "Test test_SamplePipeline_ingest: PASS" << std::endl;
}

// Test batch ingestion
void test_SamplePipeline_ingestBatch() {
    SamplePipeline pipeline("ch1", 5);
    ClampFilter clamp(0.0, 50.0);
    pipeline.addFilter(&clamp);

    std::vector<double> batch = { 10.0, 60.0, 20.0, -5.0, 30.0 }; // 60.0 and -5.0 will be rejected by clamp
    int accepted = pipeline.ingestBatch(batch, 80);

    assert(accepted == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.channel.count() == 3);

    std::cout << "Test test_SamplePipeline_ingestBatch: PASS" << std::endl;
}

// Test quality requirement manipulation
void test_SamplePipeline_requireQuality() {
    SamplePipeline pipeline("ch1", 5);
    assert(pipeline.minimumQuality == 1);

    int prev = pipeline.requireQuality(60);
    assert(prev == 1);
    assert(pipeline.minimumQuality == 60);

    // Negative values should be ignored
    prev = pipeline.requireQuality(-10);
    assert(prev == 60);
    assert(pipeline.minimumQuality == 60);

    std::cout << "Test test_SamplePipeline_requireQuality: PASS" << std::endl;
}

// Test statistical and state queries (channelMean, channelPeak, validSamples, isSaturated, latest)
void test_SamplePipeline_channelQueries() {
    SamplePipeline pipeline("sensor", 3);
    assert(double_equals(pipeline.channelMean(), 0.0));
    assert(double_equals(pipeline.channelPeak(), 0.0));
    assert(pipeline.validSamples() == 0);
    assert(!pipeline.isSaturated());

    pipeline.ingest(1, 10.0, 80);
    pipeline.ingest(2, 30.0, 90);
    pipeline.ingest(3, 20.0, 70);

    assert(double_equals(pipeline.channelMean(), 20.0));
    assert(double_equals(pipeline.channelPeak(), 30.0));
    assert(pipeline.validSamples() == 3);
    assert(pipeline.isSaturated());

    Sample latest = pipeline.latest();
    assert(latest.position() == 3);
    assert(double_equals(latest.value(), 20.0));
    assert(latest.score() == 70);

    std::cout << "Test test_SamplePipeline_channelQueries: PASS" << std::endl;
}

// Test normalisation and denormalisation with UnitScale
void test_SamplePipeline_scaling() {
    SamplePipeline pipeline("ch1", 5);
    pipeline.scale = UnitScale(2.0, 10.0, "mV"); // toBase(x) = 2*x + 10; fromBase(x) = (x - 10) / 2

    // normalised calls scale.fromBase
    double norm = pipeline.normalised(30.0);
    assert(double_equals(norm, 10.0));

    // denormalised calls scale.toBase
    double denorm = pipeline.denormalised(10.0);
    assert(double_equals(denorm, 30.0));

    std::cout << "Test test_SamplePipeline_scaling: PASS" << std::endl;
}

// Test flush method resetting pipeline and channel state
void test_SamplePipeline_flush() {
    SamplePipeline pipeline("sensorA", 4);
    pipeline.ingest(1, 10.0, 80);
    pipeline.ingest(2, 20.0, 80);
    pipeline.ingest(3, 100.0, 0); // rejected due to quality < 1

    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.channel.count() == 2);

    int flushedCount = pipeline.flush();
    assert(flushedCount == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.channel.count() == 0);
    assert(pipeline.channel.channelName() == "sensorA");
    assert(pipeline.channel.capacity == 4);

    std::cout << "Test test_SamplePipeline_flush: PASS" << std::endl;
}

// Test describe formatting
void test_SamplePipeline_describe() {
    SamplePipeline pipeline("pressure", 5);
    pipeline.scale = UnitScale(1.0, 0.0, "kPa");

    // Empty channel
    assert(pipeline.describe() == "empty");

    // With samples
    pipeline.ingest(1, 101.3, 100);
    assert(pipeline.describe() == "pressure:kPa");

    std::cout << "Test test_SamplePipeline_describe: PASS" << std::endl;
}

int main() {
    std::cout << "Starting SamplePipeline Test Suite..." << std::endl;

    test_SamplePipeline_constructors();
    test_SamplePipeline_filterManagement();
    test_SamplePipeline_processAndStage();
    test_SamplePipeline_wouldReject();
    test_SamplePipeline_ingest();
    test_SamplePipeline_ingestBatch();
    test_SamplePipeline_requireQuality();
    test_SamplePipeline_channelQueries();
    test_SamplePipeline_scaling();
    test_SamplePipeline_flush();
    test_SamplePipeline_describe();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}