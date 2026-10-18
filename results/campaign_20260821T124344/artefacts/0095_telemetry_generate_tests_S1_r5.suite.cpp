#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>

#include "SamplePipeline.h"
#include "Filter.h"
#include "Sample.h"
#include "UnitScale.h"
#include "Channel.h"

// Helper function for floating point comparisons
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) <= epsilon;
}

// Test default and parameterized constructor initialization
void test_SamplePipeline_constructors() {
    SamplePipeline defaultPipe;
    assert(defaultPipe.acceptedCount() == 0);
    assert(defaultPipe.rejectedCount() == 0);
    assert(defaultPipe.filterCount() == 0);
    assert(defaultPipe.validSamples() == 0);
    assert(!defaultPipe.isSaturated());
    assert(defaultPipe.describe() == "empty");
    assert(defaultPipe.channel.channelName() == "unnamed");
    assert(defaultPipe.channel.capacity == 16);

    SamplePipeline paramPipe("temperature", 5);
    assert(paramPipe.acceptedCount() == 0);
    assert(paramPipe.rejectedCount() == 0);
    assert(paramPipe.filterCount() == 0);
    assert(paramPipe.channel.channelName() == "temperature");
    assert(paramPipe.channel.capacity == 5);
    assert(!paramPipe.isSaturated());

    std::cout << "Test test_SamplePipeline_constructors: PASS" << std::endl;
}

// Test adding filters, null filter handling, and clearing filters
void test_SamplePipeline_filterManagement() {
    SamplePipeline pipe("test", 10);
    assert(pipe.filterCount() == 0);

    // Null filter pointer should be ignored
    int countAfterNull = pipe.addFilter(nullptr);
    assert(countAfterNull == 0);
    assert(pipe.filterCount() == 0);

    PassFilter pf;
    ClampFilter cf(0.0, 100.0);
    SmoothFilter sf(0.5);

    assert(pipe.addFilter(&pf) == 1);
    assert(pipe.addFilter(&cf) == 2);
    assert(pipe.addFilter(&sf) == 3);
    assert(pipe.filterCount() == 3);

    int clearedCount = pipe.clearFilters();
    assert(clearedCount == 3);
    assert(pipe.filterCount() == 0);

    std::cout << "Test test_SamplePipeline_filterManagement: PASS" << std::endl;
}

// Test multi-stage processing and single-stage inspection
void test_SamplePipeline_processAndStage() {
    SamplePipeline pipe("sensor", 10);
    
    // clamp reading between 10.0 and 50.0
    ClampFilter clamp(10.0, 50.0);
    // pass filter (no-op)
    PassFilter pass;

    pipe.addFilter(&clamp);
    pipe.addFilter(&pass);

    // Testing stage() with valid and out-of-bounds indices
    assert(approxEqual(pipe.stage(5.0, 0), 10.0)); // stage 0 applies clamp
    assert(approxEqual(pipe.stage(5.0, 1), 5.0));  // stage 1 applies pass
    assert(approxEqual(pipe.stage(5.0, -1), 5.0)); // invalid negative index -> returns input
    assert(approxEqual(pipe.stage(5.0, 2), 5.0));  // invalid out-of-bounds index -> returns input

    // Full processing
    assert(approxEqual(pipe.process(5.0), 10.0));
    assert(approxEqual(pipe.process(25.0), 25.0));
    assert(approxEqual(pipe.process(100.0), 50.0));

    std::cout << "Test test_SamplePipeline_processAndStage: PASS" << std::endl;
}

// Test wouldReject behavior based on registered filters
void test_SamplePipeline_wouldReject() {
    SamplePipeline pipe("sensor", 10);
    ClampFilter clamp(0.0, 10.0);
    pipe.addFilter(&clamp);

    assert(!pipe.wouldReject(5.0));
    assert(pipe.wouldReject(-1.0));
    assert(pipe.wouldReject(11.0));

    // Disable filter check via toggle
    Filter baseFilter;
    baseFilter.toggle(); // enabled becomes false
    SamplePipeline pipe2("sensor2", 10);
    pipe2.addFilter(&baseFilter);
    assert(pipe2.wouldReject(5.0));     // Rejects non-zero if disabled
    assert(!pipe2.wouldReject(0.0));    // 0.0 is not rejected even if disabled

    std::cout << "Test test_SamplePipeline_wouldReject: PASS" << std::endl;
}

