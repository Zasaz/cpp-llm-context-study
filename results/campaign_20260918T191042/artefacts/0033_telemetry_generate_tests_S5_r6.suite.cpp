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

// Helper function to compare floating-point values
bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_constructors_and_defaults() {
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
    SamplePipeline customPipeline("Temperature", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.minimumQuality == 1);
    assert(customPipeline.channel.channelName() == "Temperature");
    assert(customPipeline.channel.capacity == 5);
    assert(customPipeline.describe() == "empty");

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline("Sensor", 10);
    PassFilter pf;
    ClampFilter cf(0.0, 100.0);

    assert(pipeline.filterCount() == 0);

    // Adding null filter should not change count
    int count = pipeline.addFilter(nullptr);
    assert(count == 0);
    assert(pipeline.filterCount() == 0);

    // Adding valid filters
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

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipeline("Processing", 10);
    ClampFilter cf(0.0, 50.0);
    SmoothFilter sf(0.5);

    pipeline.addFilter(&cf);
    pipeline.addFilter(&sf);

    // Stage out of range
    assert(approxEqual(pipeline.stage(100.0, -1), 100.0));
    assert(approxEqual(pipeline.stage(100.0, 2), 100.0));

    // Stage 0 (ClampFilter 0..50)
    assert(approxEqual(pipeline.stage(80.0, 0), 50.0));
    assert(approxEqual(pipeline.stage(-10.0, 0), 0.0));

    // Stage 1 (SmoothFilter, weight 0.5) - first application primes it to 20.0
    assert(approxEqual(pipeline.stage(20.0, 1), 20.0));
    // Second application: 0.5 * 40.0 + 0.5 * 20.0 = 30.0
    assert(approxEqual(pipeline.stage(40.0, 1), 30.0));

    // Reset smooth filter
    SmoothFilter sf2(0.5);
    pipeline.clearFilters();
    pipeline.addFilter(&cf);
    pipeline.addFilter(&sf2);

    // Full process pipeline: input 80.0 -> clamped to 50.0 -> smoothed (primed) to 50.0
    double out1 = pipeline.process(80.0);
    assert(approxEqual(out1, 50.0));

    // Next input: 0.0 -> clamped to 0.0 -> smoothed: 0.5 * 0.0 + 0.5 * 50.0 = 25.0
    double out2 = pipeline.process(0.0);
    assert(approxEqual(out2, 25.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_wouldReject() {
    SamplePipeline pipeline("Test", 10);
    ClampFilter cf(10.0, 20.0);
    pipeline.addFilter(&cf);

    assert(!pipeline.wouldReject(15.0));
    assert(pipeline.wouldReject(5.0));
    assert(pipeline.wouldReject(25.0));

    // Disabled filter
    Filter baseFilter;
    baseFilter.toggle(); // disabled
    SamplePipeline pipeline2("Test2", 10);
    pipeline2.addFilter(&baseFilter);

    // Filter::rejects returns (!enabled && reading != 0.0)
    assert(pipeline2.wouldReject(5.0));
    assert(!pipeline2.wouldReject(0.0));

    std::cout << "Test test_wouldReject: PASS" << std::endl;
}

void test_requireQuality() {
    SamplePipeline pipeline("QualityCheck", 10);
    assert(pipeline.minimumQuality == 1);

    int prev = pipeline.requireQuality(50);
    assert(prev == 1);
    assert(pipeline.minimumQuality == 50);

    // Negative requirement should be ignored
    prev = pipeline.requireQuality(-5);
    assert(prev == 50);
    assert(pipeline.minimumQuality == 50);

    std::cout << "Test test_requireQuality: PASS" << std::endl;
}

void test_ingest_and_rejections() {
    SamplePipeline pipeline("IngestTest", 3);
    pipeline.requireQuality(30);

    ClampFilter cf(0.0, 100.0);
    pipeline.addFilter(&cf);

    // 1. Rejected due to quality below minimumQuality
    int res1 = pipeline.ingest(1, 50.0, 20);
    assert(res1 == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // 2. Rejected due to wouldReject (ClampFilter rejects < 0 or > 100)
    int res2 = pipeline.ingest(2, 150.0, 50);
    assert(res2 == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 0);

    // 3. Accepted sample
    int res3 = pipeline.ingest(3, 50.0, 50);
    assert(res3 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 1);

    // 4. Ingest up to capacity (capacity = 3)
    pipeline.ingest(4, 60.0, 80); // Accepted (count = 2)
    pipeline.ingest(5, 70.0, 90); // Accepted (count = 3)
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.isSaturated());

    // 5. Ingest when channel is full (Channel::append returns -1)
    int res4 = pipeline.ingest(6, 80.0, 90);
    assert(res4 == -1);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 3);

    std::cout << "Test test_ingest_and_rejections: PASS" << std::endl;
}

void test_ingestBatch() {
    SamplePipeline pipeline("BatchTest", 10);
    pipeline.requireQuality(10);
    ClampFilter cf(0.0, 50.0);
    pipeline.addFilter(&cf);

    std::vector<double> readings = {10.0, 20.0, 60.0, 40.0, -5.0};
    // Index 0: 10.0 (Accepted)
    // Index 1: 20.0 (Accepted)
    // Index 2: 60.0 (Rejected by clamp filter)
    // Index 3: 40.0 (Accepted)
    // Index 4: -5.0 (Rejected by clamp filter)

    int acceptedInBatch = pipeline.ingestBatch(readings, 50);
    assert(acceptedInBatch == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 3);

    std::cout << "Test test_ingestBatch: PASS" << std::endl;
}

void test_metrics_and_scale_methods() {
    SamplePipeline pipeline("Metrics", 10);
    pipeline.scale = UnitScale(2.0, 5.0, "celsius"); // toBase: x * 2 + 5, fromBase: (x - 5) / 2

    pipeline.ingest(1, 10.0, 80);
    pipeline.ingest(2, 20.0, 80);
    pipeline.ingest(3, 30.0, 80);

    // Channel metrics
    assert(approxEqual(pipeline.channelMean(), 20.0));
    assert(approxEqual(pipeline.channelPeak(), 30.0));

    // Latest sample
    Sample latestSample = pipeline.latest();
    assert(latestSample.position() == 3);
    assert(approxEqual(latestSample.value(), 30.0));
    assert(latestSample.score() == 80);

    // Normalisation and Denormalisation
    // fromBase(25.0) -> (25.0 - 5.0) / 2.0 = 10.0
    assert(approxEqual(pipeline.normalised(25.0), 10.0));
    // toBase(10.0) -> 10.0 * 2.0 + 5.0 = 25.0
    assert(approxEqual(pipeline.denormalised(10.0), 25.0));

    std::cout << "Test test_metrics_and_scale_methods: PASS" << std::endl;
}

void test_flush_and_describe() {
    SamplePipeline pipeline("Voltage", 5);
    pipeline.scale = UnitScale(1.0, 0.0, "V");

    assert(pipeline.describe() == "empty");

    pipeline.ingest(1, 12.0, 90);
    pipeline.ingest(2, 12.5, 95);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.describe() == "Voltage:V");

    int previousCount = pipeline.flush();
    assert(previousCount == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.channel.isEmpty());
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_flush_and_describe: PASS" << std::endl;
}

int main() {
    std::cout << "Running SamplePipeline test suite..." << std::endl;

    test_constructors_and_defaults();
    test_filter_management();
    test_process_and_stage();
    test_wouldReject();
    test_requireQuality();
    test_ingest_and_rejections();
    test_ingestBatch();
    test_metrics_and_scale_methods();
    test_flush_and_describe();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}