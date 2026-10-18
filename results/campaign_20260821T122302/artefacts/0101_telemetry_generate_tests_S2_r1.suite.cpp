#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <cassert>

#include "SamplePipeline.h"

// Helper filter implementation for testing pipeline stages and rejection logic
class MultiplierFilter : public Filter {
public:
    double multiplier;
    double maxThreshold;

    MultiplierFilter(double factor = 1.0, double rejectAbove = 1000000.0)
        : multiplier(factor), maxThreshold(rejectAbove) {}

    virtual double apply(double reading) {
        return reading * multiplier;
    }

    virtual bool rejects(double reading) const {
        return reading > maxThreshold;
    }
};

static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

void test_constructors_and_initial_state() {
    // Test default constructor
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.filterCount() == 0);
    assert(defaultPipeline.validSamples() == 0);
    assert(!defaultPipeline.isSaturated());
    assert(defaultPipeline.describe() == "empty");

    // Test parameterized constructor
    SamplePipeline customPipeline("VoltageChannel", 5);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.validSamples() == 0);
    assert(!customPipeline.isSaturated());
    assert(customPipeline.describe() == "empty");

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline("Sensor1", 10);
    assert(pipeline.filterCount() == 0);

    MultiplierFilter f1(2.0, 100.0);
    MultiplierFilter f2(0.5, 50.0);

    // Add valid filter
    int count1 = pipeline.addFilter(&f1);
    assert(count1 == 1);
    assert(pipeline.filterCount() == 1);

    // Add null filter (should be ignored)
    int countNull = pipeline.addFilter(0);
    assert(countNull == 1);
    assert(pipeline.filterCount() == 1);

    // Add second filter
    int count2 = pipeline.addFilter(&f2);
    assert(count2 == 2);
    assert(pipeline.filterCount() == 2);

    // Clear filters
    int prevCount = pipeline.clearFilters();
    assert(prevCount == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_filter_processing_and_staging() {
    SamplePipeline pipeline("Sensor1", 10);

    MultiplierFilter f1(2.0, 500.0); // doubles reading
    MultiplierFilter f2(3.0, 500.0); // triples reading

    pipeline.addFilter(&f1);
    pipeline.addFilter(&f2);

    // Test process (applies all filters in sequence: 10 * 2 = 20, 20 * 3 = 60)
    double processed = pipeline.process(10.0);
    assert(approxEqual(processed, 60.0));

    // Test stage at valid indices
    double stage0 = pipeline.stage(10.0, 0);
    assert(approxEqual(stage0, 20.0));

    double stage1 = pipeline.stage(10.0, 1);
    assert(approxEqual(stage1, 30.0));

    // Test stage with out-of-bounds indices (should return original reading)
    assert(approxEqual(pipeline.stage(10.0, -1), 10.0));
    assert(approxEqual(pipeline.stage(10.0, 2), 10.0));
    assert(approxEqual(pipeline.stage(10.0, 100), 10.0));

    // Test wouldReject
    MultiplierFilter fReject(1.0, 50.0);
    SamplePipeline pipelineReject("Sensor2", 10);
    pipelineReject.addFilter(&fReject);

    assert(!pipelineReject.wouldReject(40.0));
    assert(pipelineReject.wouldReject(60.0));

    std::cout << "Test test_filter_processing_and_staging: PASS" << std::endl;
}

void test_require_quality() {
    SamplePipeline pipeline("Sensor1", 10);

    // Default minimum quality is 1
    int prev = pipeline.requireQuality(3);
    assert(prev == 1);

    // Setting negative value should not change quality
    prev = pipeline.requireQuality(-5);
    assert(prev == 3);

    // Ingest sample with quality 2 (should be rejected since minimumQuality is 3)
    int res = pipeline.ingest(1, 10.0, 2);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Ingest sample with quality 3 (should be accepted)
    res = pipeline.ingest(2, 10.0, 3);
    assert(res == 1);
    assert(pipeline.acceptedCount() == 1);

    std::cout << "Test test_require_quality: PASS" << std::endl;
}

void test_ingest_single_and_rejection() {
    SamplePipeline pipeline("Sensor1", 2);

    MultiplierFilter fReject(1.0, 100.0);
    pipeline.addFilter(&fReject);

    // 1. Rejected due to filter rejection
    int r1 = pipeline.ingest(1, 150.0, 2);
    assert(r1 == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // 2. Successful ingestion
    int r2 = pipeline.ingest(2, 50.0, 2);
    assert(r2 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 1);

    // 3. Second successful ingestion (fills channel capacity of 2)
    int r3 = pipeline.ingest(3, 60.0, 2);
    assert(r3 == 2);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.isSaturated());

    // 4. Ingestion when channel is saturated (capacity reached)
    int r4 = pipeline.ingest(4, 70.0, 2);
    assert(r4 == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 2);

    std::cout << "Test test_ingest_single_and_rejection: PASS" << std::endl;
}

void test_ingest_batch() {
    SamplePipeline pipeline("BatchSensor", 10);
    MultiplierFilter f(1.0, 80.0); // rejects values > 80
    pipeline.addFilter(&f);

    std::vector<double> readings;
    readings.push_back(10.0); // pass
    readings.push_back(90.0); // rejected by filter
    readings.push_back(20.0); // pass
    readings.push_back(100.0); // rejected by filter
    readings.push_back(30.0); // pass

    int acceptedInBatch = pipeline.ingestBatch(readings, 2);
    assert(acceptedInBatch == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.validSamples() == 3);

    std::cout << "Test test_ingest_batch: PASS" << std::endl;
}

void test_channel_metrics_and_latest() {
    SamplePipeline pipeline("MetricsSensor", 5);
    MultiplierFilter f(2.0, 1000.0); // doubles values
    pipeline.addFilter(&f);

    pipeline.ingest(1, 10.0, 2); // becomes 20.0
    pipeline.ingest(2, 20.0, 2); // becomes 40.0
    pipeline.ingest(3, 30.0, 2); // becomes 60.0

    assert(approxEqual(pipeline.channelMean(), (20.0 + 40.0 + 60.0) / 3.0));
    assert(approxEqual(pipeline.channelPeak(), 60.0));
    assert(pipeline.validSamples() == 3);

    Sample latestSample = pipeline.latest();
    assert(latestSample.sequenceNumber() == 3);
    assert(approxEqual(latestSample.value(), 60.0));
    assert(latestSample.quality() == 2);

    std::cout << "Test test_channel_metrics_and_latest: PASS" << std::endl;
}

void test_normalised_and_denormalised() {
    SamplePipeline pipeline("TempSensor", 5);

    double baseVal = 100.0;
    double norm = pipeline.normalised(baseVal);
    double denorm = pipeline.denormalised(norm);

    assert(approxEqual(denorm, baseVal));

    std::cout << "Test test_normalised_and_denormalised: PASS" << std::endl;
}

void test_flush() {
    SamplePipeline pipeline("PressureSensor", 3);
    pipeline.ingest(1, 10.0, 1);
    pipeline.ingest(2, 20.0, 1);
    pipeline.ingest(3, 200.0, -1); // rejected due to quality < 1

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

void test_describe() {
    SamplePipeline pipeline("CurrentSensor", 5);

    // Empty channel
    assert(pipeline.describe() == "empty");

    // After ingestion, description should reflect channelName:unitName
    pipeline.ingest(1, 5.0, 1);
    std::string desc = pipeline.describe();
    assert(desc.rfind("CurrentSensor:", 0) == 0);

    std::cout << "Test test_describe: PASS" << std::endl;
}

int main() {
    test_constructors_and_initial_state();
    test_filter_management();
    test_filter_processing_and_staging();
    test_require_quality();
    test_ingest_single_and_rejection();
    test_ingest_batch();
    test_channel_metrics_and_latest();
    test_normalised_and_denormalised();
    test_flush();
    test_describe();

    std::cout << "\nAll SamplePipeline unit tests passed successfully!" << std::endl;
    return 0;
}