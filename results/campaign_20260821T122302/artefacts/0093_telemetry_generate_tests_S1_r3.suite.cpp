#include <cassert>
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

#include "SamplePipeline.h"
#include "Filter.h"
#include "Channel.h"
#include "Sample.h"
#include "UnitScale.h"

static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) < epsilon;
}

// 1. Test constructors and initial default states
void test_SamplePipeline_constructorsAndDefaults() {
    SamplePipeline defaultPipe;
    assert(defaultPipe.channel.channelName() == "unnamed");
    assert(defaultPipe.channel.capacity == 16);
    assert(defaultPipe.filterCount() == 0);
    assert(defaultPipe.acceptedCount() == 0);
    assert(defaultPipe.rejectedCount() == 0);
    assert(defaultPipe.minimumQuality == 1);
    assert(defaultPipe.validSamples() == 0);
    assert(!defaultPipe.isSaturated());
    assert(defaultPipe.describe() == "empty");

    SamplePipeline customPipe("temperature", 5);
    assert(customPipe.channel.channelName() == "temperature");
    assert(customPipe.channel.capacity == 5);
    assert(customPipe.filterCount() == 0);
    assert(customPipe.acceptedCount() == 0);
    assert(customPipe.rejectedCount() == 0);
    assert(customPipe.minimumQuality == 1);

    std::cout << "Test test_SamplePipeline_constructorsAndDefaults: PASS" << std::endl;
}

// 2. Test filter management: addFilter, filterCount, clearFilters
void test_SamplePipeline_filterManagement() {
    SamplePipeline pipeline("sensor", 10);

    PassFilter pf;
    ClampFilter cf(0.0, 100.0);
    SmoothFilter sf(0.5);

    assert(pipeline.addFilter(nullptr) == 0);
    assert(pipeline.filterCount() == 0);

    assert(pipeline.addFilter(&pf) == 1);
    assert(pipeline.addFilter(&cf) == 2);
    assert(pipeline.addFilter(&sf) == 3);
    assert(pipeline.filterCount() == 3);

    int cleared = pipeline.clearFilters();
    assert(cleared == 3);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_SamplePipeline_filterManagement: PASS" << std::endl;
}

// 3. Test processing through the filter pipeline and stage inspection
void test_SamplePipeline_processingAndStaging() {
    SamplePipeline pipeline("voltage", 10);
    ClampFilter clamp(0.0, 10.0);
    SmoothFilter smooth(0.5); // First apply sets primed: 10.0, second: 0.5 * 2.0 + 0.5 * 10.0 = 6.0

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // Testing stage() with invalid indices
    assert(approxEqual(pipeline.stage(15.0, -1), 15.0));
    assert(approxEqual(pipeline.stage(15.0, 5), 15.0));

    // Staging directly applies filter at that index
    // Note: stage applies the filter so clamp will clamp 15.0 -> 10.0
    double stagedClamp = pipeline.stage(15.0, 0);
    assert(approxEqual(stagedClamp, 10.0));

    // Process through full chain
    pipeline.clearFilters();
    ClampFilter cFilter(-5.0, 5.0);
    pipeline.addFilter(&cFilter);

    double processedVal = pipeline.process(10.0);
    assert(approxEqual(processedVal, 5.0));
    processedVal = pipeline.process(-10.0);
    assert(approxEqual(processedVal, -5.0));
    processedVal = pipeline.process(3.0);
    assert(approxEqual(processedVal, 3.0));

    std::cout << "Test test_SamplePipeline_processingAndStaging: PASS" << std::endl;
}

// 4. Test wouldReject checks
void test_SamplePipeline_wouldReject() {
    SamplePipeline pipeline("pressure", 10);
    ClampFilter clamp(0.0, 100.0); // rejects if reading < 0 or reading > 100

    pipeline.addFilter(&clamp);
    assert(pipeline.wouldReject(-5.0));
    assert(pipeline.wouldReject(105.0));
    assert(!pipeline.wouldReject(50.0));

    // Disabled filter rejection check
    pipeline.clearFilters();
    PassFilter pass;
    pipeline.addFilter(&pass);
    assert(!pipeline.wouldReject(50.0));
    pass.toggle(); // enabled = false; Filter::rejects returns true if !enabled && reading != 0.0
    assert(pipeline.wouldReject(50.0));
    assert(!pipeline.wouldReject(0.0)); // 0.0 is not rejected even if disabled

    std::cout << "Test test_SamplePipeline_wouldReject: PASS" << std::endl;
}

// 5. Test quality thresholds and requireQuality
void test_SamplePipeline_qualityHandling() {
    SamplePipeline pipeline("humidity", 10);
    
    int prevMin = pipeline.requireQuality(50);
    assert(prevMin == 1);
    assert(pipeline.minimumQuality == 50);

    // Negative values should be ignored by requireQuality
    pipeline.requireQuality(-5);
    assert(pipeline.minimumQuality == 50);

    // Ingest with quality lower than minimumQuality should be rejected
    int res = pipeline.ingest(1, 20.0, 49);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Ingest with quality >= minimumQuality should be accepted
    res = pipeline.ingest(2, 20.0, 50);
    assert(res == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 1);

    std::cout << "Test test_SamplePipeline_qualityHandling: PASS" << std::endl;
}

