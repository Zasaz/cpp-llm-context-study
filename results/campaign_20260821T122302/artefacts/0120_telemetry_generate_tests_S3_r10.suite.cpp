#include "SamplePipeline.h"
#include "Channel.h"
#include "Filter.h"
#include "Sample.h"
#include "UnitScale.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>

// Helper to compare floating point numbers with tolerance
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) <= epsilon;
}

void test_constructors_and_defaults() {
    // Default constructor
    SamplePipeline p1;
    assert(p1.acceptedCount() == 0);
    assert(p1.rejectedCount() == 0);
    assert(p1.minimumQuality == 1);
    assert(p1.filterCount() == 0);
    assert(p1.channel.channelName() == "unnamed");
    assert(p1.channel.capacity == 16);
    assert(!p1.isSaturated());
    assert(p1.validSamples() == 0);
    assert(p1.describe() == "empty");

    // Parameterized constructor
    SamplePipeline p2("sensor_A", 5);
    assert(p2.acceptedCount() == 0);
    assert(p2.rejectedCount() == 0);
    assert(p2.minimumQuality == 1);
    assert(p2.filterCount() == 0);
    assert(p2.channel.channelName() == "sensor_A");
    assert(p2.channel.capacity == 5);
    assert(!p2.isSaturated());
    assert(p2.describe() == "empty");

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_add_and_clear_filters() {
    SamplePipeline pipeline("ch1", 10);

    // Adding null filter should do nothing and return current count
    assert(pipeline.addFilter(nullptr) == 0);
    assert(pipeline.filterCount() == 0);

    PassFilter pf;
    ClampFilter cf(0.0, 10.0);
    SmoothFilter sf(0.5);

    assert(pipeline.addFilter(&pf) == 1);
    assert(pipeline.addFilter(&cf) == 2);
    assert(pipeline.addFilter(&sf) == 3);
    assert(pipeline.filterCount() == 3);

    // Clear filters
    int previousCount = pipeline.clearFilters();
    assert(previousCount == 3);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_add_and_clear_filters: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipeline("ch1", 10);
    ClampFilter cf(0.0, 50.0);
    PassFilter pf;

    pipeline.addFilter(&cf);
    pipeline.addFilter(&pf);

    // stage() bounds checking
    assert(approxEqual(pipeline.stage(100.0, -1), 100.0));
    assert(approxEqual(pipeline.stage(100.0, 2), 100.0));

    // stage() single filter execution
    assert(approxEqual(pipeline.stage(100.0, 0), 50.0)); // clamped
    assert(approxEqual(pipeline.stage(100.0, 1), 100.0)); // pass filter

    // process() pipeline execution
    double processed = pipeline.process(75.0);
    assert(approxEqual(processed, 50.0));

    double processedInBounds = pipeline.process(25.0);
    assert(approxEqual(processedInBounds, 25.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_wouldReject() {
    SamplePipeline pipeline("ch1", 10);
    assert(!pipeline.wouldReject(100.0));

    ClampFilter cf(10.0, 20.0);
    pipeline.addFilter(&cf);

    assert(pipeline.wouldReject(5.0));   // below clamp
    assert(pipeline.wouldReject(25.0));  // above clamp
    assert(!pipeline.wouldReject(15.0)); // within clamp

    std::cout << "Test test_wouldReject: PASS" << std::endl;
}

void test_requireQuality() {
    SamplePipeline pipeline("ch1", 10);
    assert(pipeline.minimumQuality == 1);

    // Set new valid minimum
    int prev = pipeline.requireQuality(50);
    assert(prev == 1);
    assert(pipeline.minimumQuality == 50);

    // Negative value should not change minimumQuality
    prev = pipeline.requireQuality(-5);
    assert(prev == 50);
    assert(pipeline.minimumQuality == 50);

    std::cout << "Test test_requireQuality: PASS" << std::endl;
}

void test_ingest_success_and_filtering() {
    SamplePipeline pipeline("temperature", 5);
    ClampFilter cf(0.0, 100.0);
    pipeline.addFilter(&cf);

    // Ingest valid sample with clamping applied
    int status = pipeline.ingest(1, 150.0, 80);
    assert(status == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 0);

    Sample latestSample = pipeline.latest();
    assert(latestSample.position() == 1);
    assert(approxEqual(latestSample.value(), 100.0)); // Clamped from 150.0
    assert(latestSample.score() == 80);

    // Ingest another valid sample
    status = pipeline.ingest(2, 45.0, 90);
    assert(status == 2);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.channel.count() == 2);

    std::cout << "Test test_ingest_success_and_filtering: PASS" << std::endl;
}

void test_ingest_rejections() {
    SamplePipeline pipeline("voltage", 3);
    ClampFilter cf(0.0, 10.0);
    pipeline.addFilter(&cf);
    pipeline.requireQuality(40);

    // 1. Rejected due to low quality
    int res = pipeline.ingest(1, 5.0, 30);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // 2. Rejected due to wouldReject (ClampFilter rejects values < 0.0 or > 10.0)
    res = pipeline.ingest(2, 15.0, 50);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 0);

    // 3. Rejected due to quality <= 0 leading to invalid Sample (when minQuality allows 0)
    pipeline.requireQuality(0);
    res = pipeline.ingest(3, 5.0, 0); // Quality 0 sample has isValid() == false
    assert(res == -1);
    assert(pipeline.rejectedCount() == 3);

    std::cout << "Test test_ingest_rejections: PASS" << std::endl;
}

void test_ingest_channel_capacity_saturation() {
    SamplePipeline pipeline("buffer", 2); // capacity = 2

    assert(pipeline.ingest(1, 10.0, 80) == 1);
    assert(!pipeline.isSaturated());

    assert(pipeline.ingest(2, 20.0, 85) == 2);
    assert(pipeline.isSaturated());

    // Ingesting beyond capacity should fail
    int res = pipeline.ingest(3, 30.0, 90);
    assert(res == -1);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.isSaturated());

    std::cout << "Test test_ingest_channel_capacity_saturation: PASS" << std::endl;
}

void test_ingestBatch() {
    SamplePipeline pipeline("batch_chan", 10);
    ClampFilter cf(0.0, 50.0);
    pipeline.addFilter(&cf);
    pipeline.requireQuality(20);

    std::vector<double> readings = {10.0, -5.0, 25.0, 60.0, 30.0};
    // -5.0 and 60.0 will be rejected by ClampFilter::rejects
    // 10.0, 25.0, 30.0 should be accepted
    int acceptedCount = pipeline.ingestBatch(readings, 80);
    assert(acceptedCount == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.channel.count() == 3);

    std::cout << "Test test_ingestBatch: PASS" << std::endl;
}

void test_channel_statistics_and_scaling() {
    SamplePipeline pipeline("current", 10);
    pipeline.scale = UnitScale(2.0, 1.0, "mA"); // toBase: x * 2 + 1, fromBase: (x - 1) / 2

    pipeline.ingest(1, 10.0, 80);
    pipeline.ingest(2, 20.0, 90);
    pipeline.ingest(3, 30.0, 70);

    // Channel statistics
    assert(approxEqual(pipeline.channelMean(), 20.0));
    assert(approxEqual(pipeline.channelPeak(), 30.0));
    assert(pipeline.validSamples() == 3);

    // Normalisation and Denormalisation via pipeline scale
    // normalised = fromBase(5.0) = (5.0 - 1.0) / 2.0 = 2.0
    assert(approxEqual(pipeline.normalised(5.0), 2.0));
    // denormalised = toBase(5.0) = 5.0 * 2.0 + 1.0 = 11.0
    assert(approxEqual(pipeline.denormalised(5.0), 11.0));

    // Describe non-empty channel
    assert(pipeline.describe() == "current:mA");

    std::cout << "Test test_channel_statistics_and_scaling: PASS" << std::endl;
}

void test_flush() {
    SamplePipeline pipeline("stream", 5);
    pipeline.scale = UnitScale(1.0, 0.0, "V");

    pipeline.ingest(1, 10.0, 80);
    pipeline.ingest(2, 20.0, 80);
    pipeline.requireQuality(50);
    pipeline.ingest(3, 15.0, 10); // will be rejected

    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.channel.count() == 2);

    int flushedCount = pipeline.flush();
    assert(flushedCount == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.channel.count() == 0);
    assert(pipeline.channel.channelName() == "stream");
    assert(pipeline.channel.capacity == 5);
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_flush: PASS" << std::endl;
}

int main() {
    test_constructors_and_defaults();
    test_add_and_clear_filters();
    test_process_and_stage();
    test_wouldReject();
    test_requireQuality();
    test_ingest_success_and_filtering();
    test_ingest_rejections();
    test_ingest_channel_capacity_saturation();
    test_ingestBatch();
    test_channel_statistics_and_scaling();
    test_flush();

    std::cout << "\nAll SamplePipeline unit tests passed successfully!" << std::endl;
    return 0;
}