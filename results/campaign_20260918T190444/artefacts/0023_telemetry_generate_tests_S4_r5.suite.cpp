#include <iostream>
#include <cassert>
#include <cmath>
#include <vector>
#include <string>

#include "SamplePipeline.h"
#include "Filter.h"
#include "Channel.h"
#include "Sample.h"
#include "UnitScale.h"

// Helper comparison for floating-point values
bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// Concrete mock/test filter implementations since Filter may be abstract or polymorphic
class GainFilter : public Filter {
private:
    double factor;
public:
    GainFilter(double f) : factor(f) {}
    double apply(double value) const {
        return value * factor;
    }
    bool rejects(double /*value*/) const {
        return false;
    }
};

class ThresholdFilter : public Filter {
private:
    double maxLimit;
public:
    ThresholdFilter(double maxVal) : maxLimit(maxVal) {}
    double apply(double value) const {
        return value;
    }
    bool rejects(double value) const {
        return value > maxLimit;
    }
};

class OffsetFilter : public Filter {
private:
    double offset;
public:
    OffsetFilter(double off) : offset(off) {}
    double apply(double value) const {
        return value + offset;
    }
    bool rejects(double /*value*/) const {
        return false;
    }
};

void test_constructors_and_defaults() {
    SamplePipeline defaultPipeline;
    assert(defaultPipeline.acceptedCount() == 0);
    assert(defaultPipeline.rejectedCount() == 0);
    assert(defaultPipeline.minimumQuality == 1);
    assert(defaultPipeline.filterCount() == 0);

    SamplePipeline customPipeline("Voltage", 10);
    assert(customPipeline.acceptedCount() == 0);
    assert(customPipeline.rejectedCount() == 0);
    assert(customPipeline.minimumQuality == 1);
    assert(customPipeline.filterCount() == 0);
    assert(customPipeline.channel.channelName() == "Voltage");
    assert(customPipeline.channel.capacity == 10);

    std::cout << "Test test_constructors_and_defaults: PASS" << std::endl;
}

