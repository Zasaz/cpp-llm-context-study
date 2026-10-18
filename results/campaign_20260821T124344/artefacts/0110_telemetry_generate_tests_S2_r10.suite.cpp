#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cmath>

#include "SamplePipeline.h"
#include "Filter.h"
#include "Sample.h"
#include "Channel.h"

// Concrete mock filter implementations for testing
class ScaleFilter : public Filter {
private:
    double factor;
public:
    explicit ScaleFilter(double f) : factor(f) {}
    double apply(double value) const /* override */ {
        return value * factor;
    }
    bool rejects(double /* value */) const /* override */ {
        return false;
    }
};

class OffsetFilter : public Filter {
private:
    double offset;
public:
    explicit OffsetFilter(double o) : offset(o) {}
    double apply(double value) const /* override */ {
        return value + offset;
    }
    bool rejects(double /* value */) const /* override */ {
        return false;
    }
};

class ThresholdRejectFilter : public Filter {
private:
    double maxThreshold;
public:
    explicit ThresholdRejectFilter(double maxT) : maxThreshold(maxT) {}
    double apply(double value) const /* override */ {
        return value;
    }
    bool rejects(double value) const /* override */ {
        return value > maxThreshold;
    }
};

// Helper for floating point comparison
bool isClose(double a, double b, double eps = 1e-6) {
    return std::fabs(a - b) < eps;
}

void test_constructors_and_initial_state() {
    // Default constructor
    SamplePipeline p1;
    assert(p1.acceptedCount() == 0);
    assert(p1.rejectedCount() == 0);
    assert(p1.filterCount() == 0);
    assert(p1.validSamples() == 0);
    assert(!p1.isSaturated());
    assert(p1.describe() == "empty");

    // Parameterized constructor
    SamplePipeline p2("Voltage", 5);
    assert(p2.acceptedCount() == 0);
    assert(p2.rejectedCount() == 0);
    assert(p2.filterCount() == 0);
    assert(p2.validSamples() == 0);
    assert(!p2.isSaturated());
    assert(p2.describe() == "empty");

    std::cout << "Test test_constructors_and_initial_state: PASS" << std::endl;
}

void test_filter_management_and_pipeline_processing() {
    SamplePipeline p("SensorA", 10);

    ScaleFilter sf(2.0);
    OffsetFilter of(5.0);

    // Test adding null pointer
    assert(p.addFilter(nullptr) == 0);
    assert(p.filterCount() == 0);

    // Test adding filters
    assert(p.addFilter(&sf) == 1);
    assert(p.addFilter(&of) == 2);
    assert(p.filterCount() == 2);

    // Test stage execution
    // Stage 0: 10.0 * 2.0 = 20.0
    assert(isClose(p.stage(10.0, 0), 20.0));
    // Stage 1: 10.0 + 5.0 = 15.0
    assert(isClose(p.stage(10.0, 1), 15.0));
    // Invalid stages should return the input unmodified
    assert(isClose(p.stage(10.0, -1), 10.0));
    assert(isClose(p.stage(10.0, 2), 10.0));

    // Test full process pipeline: (10.0 * 2.0) + 5.0 = 25.0
    assert(isClose(p.process(10.0), 25.0));

    // Test clear filters
    assert(p.clearFilters() == 2);
    assert(p.filterCount() == 0);
    assert(isClose(p.process(10.0), 10.0));

    std::cout << "Test test_filter_management_and_pipeline_processing: PASS" << std::endl;
}

void test_rejection_and_quality_checks() {
    SamplePipeline p("Current", 10);
    ThresholdRejectFilter tf(100.0);
    p.addFilter(&tf);

    // Default minimumQuality is 1
    assert(p.wouldReject(150.0) == true);
    assert(p.wouldReject(50.0) == false);

    // Quality lower than minimumQuality (1) should be rejected
    int res1 = p.ingest(1, 50.0, 0);
    assert(res1 == -1);
    assert(p.rejectedCount() == 1);
    assert(p.acceptedCount() == 0);

    // Reading rejected by filter should be rejected
    int res2 = p.ingest(2, 120.0, 2);
    assert(res2 == -1);
    assert(p.rejectedCount() == 2);
    assert(p.acceptedCount() == 0);

    // Change minimum quality requirement
    int prevMin = p.requireQuality(5);
    assert(prevMin == 1);
    // Supplying negative value should not alter minimum quality
    assert(p.requireQuality(-1) == 5);

    // Ingest with quality 4 (below new minimum 5) should fail
    int res3 = p.ingest(3, 50.0, 4);
    assert(res3 == -1);
    assert(p.rejectedCount() == 3);

    // Ingest with quality 5 and acceptable value should succeed
    int res4 = p.ingest(4, 50.0, 5);
    assert(res4 == 1); // accepted count is returned
    assert(p.acceptedCount() == 1);
    assert(p.rejectedCount() == 3);

    std::cout << "Test test_rejection_and_quality_checks: PASS" << std::endl;
}

void test_batch_ingestion_and_channel_saturation() {
    const int capacity = 3;
    SamplePipeline p("Pressure", capacity);

    std::vector<double> batch;
    batch.push_back(10.0);
    batch.push_back(20.0);
    batch.push_back(30.0);
    batch.push_back(40.0); // Will exceed capacity

    int taken = p.ingestBatch(batch, 1);
    // First 3 fit into capacity, 4th should be rejected
    assert(taken == 3);
    assert(p.acceptedCount() == 3);
    assert(p.rejectedCount() == 1);
    assert(p.validSamples() == 3);
    assert(p.isSaturated() == true);

    // Latest sample should be the 3rd one (reading 30.0, sequence 2)
    Sample latestSample = p.latest();
    assert(isClose(latestSample.value, 30.0));

    // Channel statistics
    assert(isClose(p.channelMean(), 20.0)); // (10 + 20 + 30) / 3
    assert(isClose(p.channelPeak(), 30.0));

    // Describe when not empty
    std::string desc = p.describe();
    assert(desc.find("Pressure") != std::string::npos);
    assert(desc != "empty");

    std::cout << "Test test_batch_ingestion_and_channel_saturation: PASS" << std::endl;
}

void test_scaling_and_flush() {
    SamplePipeline p("Temperature", 5);
    ScaleFilter sf(1.5);
    p.addFilter(&sf);

    p.ingest(0, 10.0, 2);
    p.ingest(1, 20.0, 2);
    assert(p.acceptedCount() == 2);
    assert(p.validSamples() == 2);

    // Test normalised and denormalised scaling round-trip
    double val = 42.0;
    double norm = p.normalised(val);
    double denorm = p.denormalised(norm);
    assert(isClose(val, denorm));

    // Test flush
    int previousCount = p.flush();
    assert(previousCount == 2);
    assert(p.acceptedCount() == 0);
    assert(p.rejectedCount() == 0);
    assert(p.validSamples() == 0);
    assert(!p.isSaturated());
    assert(p.describe() == "empty");

    // Filters should persist across flush
    assert(p.filterCount() == 1);
    p.ingest(0, 10.0, 2);
    assert(isClose(p.latest().value, 15.0)); // 10.0 * 1.5

    std::cout << "Test test_scaling_and_flush: PASS" << std::endl;
}

int main() {
    std::cout << "Starting SamplePipeline unit tests..." << std::endl;

    test_constructors_and_initial_state();
    test_filter_management_and_pipeline_processing();
    test_rejection_and_quality_checks();
    test_batch_ingestion_and_channel_saturation();
    test_scaling_and_flush();

    std::cout << "All SamplePipeline unit tests passed successfully!" << std::endl;
    return 0;
}