// 6. Test ingest behavior with filters and capacity saturation
void test_SamplePipeline_ingestAndSaturation() {
    SamplePipeline pipeline("sensor_stream", 2); // capacity = 2
    ClampFilter clamp(0.0, 50.0);
    pipeline.addFilter(&clamp);

    // 1. Rejected due to wouldReject
    int res1 = pipeline.ingest(1, 100.0, 80);
    assert(res1 == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // 2. Successful ingest 1
    int res2 = pipeline.ingest(2, 30.0, 80);
    assert(res2 == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(!pipeline.isSaturated());
    assert(pipeline.validSamples() == 1);

    // 3. Successful ingest 2
    int res3 = pipeline.ingest(3, 40.0, 80);
    assert(res3 == 2);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.isSaturated());
    assert(pipeline.validSamples() == 2);

    // 4. Ingest when full -> Channel::append fails and returns -1 -> ingest rejects
    int res4 = pipeline.ingest(4, 25.0, 80);
    assert(res4 == -1);
    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 2);

    // Check latest sample
    Sample latest = pipeline.latest();
    assert(latest.position() == 3);
    assert(approxEqual(latest.value(), 40.0));

    std::cout << "Test test_SamplePipeline_ingestAndSaturation: PASS" << std::endl;
}

// 7. Test ingestBatch
void test_SamplePipeline_ingestBatch() {
    SamplePipeline pipeline("batch_stream", 5);
    std::vector<double> batch = {10.0, 20.0, 30.0, 40.0, 50.0, 60.0};

    // Quality 70 satisfies default minimum quality 1
    int acceptedInBatch = pipeline.ingestBatch(batch, 70);
    // Capacity is 5, so 5 should be accepted, 6th rejected due to full capacity
    assert(acceptedInBatch == 5);
    assert(pipeline.acceptedCount() == 5);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.isSaturated());

    std::cout << "Test test_SamplePipeline_ingestBatch: PASS" << std::endl;
}

// 8. Test channel statistics: mean and peak
void test_SamplePipeline_statistics() {
    SamplePipeline pipeline("metrics", 10);
    assert(approxEqual(pipeline.channelMean(), 0.0));
    assert(approxEqual(pipeline.channelPeak(), 0.0));

    pipeline.ingest(1, 10.0, 80);
    pipeline.ingest(2, 30.0, 80);
    pipeline.ingest(3, 20.0, 80);

    assert(approxEqual(pipeline.channelMean(), 20.0));
    assert(approxEqual(pipeline.channelPeak(), 30.0));

    std::cout << "Test test_SamplePipeline_statistics: PASS" << std::endl;
}

// 9. Test scaling conversions: normalised and denormalised
void test_SamplePipeline_scaling() {
    SamplePipeline pipeline("voltage", 10);
    // UnitScale(scaleFactor, scaleOffset, unitName)
    // toBase: x * factor + offset
    // fromBase: (x - offset) / factor
    pipeline.scale = UnitScale(2.0, 5.0, "Volts");

    // normalised calls scale.fromBase(reading): (reading - 5) / 2
    assert(approxEqual(pipeline.normalised(15.0), 5.0));
    assert(approxEqual(pipeline.normalised(5.0), 0.0));

    // denormalised calls scale.toBase(reading): reading * 2 + 5
    assert(approxEqual(pipeline.denormalised(5.0), 15.0));
    assert(approxEqual(pipeline.denormalised(0.0), 5.0));

    std::cout << "Test test_SamplePipeline_scaling: PASS" << std::endl;
}

// 10. Test describe and flush
void test_SamplePipeline_describeAndFlush() {
    SamplePipeline pipeline("pressure", 5);
    pipeline.scale = UnitScale(1.0, 0.0, "PSI");

    // Empty channel describe
    assert(pipeline.describe() == "empty");

    pipeline.ingest(1, 14.7, 90);
    // Non-empty channel describe: name + ":" + unitName
    assert(pipeline.describe() == "pressure:PSI");

    // Flush resets channel and counters, preserving channel name and capacity
    int flushedCount = pipeline.flush();
    assert(flushedCount == 1);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.describe() == "empty");
    assert(pipeline.channel.channelName() == "pressure");
    assert(pipeline.channel.capacity == 5);

    std::cout << "Test test_SamplePipeline_describeAndFlush: PASS" << std::endl;
}

int main() {
    test_SamplePipeline_constructorsAndDefaults();
    test_SamplePipeline_filterManagement();
    test_SamplePipeline_processingAndStaging();
    test_SamplePipeline_wouldReject();
    test_SamplePipeline_qualityHandling();
    test_SamplePipeline_ingestAndSaturation();
    test_SamplePipeline_ingestBatch();
    test_SamplePipeline_statistics();
    test_SamplePipeline_scaling();
    test_SamplePipeline_describeAndFlush();

    std::cout << "\nAll SamplePipeline unit tests completed successfully." << std::endl;
    return 0;
}