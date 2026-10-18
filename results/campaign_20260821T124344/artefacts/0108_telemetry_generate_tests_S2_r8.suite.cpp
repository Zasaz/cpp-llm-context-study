#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cmath>

#include "SamplePipeline.h"
#include "Filter.h"
#include "Sample.h"
#include "Channel.h"
#include "Scale.h"

// Helper dummy filter for testing pipeline filter operations
class OffsetFilter : public Filter {
private:
    double offset;
    double rejectAbove;
public:
    OffsetFilter(double offsetValue = 0.0, double maxAcceptable = 1000.0)
        : offset(offsetValue), rejectAbove(maxAcceptable) {}

    virtual double apply(double value) const {
        return value + offset;
    }

    virtual bool rejects(double value) const {
        return value > rejectAbove;
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
    SamplePipeline customPipeline("Voltage", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.describe() == "empty");

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline("Temp", 10);

    OffsetFilter f1(2.0, 100.0);
    OffsetFilter f2(3.0, 100.0);

    // Adding null filter should be ignored
    assert(pipeline.addFilter(0) == 0);
    assert(pipeline.filterCount() == 0);

    // Adding valid filters
    assert(pipeline.addFilter(&f1) == 1);
    assert(pipeline.filterCount() == 1);
    assert(pipeline.addFilter(&f2) == 2);
    assert(pipeline.filterCount() == 2);

    // Clear filters
    int previous = pipeline.clearFilters();
    assert(previous == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipeline("Sensor", 10);
    OffsetFilter f1(5.0, 100.0); // +5
    OffsetFilter f2(2.5, 100.0); // +2.5

    pipeline.addFilter(&f1);
    pipeline.addFilter(&f2);

    // Stage testing
    assert(pipeline.stage(10.0, -1) == 10.0); // Out of bounds low
    assert(pipeline.stage(10.0, 2) == 10.0);  // Out of bounds high
    assert(pipeline.stage(10.0, 0) == 15.0);  // Filter 0 applied
    assert(pipeline.stage(10.0, 1) == 12.5);  // Filter 1 applied

    // Process testing (chained application)
    assert(pipeline.process(10.0) == 17.5);   // 10.0 + 5.0 + 2.5

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_require_quality() {
    SamplePipeline pipeline("Pressure", 10);
    
    // Default quality threshold is 1
    int prev = pipeline.requireQuality(5);
    assert(prev == 1);

    // Setting negative quality should be ignored
    prev = pipeline.requireQuality(-1);
    assert(prev == 5);

    // Ingest sample below required quality (5)
    int res = pipeline.ingest(0, 10.0, 4);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Ingest sample matching required quality
    res = pipeline.ingest(1, 10.0, 5);
    assert(res == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 1);

    std::cout << "Test test_require_quality: PASS" << std::endl;
}

void test_ingest_rejections_and_filters() {
    SamplePipeline pipeline("Flow", 5);
    OffsetFilter f1(1.0, 50.0); // Rejects values > 50
    pipeline.addFilter(&f1);

    // Rejection due to filter wouldReject
    assert(pipeline.wouldReject(60.0) == true);
    assert(pipeline.wouldReject(40.0) == false);

    int res = pipeline.ingest(0, 60.0, 2);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 1);

    // Accepted sample (value processed to 10 + 1 = 11)
    res = pipeline.ingest(1, 10.0, 2);
    assert(res == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.latest().value == 11.0);

    std::cout << "Test test_ingest_rejections_and_filters: PASS" << std::endl;
}

void test_ingest_batch_and_capacity() {
    SamplePipeline pipeline("BatchTest", 3); // Capacity = 3

    std::vector<double> batch;
    batch.push_back(10.0);
    batch.push_back(20.0);
    batch.push_back(30.0);
    batch.push_back(40.0); // Should fail/saturate channel capacity

    int taken = pipeline.ingestBatch(batch, 1);
    assert(taken == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.isSaturated() == true);
    assert(pipeline.validSamples() == 3);

    std::cout << "Test test_ingest_batch_and_capacity: PASS" << std::endl;
}

void test_channel_aggregations_and_scale() {
    SamplePipeline pipeline("Current", 10);

    pipeline.ingest(0, 10.0, 1);
    pipeline.ingest(1, 30.0, 1);
    pipeline.ingest(2, 20.0, 1);

    // Channel mean and peak
    assert(std::fabs(pipeline.channelMean() - 20.0) < 1e-6);
    assert(std::fabs(pipeline.channelPeak() - 30.0) < 1e-6);

    // Test describe when not empty
    std::string desc = pipeline.describe();
    assert(desc.find("Current") != std::string::npos);

    // Normalised / denormalised operations
    double val = 50.0;
    double norm = pipeline.normalised(val);
    double denorm = pipeline.denormalised(norm);
    assert(std::fabs(denorm - val) < 1e-6);

    std::cout << "Test test_channel_aggregations_and_scale: PASS" << std::endl;
}

void test_flush() {
    SamplePipeline pipeline("Vibration", 5);
    pipeline.ingest(0, 5.0, 1);
    pipeline.ingest(1, 15.0, 1);
    pipeline.ingest(2, 25.0, -5); // Rejected due to quality < 1

    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);

    int flushed = pipeline.flush();
    assert(flushed == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.describe() == "empty");

    std::cout << "Test test_flush: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_filter_management();
    test_process_and_stage();
    test_require_quality();
    test_ingest_rejections_and_filters();
    test_ingest_batch_and_capacity();
    test_channel_aggregations_and_scale();
    test_flush();

    std::cout << "All SamplePipeline unit tests passed successfully!" << std::endl;
    return 0;
}