#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cassert>

#include "SamplePipeline.h"

// Concrete test filter implementation for testing pipeline behavior
class TestOffsetFilter : public Filter {
private:
    double offset;
    double rejectThreshold;

public:
    TestOffsetFilter(double off = 0.0, double rejectAbove = 1000.0)
        : offset(off), rejectThreshold(rejectAbove) {}

    double apply(double value) override {
        return value + offset;
    }

    bool rejects(double value) const override {
        return value > rejectThreshold;
    }
};

static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_default_constructor_and_initial_state() {
    SamplePipeline pipeline;

    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.filterCount() == 0);
    assert(pipeline.describe() == "empty");
    assert(!pipeline.isSaturated());

    std::cout << "Test test_default_constructor_and_initial_state: PASS" << std::endl;
}

void test_parameterized_constructor() {
    SamplePipeline pipeline("Temperature", 5);

    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.filterCount() == 0);
    assert(pipeline.describe() == "empty");
    assert(!pipeline.isSaturated());

    std::cout << "Test test_parameterized_constructor: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline;

    assert(pipeline.filterCount() == 0);

    // Adding null pointer should do nothing and return 0
    int count = pipeline.addFilter(nullptr);
    assert(count == 0);
    assert(pipeline.filterCount() == 0);

    TestOffsetFilter f1(10.0, 500.0);
    TestOffsetFilter f2(5.0, 500.0);

    assert(pipeline.addFilter(&f1) == 1);
    assert(pipeline.filterCount() == 1);

    assert(pipeline.addFilter(&f2) == 2);
    assert(pipeline.filterCount() == 2);

    int cleared = pipeline.clearFilters();
    assert(cleared == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipeline;
    TestOffsetFilter f1(10.0);
    TestOffsetFilter f2(2.5);

    pipeline.addFilter(&f1);
    pipeline.addFilter(&f2);

    // Filter 1 adds 10, Filter 2 adds 2.5 => total +12.5
    double processed = pipeline.process(5.0);
    assert(approxEqual(processed, 17.5));

    // Stage out of bounds
    assert(approxEqual(pipeline.stage(5.0, -1), 5.0));
    assert(approxEqual(pipeline.stage(5.0, 2), 5.0));

    // Valid stages
    assert(approxEqual(pipeline.stage(5.0, 0), 15.0));
    assert(approxEqual(pipeline.stage(5.0, 1), 7.5));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_would_reject() {
    SamplePipeline pipeline;
    TestOffsetFilter f1(0.0, 100.0);
    TestOffsetFilter f2(0.0, 50.0);

    pipeline.addFilter(&f1);
    pipeline.addFilter(&f2);

    assert(!pipeline.wouldReject(25.0));
    assert(pipeline.wouldReject(75.0));  // Rejected by f2
    assert(pipeline.wouldReject(150.0)); // Rejected by f1 and f2

    std::cout << "Test test_would_reject: PASS" << std::endl;
}

void test_require_quality() {
    SamplePipeline pipeline;

    // Default minimum quality is 1
    int prev = pipeline.requireQuality(3);
    assert(prev == 1);

    // Ingesting sample with quality lower than 3 should fail
    int res = pipeline.ingest(0, 10.0, 2);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Negative quality change is ignored
    prev = pipeline.requireQuality(-5);
    assert(prev == 3);

    // Sample with quality >= 3 should succeed
    res = pipeline.ingest(1, 10.0, 3);
    assert(res == 1);
    assert(pipeline.acceptedCount() == 1);

    std::cout << "Test test_require_quality: PASS" << std::endl;
}

void test_ingest_and_rejection_rules() {
    SamplePipeline pipeline("Sensor", 2);
    TestOffsetFilter filter(5.0, 100.0);
    pipeline.addFilter(&filter);

    // 1. Rejected due to quality < minimumQuality (default 1)
    int res1 = pipeline.ingest(0, 10.0, 0);
    assert(res1 == -1);
    assert(pipeline.rejectedCount() == 1);

    // 2. Rejected due to filter threshold (> 100.0)
    int res2 = pipeline.ingest(1, 150.0, 1);
    assert(res2 == -1);
    assert(pipeline.rejectedCount() == 2);

    // 3. Accepted: reading 20.0 -> filtered to 25.0
    int res3 = pipeline.ingest(2, 20.0, 1);
    assert(res3 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(approxEqual(pipeline.latest().reading, 25.0));

    // 4. Accepted second sample
    int res4 = pipeline.ingest(3, 30.0, 2);
    assert(res4 == 2);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.isSaturated());

    // 5. Channel capacity is 2, third sample should be rejected due to full channel
    int res5 = pipeline.ingest(4, 40.0, 1);
    assert(res5 == -1);
    assert(pipeline.rejectedCount() == 3);

    std::cout << "Test test_ingest_and_rejection_rules: PASS" << std::endl;
}

void test_ingest_batch() {
    SamplePipeline pipeline("BatchSensor", 10);
    TestOffsetFilter filter(0.0, 50.0);
    pipeline.addFilter(&filter);

    std::vector<double> readings = {10.0, 60.0, 20.0, 80.0, 30.0};
    int taken = pipeline.ingestBatch(readings, 1);

    // 10.0, 20.0, 30.0 should be accepted (3 items)
    // 60.0, 80.0 should be rejected (2 items)
    assert(taken == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 3);

    std::cout << "Test test_ingest_batch: PASS" << std::endl;
}

void test_channel_statistics_and_describe() {
    SamplePipeline pipeline("Voltage", 5);
    TestOffsetFilter filter(2.0, 100.0);
    pipeline.addFilter(&filter);

    pipeline.ingest(1, 10.0, 1); // filtered: 12.0
    pipeline.ingest(2, 20.0, 1); // filtered: 22.0
    pipeline.ingest(3, 30.0, 1); // filtered: 32.0

    assert(approxEqual(pipeline.channelMean(), (12.0 + 22.0 + 32.0) / 3.0));
    assert(approxEqual(pipeline.channelPeak(), 32.0));
    assert(pipeline.validSamples() == 3);

    std::string desc = pipeline.describe();
    assert(desc.rfind("Voltage:", 0) == 0);

    std::cout << "Test test_channel_statistics_and_describe: PASS" << std::endl;
}

void test_scaling_methods() {
    SamplePipeline pipeline;
    double raw = 100.0;
    double norm = pipeline.normalised(raw);
    double denorm = pipeline.denormalised(norm);

    assert(approxEqual(denorm, raw));

    std::cout << "Test test_scaling_methods: PASS" << std::endl;
}

void test_flush() {
    SamplePipeline pipeline("Temp", 5);
    pipeline.ingest(1, 10.0, 1);
    pipeline.ingest(2, 20.0, 1);
    pipeline.ingest(3, 100.0, 0); // rejected

    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.validSamples() == 2);

    int flushed = pipeline.flush();
    assert(flushed == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_flush: PASS" << std::endl;
}

int main() {
    std::cout << "Starting SamplePipeline unit tests..." << std::endl;

    test_default_constructor_and_initial_state();
    test_parameterized_constructor();
    test_filter_management();
    test_process_and_stage();
    test_would_reject();
    test_require_quality();
    test_ingest_and_rejection_rules();
    test_ingest_batch();
    test_channel_statistics_and_describe();
    test_scaling_methods();
    test_flush();

    std::cout << "All SamplePipeline unit tests completed successfully!" << std::endl;
    return 0;
}