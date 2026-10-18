#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cmath>

#include "SamplePipeline.h"

// Concrete mock filter implementation for testing filter functionality
class MockFilter : public Filter {
public:
    double multiplier;
    double offset;
    double rejectThreshold;
    bool enableRejection;

    MockFilter(double m = 1.0, double o = 0.0, double rejThreshold = 1000.0, bool rej = false)
        : multiplier(m), offset(o), rejectThreshold(rejThreshold), enableRejection(rej) {}

    virtual double apply(double value) const {
        return value * multiplier + offset;
    }

    virtual bool rejects(double value) const {
        if (enableRejection && value > rejectThreshold) {
            return true;
        }
        return false;
    }
};

void test_constructors_and_initial_state() {
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.validSamples() == 0);
    assert(!defaultPipeline.isSaturated());
    assert(defaultPipeline.describe() == "empty");

    SamplePipeline paramPipeline("SensorA", 5);
    assert(paramPipeline.acceptedCount() == 0);
    assert(paramPipeline.rejectedCount() == 0);
    assert(paramPipeline.filterCount() == 0);
    assert(paramPipeline.validSamples() == 0);
    assert(!paramPipeline.isSaturated());
    assert(paramPipeline.describe() == "empty");

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline("FilterTest", 10);
    MockFilter f1(2.0, 1.0);
    MockFilter f2(0.5, -2.0);

    // Adding null filter should be ignored
    int count = pipeline.addFilter(0);
    assert(count == 0);
    assert(pipeline.filterCount() == 0);

    // Adding valid filters
    count = pipeline.addFilter(&f1);
    assert(count == 1);
    assert(pipeline.filterCount() == 1);

    count = pipeline.addFilter(&f2);
    assert(count == 2);
    assert(pipeline.filterCount() == 2);

    // Clearing filters
    int previousCount = pipeline.clearFilters();
    assert(previousCount == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipeline("ProcessTest", 10);
    MockFilter f1(2.0, 3.0); // y = 2x + 3
    MockFilter f2(3.0, -1.0); // y = 3x - 1

    pipeline.addFilter(&f1);
    pipeline.addFilter(&f2);

    // Testing individual stage processing
    // Out of bounds checks
    assert(pipeline.stage(5.0, -1) == 5.0);
    assert(pipeline.stage(5.0, 2) == 5.0);

    // Valid stages
    assert(pipeline.stage(5.0, 0) == (5.0 * 2.0 + 3.0)); // 13.0
    assert(pipeline.stage(5.0, 1) == (5.0 * 3.0 - 1.0)); // 14.0

    // Full pipeline processing: f2(f1(5.0)) = (5*2 + 3)*3 - 1 = 13*3 - 1 = 38
    double result = pipeline.process(5.0);
    assert(result == 38.0);

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_require_quality() {
    SamplePipeline pipeline("QualityTest", 10);

    // Default minimum quality is 1
    int prev = pipeline.requireQuality(3);
    assert(prev == 1);

    // Setting negative value should not alter the requirement
    prev = pipeline.requireQuality(-5);
    assert(prev == 3);

    // Ingesting with quality below minimum
    int status = pipeline.ingest(0, 10.0, 2); // quality 2 < 3
    assert(status == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Ingesting with acceptable quality
    status = pipeline.ingest(1, 10.0, 3);
    assert(status == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 1);

    std::cout << "Test test_require_quality: PASS" << std::endl;
}

void test_ingest_rejection_and_filtering() {
    SamplePipeline pipeline("RejectTest", 5);
    MockFilter rejectFilter(1.0, 0.0, 50.0, true); // Rejects values > 50.0
    pipeline.addFilter(&rejectFilter);

    // Test wouldReject helper
    assert(!pipeline.wouldReject(40.0));
    assert(pipeline.wouldReject(60.0));

    // Reject due to filter
    int res = pipeline.ingest(1, 60.0, 1);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Accept valid
    res = pipeline.ingest(2, 40.0, 1);
    assert(res == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 1);

    std::cout << "Test test_ingest_rejection_and_filtering: PASS" << std::endl;
}

void test_ingest_batch_and_channel_metrics() {
    SamplePipeline pipeline("BatchTest", 10);
    MockFilter scaleFilter(2.0, 0.0); // doubles readings
    pipeline.addFilter(&scaleFilter);

    std::vector<double> batch;
    batch.push_back(10.0); // processed -> 20.0
    batch.push_back(20.0); // processed -> 40.0
    batch.push_back(30.0); // processed -> 60.0

    int taken = pipeline.ingestBatch(batch, 1);
    assert(taken == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.validSamples() == 3);

    // Channel mean: (20 + 40 + 60) / 3 = 40.0
    assert(std::fabs(pipeline.channelMean() - 40.0) < 1e-6);

    // Channel peak
    assert(std::fabs(pipeline.channelPeak() - 60.0) < 1e-6);

    // Latest sample
    Sample latest = pipeline.latest();
    assert(latest.sequenceNumber() == 2 || latest.value() == 60.0);

    // Saturation check
    assert(!pipeline.isSaturated());

    std::cout << "Test test_ingest_batch_and_channel_metrics: PASS" << std::endl;
}

void test_capacity_saturation_and_flush() {
    SamplePipeline pipeline("CapacityTest", 2);

    assert(pipeline.ingest(0, 1.0, 1) == 1);
    assert(pipeline.ingest(1, 2.0, 1) == 2);
    assert(pipeline.isSaturated());

    // Ingesting beyond capacity should fail and increment rejected
    assert(pipeline.ingest(2, 3.0, 1) == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 2);

    // Description when not empty
    std::string desc = pipeline.describe();
    assert(desc != "empty");
    assert(desc.find("CapacityTest") != std::string::npos);

    // Flush
    int flushed = pipeline.flush();
    assert(flushed == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(!pipeline.isSaturated());
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_capacity_saturation_and_flush: PASS" << std::endl;
}

void test_normalised_and_denormalised() {
    SamplePipeline pipeline("ScaleTest", 10);

    double val = 42.0;
    double norm = pipeline.normalised(val);
    double denorm = pipeline.denormalised(norm);

    // Round-trip check
    assert(std::fabs(denorm - val) < 1e-5);

    std::cout << "Test test_normalised_and_denormalised: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_filter_management();
    test_process_and_stage();
    test_require_quality();
    test_ingest_rejection_and_filtering();
    test_ingest_batch_and_channel_metrics();
    test_capacity_saturation_and_flush();
    test_normalised_and_denormalised();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}