void test_filter_management() {
    SamplePipeline pipeline("Temp", 5);
    GainFilter gFilter(2.0);
    OffsetFilter oFilter(5.0);

    // Adding null filter should not change count
    assert(pipeline.addFilter(0) == 0);
    assert(pipeline.filterCount() == 0);

    // Adding valid filters
    assert(pipeline.addFilter(&gFilter) == 1);
    assert(pipeline.filterCount() == 1);
    assert(pipeline.addFilter(&oFilter) == 2);
    assert(pipeline.filterCount() == 2);

    // Clearing filters
    assert(pipeline.clearFilters() == 2);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

void test_process_and_stage() {
    SamplePipeline pipeline("Signal", 10);
    GainFilter gFilter(2.0);       // value * 2
    OffsetFilter oFilter(10.0);    // value + 10

    pipeline.addFilter(&gFilter);
    pipeline.addFilter(&oFilter);

    // Pipeline processing: (3.0 * 2.0) + 10.0 = 16.0
    double processed = pipeline.process(3.0);
    assert(approxEqual(processed, 16.0));

    // Test individual stages
    assert(approxEqual(pipeline.stage(5.0, 0), 10.0)); // 5.0 * 2.0
    assert(approxEqual(pipeline.stage(5.0, 1), 15.0)); // 5.0 + 10.0

    // Out-of-bounds stage indices should return input reading
    assert(approxEqual(pipeline.stage(5.0, -1), 5.0));
    assert(approxEqual(pipeline.stage(5.0, 2), 5.0));
    assert(approxEqual(pipeline.stage(5.0, 99), 5.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

void test_wouldReject() {
    SamplePipeline pipeline("Sensor", 10);
    ThresholdFilter tFilter(50.0); // rejects values > 50.0
    pipeline.addFilter(&tFilter);

    assert(!pipeline.wouldReject(30.0));
    assert(!pipeline.wouldReject(50.0));
    assert(pipeline.wouldReject(50.1));
    assert(pipeline.wouldReject(100.0));

    std::cout << "Test test_wouldReject: PASS" << std::endl;
}

void test_requireQuality() {
    SamplePipeline pipeline;
    assert(pipeline.minimumQuality == 1);

    int prev = pipeline.requireQuality(5);
    assert(prev == 1);
    assert(pipeline.minimumQuality == 5);

    // Negative minimum should not change the quality threshold
    prev = pipeline.requireQuality(-2);
    assert(prev == 5);
    assert(pipeline.minimumQuality == 5);

    std::cout << "Test test_requireQuality: PASS" << std::endl;
}

void test_ingest_and_rejection() {
    SamplePipeline pipeline("Channel1", 3);
    GainFilter gFilter(2.0);
    ThresholdFilter tFilter(100.0); // rejects raw reading > 100.0

    pipeline.addFilter(&tFilter);
    pipeline.addFilter(&gFilter);
    pipeline.requireQuality(2);

    // 1. Ingest valid sample: reading 20 -> filtered 40, quality 2 >= min 2
    int res1 = pipeline.ingest(1, 20.0, 2);
    assert(res1 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 0);
    assert(approxEqual(pipeline.latest().value, 40.0));

    // 2. Ingest low quality sample -> reject
    int res2 = pipeline.ingest(2, 25.0, 1);
    assert(res2 == -1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 1);

    // 3. Ingest reading that filter rejects -> reject
    int res3 = pipeline.ingest(3, 105.0, 3);
    assert(res3 == -1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);

    // 4. Fill channel to capacity (limit is 3)
    int res4 = pipeline.ingest(4, 10.0, 2); // 2nd accepted
    assert(res4 == 2);
    int res5 = pipeline.ingest(5, 15.0, 2); // 3rd accepted
    assert(res5 == 3);
    assert(pipeline.isSaturated());

    // 5. Ingest when full -> channel.append will fail -> reject
    int res6 = pipeline.ingest(6, 12.0, 2);
    assert(res6 == -1);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 3);

    std::cout << "Test test_ingest_and_rejection: PASS" << std::endl;
}

void test_ingestBatch() {
    SamplePipeline pipeline("BatchChannel", 10);
    pipeline.requireQuality(1);

    std::vector<double> readings = { 10.0, 20.0, 30.0, 40.0 };
    int taken = pipeline.ingestBatch(readings, 2);

    assert(taken == 4);
    assert(pipeline.acceptedCount() == 4);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 4);

    std::cout << "Test test_ingestBatch: PASS" << std::endl;
}

void test_channel_metrics_and_scale() {
    SamplePipeline pipeline("MetricsChannel", 5);
    pipeline.ingest(1, 10.0, 1);
    pipeline.ingest(2, 20.0, 1);
    pipeline.ingest(3, 30.0, 1);

    // Channel mean and peak
    assert(approxEqual(pipeline.channelMean(), 20.0));
    assert(approxEqual(pipeline.channelPeak(), 30.0));
    assert(pipeline.validSamples() == 3);

    // Normalised / Denormalised roundtrip with UnitScale
    double raw = 25.0;
    double norm = pipeline.normalised(raw);
    double denorm = pipeline.denormalised(norm);
    assert(approxEqual(denorm, raw));

    std::cout << "Test test_channel_metrics_and_scale: PASS" << std::endl;
}

void test_flush_and_describe() {
    SamplePipeline pipeline("SensA", 5);

    // When empty
    assert(pipeline.describe() == "empty");

    pipeline.ingest(1, 42.0, 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.describe() != "empty");
    assert(pipeline.describe().find("SensA") != std::string::npos);

    // Flush channel
    int previousCount = pipeline.flush();
    assert(previousCount == 1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.describe() == "empty");
    assert(pipeline.channel.channelName() == "SensA");
    assert(pipeline.channel.capacity == 5);

    std::cout << "Test test_flush_and_describe: PASS" << std::endl;
}

int main() {
    test_constructors_and_defaults();
    test_filter_management();
    test_process_and_stage();
    test_wouldReject();
    test_requireQuality();
    test_ingest_and_rejection();
    test_ingestBatch();
    test_channel_metrics_and_scale();
    test_flush_and_describe();

    std::cout << "All SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}