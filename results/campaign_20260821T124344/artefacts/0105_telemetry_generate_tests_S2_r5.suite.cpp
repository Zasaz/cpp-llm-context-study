#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cmath>

#include "SamplePipeline.h"
#include "Filter.h"
#include "Sample.h"
#include "Channel.h"

// Helper dummy filter for testing pipeline filter manipulation and rejection
class MockMultiplierFilter : public Filter {
public:
    double factor;
    double rejectAbove;

    MockMultiplierFilter(double f = 2.0, double max = 1000.0) : factor(f), rejectAbove(max) {}

    double apply(double value) const /* or override depending on interface */ {
        return value * factor;
    }

    bool rejects(double value) const /* or override */ {
        return value > rejectAbove;
    }
};

class MockOffsetFilter : public Filter {
public:
    double offset;

    MockOffsetFilter(double off = 5.0) : offset(off) {}

    double apply(double value) const {
        return value + offset;
    }

    bool rejects(double /*value*/) const {
        return false;
    }
};

void test_constructor_defaults() {
    SamplePipeline pipeline;
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.filterCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.describe() == "empty");
    std::cout << "Test test_constructor_defaults: PASS" << std::endl;
}

void test_constructor_parameterized() {
    SamplePipeline pipeline("Voltage", 10);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.filterCount() == 0);
    assert(!pipeline.isSaturated());
    assert(pipeline.describe() == "empty");
    std::cout << "Test test_constructor_parameterized: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline;
    MockMultiplierFilter f1(2.0);
    MockOffsetFilter f2(10.0);

    // Test adding null pointer
    assert(pipeline.addFilter(nullptr) == 0);
    assert(pipeline.filterCount() == 0);

    // Test adding valid filters
    assert(pipeline.addFilter(&f1) == 1);
    assert(pipeline.filterCount() == 1);
    assert(pipeline.addFilter(&f2) == 2);
    assert(pipeline.filterCount() == 2);

    // Test clearing filters
    int prevCount = pipeline.clearFilters();
    assert(prevCount == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipeline;
    MockMultiplierFilter f1(2.0); // multiplies by 2
    MockOffsetFilter f2(5.0);     // adds 5

    pipeline.addFilter(&f1);
    pipeline.addFilter(&f2);

    // Processing through pipeline: (10.0 * 2.0) + 5.0 = 25.0
    double processed = pipeline.process(10.0);
    assert(std::fabs(processed - 25.0) < 1e-6);

    // Stage out of range checks
    assert(std::fabs(pipeline.stage(10.0, -1) - 10.0) < 1e-6);
    assert(std::fabs(pipeline.stage(10.0, 2) - 10.0) < 1e-6);

    // Stage individual filter checks
    assert(std::fabs(pipeline.stage(10.0, 0) - 20.0) < 1e-6);
    assert(std::fabs(pipeline.stage(10.0, 1) - 15.0) < 1e-6);

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_quality_and_would_reject() {
    SamplePipeline pipeline("TestChannel", 5);
    MockMultiplierFilter f1(1.0, 50.0); // Rejects if reading > 50.0

    pipeline.addFilter(&f1);

    assert(!pipeline.wouldReject(30.0));
    assert(pipeline.wouldReject(60.0));

    // Test quality adjustment
    int prevQ = pipeline.requireQuality(3);
    assert(prevQ == 1); // Default was 1

    // Setting negative quality should not change the value
    prevQ = pipeline.requireQuality(-5);
    assert(prevQ == 3);

    std::cout << "Test test_quality_and_would_reject: PASS" << std::endl;
}

void test_ingest_and_rejection() {
    SamplePipeline pipeline("Sensor", 2); // capacity 2
    pipeline.requireQuality(2);

    MockMultiplierFilter f1(2.0, 100.0);
    pipeline.addFilter(&f1);

    // Ingest with low quality (should reject)
    int res1 = pipeline.ingest(1, 10.0, 1);
    assert(res1 == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Ingest with reading that filter rejects (should reject)
    int res2 = pipeline.ingest(2, 150.0, 2);
    assert(res2 == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 0);

    // Valid ingest
    int res3 = pipeline.ingest(3, 10.0, 2);
    assert(res3 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);

    // Valid ingest 2
    int res4 = pipeline.ingest(4, 20.0, 3);
    assert(res4 == 2);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.isSaturated());

    // Ingest when channel is full (capacity is 2)
    int res5 = pipeline.ingest(5, 30.0, 2);
    assert(res5 == -1);
    assert(pipeline.rejectedCount() == 3);
    assert(pipeline.acceptedCount() == 2);

    std::cout << "Test test_ingest_and_rejection: PASS" << std::endl;
}

void test_ingest_batch() {
    SamplePipeline pipeline("BatchSensor", 10);
    pipeline.requireQuality(1);

    std::vector<double> batch = { 1.0, 2.0, 3.0, 4.0, 5.0 };
    int taken = pipeline.ingestBatch(batch, 1);

    assert(taken == 5);
    assert(pipeline.acceptedCount() == 5);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 5);

    std::cout << "Test test_ingest_batch: PASS" << std::endl;
}

void test_channel_stats_and_latest() {
    SamplePipeline pipeline("StatsChannel", 5);
    MockMultiplierFilter f(2.0, 500.0); // multiplies readings by 2
    pipeline.addFilter(&f);

    pipeline.ingest(1, 10.0, 1); // processed: 20.0
    pipeline.ingest(2, 20.0, 1); // processed: 40.0

    assert(pipeline.validSamples() == 2);
    assert(std::fabs(pipeline.channelMean() - 30.0) < 1e-6);
    assert(std::fabs(pipeline.channelPeak() - 40.0) < 1e-6);

    Sample latest = pipeline.latest();
    assert(latest.getSequence() == 2 || latest.sequence == 2 || latest.getValue() == 40.0 || latest.value == 40.0);

    std::cout << "Test test_channel_stats_and_latest: PASS" << std::endl;
}

void test_flush_and_describe() {
    SamplePipeline pipeline("MyChannel", 5);

    assert(pipeline.describe() == "empty");

    pipeline.ingest(1, 10.0, 1);
    assert(pipeline.describe() != "empty");

    int previousCount = pipeline.flush();
    assert(previousCount == 1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_flush_and_describe: PASS" << std::endl;
}

void test_scale_conversions() {
    SamplePipeline pipeline("Scaled", 5);
    // Tests that scale methods execute without crashing
    double val = 42.0;
    double norm = pipeline.normalised(val);
    double denorm = pipeline.denormalised(norm);
    (void)denorm; // silence unused variable if assertions are simple
    std::cout << "Test test_scale_conversions: PASS" << std::endl;
}

int main() {
    test_constructor_defaults();
    test_constructor_parameterized();
    test_filter_management();
    test_process_and_stage();
    test_quality_and_would_reject();
    test_ingest_and_rejection();
    test_ingestBatch();
    test_channel_stats_and_latest();
    test_flush_and_describe();
    test_scale_conversions();

    std::cout << "\nAll SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}