// Test ingestion logic: quality gating, filter rejection, capacity limits
void test_SamplePipeline_ingest() {
    SamplePipeline pipe("voltage", 3);
    ClampFilter clamp(0.0, 100.0);
    pipe.addFilter(&clamp);

    // Default min quality is 1
    // Ingest valid sample
    int res = pipe.ingest(1, 50.0, 80);
    assert(res == 1);
    assert(pipe.acceptedCount() == 1);
    assert(pipe.rejectedCount() == 0);
    assert(pipe.validSamples() == 1);

    // Ingest sample failing quality threshold
    res = pipe.ingest(2, 50.0, 0); // quality < minimumQuality (1)
    assert(res == -1);
    assert(pipe.acceptedCount() == 1);
    assert(pipe.rejectedCount() == 1);

    // Ingest sample rejected by filter
    res = pipe.ingest(3, 150.0, 90); // ClampFilter rejects reading > 100
    assert(res == -1);
    assert(pipe.acceptedCount() == 1);
    assert(pipe.rejectedCount() == 2);

    // Ingest sample with custom requireQuality
    int prevMin = pipe.requireQuality(70);
    assert(prevMin == 1);
    // Negative minimum quality is ignored
    assert(pipe.requireQuality(-5) == 70);

    // Quality 60 is now below 70 requirement
    res = pipe.ingest(4, 30.0, 60);
    assert(res == -1);
    assert(pipe.rejectedCount() == 3);

    // Ingest second valid sample
    res = pipe.ingest(5, 40.0, 75);
    assert(res == 2);
    assert(pipe.acceptedCount() == 2);

    // Ingest third valid sample (fills capacity of 3)
    res = pipe.ingest(6, 60.0, 85);
    assert(res == 3);
    assert(pipe.acceptedCount() == 3);
    assert(pipe.isSaturated());

    // Ingesting when channel is full should reject
    res = pipe.ingest(7, 50.0, 90);
    assert(res == -1);
    assert(pipe.rejectedCount() == 4);
    assert(pipe.acceptedCount() == 3);

    std::cout << "Test test_SamplePipeline_ingest: PASS" << std::endl;
}

// Test batch ingestion helper
void test_SamplePipeline_ingestBatch() {
    SamplePipeline pipe("batch_chan", 10);
    ClampFilter clamp(0.0, 50.0);
    pipe.addFilter(&clamp);

    std::vector<double> readings = {10.0, 20.0, 60.0, 30.0, -5.0, 40.0};
    // 10.0 (ok), 20.0 (ok), 60.0 (reject), 30.0 (ok), -5.0 (reject), 40.0 (ok)
    int accepted = pipe.ingestBatch(readings, 80);
    assert(accepted == 4);
    assert(pipe.acceptedCount() == 4);
    assert(pipe.rejectedCount() == 2);
    assert(pipe.validSamples() == 4);

    std::cout << "Test test_SamplePipeline_ingestBatch: PASS" << std::endl;
}

// Test channel metrics query (mean, peak, latest)
void test_SamplePipeline_channelMetrics() {
    SamplePipeline pipe("metrics", 5);
    assert(approxEqual(pipe.channelMean(), 0.0));
    assert(approxEqual(pipe.channelPeak(), 0.0));

    pipe.ingest(1, 10.0, 80);
    pipe.ingest(2, 30.0, 80);
    pipe.ingest(3, 20.0, 80);

    assert(approxEqual(pipe.channelMean(), 20.0));
    assert(approxEqual(pipe.channelPeak(), 30.0));

    Sample latest = pipe.latest();
    assert(latest.position() == 3);
    assert(approxEqual(latest.value(), 20.0));
    assert(latest.score() == 80);

    std::cout << "Test test_SamplePipeline_channelMetrics: PASS" << std::endl;
}

// Test normalisation and denormalisation using UnitScale
void test_SamplePipeline_scaling() {
    SamplePipeline pipe("temp_c", 5);
    // Scale: y = 1.8 * x + 32 (Celsius to Fahrenheit)
    pipe.scale = UnitScale(1.8, 32.0, "degF");

    // normalised (fromBase): (reading - 32) / 1.8
    assert(approxEqual(pipe.normalised(212.0), 100.0));
    assert(approxEqual(pipe.normalised(32.0), 0.0));

    // denormalised (toBase): reading * 1.8 + 32
    assert(approxEqual(pipe.denormalised(100.0), 212.0));
    assert(approxEqual(pipe.denormalised(0.0), 32.0));

    std::cout << "Test test_SamplePipeline_scaling: PASS" << std::endl;
}

// Test flush method resetting channel and statistics
void test_SamplePipeline_flush() {
    SamplePipeline pipe("chan", 5);
    pipe.ingest(1, 10.0, 80);
    pipe.ingest(2, 20.0, 80);
    pipe.ingest(3, 30.0, 0); // rejected

    assert(pipe.acceptedCount() == 2);
    assert(pipe.rejectedCount() == 1);
    assert(pipe.channel.count() == 2);

    int flushed = pipe.flush();
    assert(flushed == 2);
    assert(pipe.acceptedCount() == 0);
    assert(pipe.rejectedCount() == 0);
    assert(pipe.channel.count() == 0);
    assert(pipe.describe() == "empty");

    std::cout << "Test test_SamplePipeline_flush: PASS" << std::endl;
}

// Test describe representation
void test_SamplePipeline_describe() {
    SamplePipeline pipe("pressure", 5);
    assert(pipe.describe() == "empty");

    pipe.scale = UnitScale(1.0, 0.0, "psi");
    pipe.ingest(1, 14.7, 90);

    assert(pipe.describe() == "pressure:psi");

    std::cout << "Test test_SamplePipeline_describe: PASS" << std::endl;
}

int main() {
    std::cout << "Running SamplePipeline unit tests..." << std::endl;

    test_SamplePipeline_constructors();
    test_SamplePipeline_filterManagement();
    test_SamplePipeline_processAndStage();
    test_SamplePipeline_wouldReject();
    test_SamplePipeline_ingest();
    test_SamplePipeline_ingestBatch();
    test_SamplePipeline_channelMetrics();
    test_SamplePipeline_scaling();
    test_SamplePipeline_flush();
    test_SamplePipeline_describe();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}