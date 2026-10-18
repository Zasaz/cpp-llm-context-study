#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>

#include "SamplePipeline.h"

// Helper function for floating point comparisons
static bool doubleEquals(double a, double b, double epsilon = 1e-5) {
    return std::fabs(a - b) < epsilon;
}

// Test Filter implementation to exercise Filter integration in SamplePipeline
class MultiplierFilter : public Filter {
public:
    double multiplier;
    bool shouldReject;

    explicit MultiplierFilter(double factor = 1.0, bool reject = false)
        : multiplier(factor), shouldReject(reject) {}

    double apply(double reading) override {
        return reading * multiplier;
    }

    bool rejects(double /*reading*/) const override {
        return shouldReject;
    }
};

// Threshold Filter implementation
class ThresholdFilter : public Filter {
public:
    double maxThreshold;

    explicit ThresholdFilter(double maxVal) : maxThreshold(maxVal) {}

    double apply(double reading) override {
        return reading;
    }

    bool rejects(double reading) const override {
        return reading > maxThreshold;
    }
};

void test_constructors_and_initial_state() {
    // Default constructor
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.validSamples() == 0);
    assert(!defaultPipeline.isSaturated());
    assert(defaultPipeline.describe() == "empty");

    // Parameterized constructor
    SamplePipeline customPipeline("TemperatureChannel", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.validSamples() == 0);
    assert(!customPipeline.isSaturated());
    assert(customPipeline.describe() == "empty");

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline("TestChannel", 10);

    MultiplierFilter f1(2.0);
    MultiplierFilter f2(3.0);

    // Null filter handling
    assert(pipeline.addFilter(nullptr) == 0);
    assert(pipeline.filterCount() == 0);

    // Add valid filters
    assert(pipeline.addFilter(&f1) == 1);
    assert(pipeline.filterCount() == 1);
    assert(pipeline.addFilter(&f2) == 2);
    assert(pipeline.filterCount() == 2);

    // Clear filters
    int previousCount = pipeline.clearFilters();
    assert(previousCount == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stages() {
    SamplePipeline pipeline("TestChannel", 10);
    MultiplierFilter f1(2.0);
    MultiplierFilter f2(3.0);

    pipeline.addFilter(&f1);
    pipeline.addFilter(&f2);

    // Filter chaining: 5.0 * 2.0 * 3.0 = 30.0
    double processed = pipeline.process(5.0);
    assert(doubleEquals(processed, 30.0));

    // Individual stage testing
    assert(doubleEquals(pipeline.stage(5.0, 0), 10.0)); // 5.0 * 2.0
    assert(doubleEquals(pipeline.stage(5.0, 1), 15.0)); // 5.0 * 3.0

    // Out-of-bounds stage index returns original reading
    assert(doubleEquals(pipeline.stage(5.0, -1), 5.0));
    assert(doubleEquals(pipeline.stage(5.0, 2), 5.0));

    std::cout << "Test test_process_and_stages: PASS" << std::endl;
}

void test_quality_and_rejection_rules() {
    SamplePipeline pipeline("PressureChannel", 10);

    // Default minimum quality is 1
    int prevMinQuality = pipeline.requireQuality(5);
    assert(prevMinQuality == 1);

    // Ingesting sample with quality < minimumQuality
    int res1 = pipeline.ingest(1, 100.0, 4);
    assert(res1 == -1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 1);

    // Ingesting sample with quality >= minimumQuality
    int res2 = pipeline.ingest(2, 100.0, 5);
    assert(res2 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 1);

    // Invalid minimum quality argument (negative value shouldn't update)
    prevMinQuality = pipeline.requireQuality(-1);
    assert(prevMinQuality == 5);
    assert(pipeline.requireQuality(5) == 5);

    // Filter-based rejection
    ThresholdFilter maxLimit(50.0);
    pipeline.addFilter(&maxLimit);

    assert(pipeline.wouldReject(60.0) == true);
    assert(pipeline.wouldReject(40.0) == false);

    // Ingesting value that exceeds filter threshold
    int res3 = pipeline.ingest(3, 60.0, 5);
    assert(res3 == -1);
    assert(pipeline.rejectedCount() == 2);

    std::cout << "Test test_quality_and_rejection_rules: PASS" << std::endl;
}

void test_batch_ingest_and_capacity_limits() {
    const int capacity = 3;
    SamplePipeline pipeline("Sensors", capacity);
    pipeline.requireQuality(1);

    std::vector<double> readings = {10.0, 20.0, 30.0, 40.0, 50.0};
    int taken = pipeline.ingestBatch(readings, 2);

    // Only 3 should be accepted due to channel capacity
    assert(taken == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.isSaturated());
    assert(pipeline.validSamples() == 3);

    std::cout << "Test test_batch_ingest_and_capacity_limits: PASS" << std::endl;
}

void test_channel_statistics_and_latest() {
    SamplePipeline pipeline("Voltage", 5);
    pipeline.requireQuality(1);

    pipeline.ingest(1, 10.0, 1);
    pipeline.ingest(2, 20.0, 1);
    pipeline.ingest(3, 30.0, 1);

    assert(doubleEquals(pipeline.channelMean(), 20.0));
    assert(doubleEquals(pipeline.channelPeak(), 30.0));
    assert(pipeline.validSamples() == 3);

    Sample latestSample = pipeline.latest();
    assert(latestSample.sequence == 2); // 0-indexed in ingestBatch or sequence passed in ingest
    assert(doubleEquals(latestSample.value, 30.0));
    assert(latestSample.quality == 1);

    std::cout << "Test test_channel_statistics_and_latest: PASS" << std::endl;
}

void test_scale_conversions_and_describe() {
    SamplePipeline pipeline("FlowRate", 5);

    // Initially empty
    assert(pipeline.describe() == "empty");

    // Normalisation and Denormalisation checks
    double val = 42.0;
    double norm = pipeline.normalised(val);
    double denorm = pipeline.denormalised(norm);
    assert(doubleEquals(val, denorm));

    // After ingest, description should reflect channel and unit name
    pipeline.ingest(1, 25.0, 1);
    std::string desc = pipeline.describe();
    assert(desc != "empty");
    assert(desc.find("FlowRate") != std::string::npos);

    std::cout << "Test test_scale_conversions_and_describe: PASS" << std::endl;
}

void test_flush() {
    SamplePipeline pipeline("Current", 5);
    pipeline.ingest(1, 15.0, 1);
    pipeline.ingest(2, 25.0, 1);
    pipeline.ingest(3, 35.0, 0); // Rejected due to default minQuality = 1

    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);

    int flushedCount = pipeline.flush();
    assert(flushedCount == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_flush: PASS" << std::endl;
}

int main() {
    std::cout << "Starting SamplePipeline unit tests..." << std::endl;

    test_constructors_and_initial_state();
    test_filter_management();
    test_process_and_stages();
    test_quality_and_rejection_rules();
    test_batch_ingest_and_capacity_limits();
    test_channel_statistics_and_latest();
    test_scale_conversions_and_describe();
    test_flush();

    std::cout << "All SamplePipeline unit tests passed successfully!" << std::endl;
    return 0;
}