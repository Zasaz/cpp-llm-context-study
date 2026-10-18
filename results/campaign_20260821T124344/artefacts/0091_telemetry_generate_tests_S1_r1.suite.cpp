#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <cmath>

#include "SamplePipeline.h"
#include "Filter.h"
#include "UnitScale.h"
#include "Sample.h"
#include "Channel.h"

// Helper function for floating point comparisons
static bool approxEqual(double a, double b, double epsilon = 1e-6) {
    return std::fabs(a - b) <= epsilon;
}

// 1. Test default and parameterized constructors
void test_constructors() {
    // Default constructor
    SamplePipeline p1;
    assert(p1.acceptedCount() == 0);
    assert(p1.rejectedCount() == 0);
    assert(p1.filterCount() == 0);
    assert(p1.minimumQuality == 1);
    assert(p1.channel.channelName() == "unnamed");
    assert(p1.channel.capacity == 16);
    assert(p1.channel.isEmpty());
    assert(p1.describe() == "empty");

    // Parameterized constructor
    SamplePipeline p2("sensor_temp", 5);
    assert(p2.acceptedCount() == 0);
    assert(p2.rejectedCount() == 0);
    assert(p2.filterCount() == 0);
    assert(p2.minimumQuality == 1);
    assert(p2.channel.channelName() == "sensor_temp");
    assert(p2.channel.capacity == 5);
    assert(p2.channel.isEmpty());
    assert(p2.describe() == "empty");

    std::cout << "Test test_constructors: PASS" << std::endl;
}

// 2. Test filter management (add, count, clear, null handling)
void test_filter_management() {
    SamplePipeline pipeline("test", 10);
    assert(pipeline.filterCount() == 0);

    // Adding null pointer should not change filter count
    int count = pipeline.addFilter(nullptr);
    assert(count == 0);
    assert(pipeline.filterCount() == 0);

    PassFilter pass;
    ClampFilter clamp(0.0, 10.0);
    SmoothFilter smooth(0.5);

    assert(pipeline.addFilter(&pass) == 1);
    assert(pipeline.addFilter(&clamp) == 2);
    assert(pipeline.addFilter(&smooth) == 3);
    assert(pipeline.filterCount() == 3);

    // Clear filters
    int cleared = pipeline.clearFilters();
    assert(cleared == 3);
    assert(pipeline.filterCount() == 0);

    std::cout << "Test test_filter_management: PASS" << std::endl;
}

// 3. Test filter execution and staging logic
void test_process_and_stage() {
    SamplePipeline pipeline("filter_chain", 10);

    // Filter 1: Clamp between -10.0 and 10.0
    ClampFilter clamp(-10.0, 10.0);
    // Filter 2: Pass filter
    PassFilter pass;

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&pass);

    // Test process (full chain)
    assert(approxEqual(pipeline.process(15.0), 10.0));
    assert(approxEqual(pipeline.process(-25.0), -10.0));
    assert(approxEqual(pipeline.process(5.0), 5.0));

    // Test stage (individual filter invocation)
    // Stage 0 is ClampFilter
    assert(approxEqual(pipeline.stage(50.0, 0), 10.0));
    // Stage 1 is PassFilter
    assert(approxEqual(pipeline.stage(50.0, 1), 50.0));

    // Stage out of range bounds returns input unchanged
    assert(approxEqual(pipeline.stage(50.0, -1), 50.0));
    assert(approxEqual(pipeline.stage(50.0, 2), 50.0));
    assert(approxEqual(pipeline.stage(50.0, 99), 50.0));

    std::cout << "Test test_process_and_stage: PASS" << std::endl;
}

// 4. Test wouldReject logic
void test_would_reject() {
    SamplePipeline pipeline("rejection_test", 10);
    ClampFilter clamp(0.0, 100.0);
    pipeline.addFilter(&clamp);

    assert(!pipeline.wouldReject(50.0));
    assert(!pipeline.wouldReject(0.0));
    assert(!pipeline.wouldReject(100.0));
    assert(pipeline.wouldReject(-0.1));
    assert(pipeline.wouldReject(100.1));

    // Disabled filter rejection behavior
    PassFilter pass;
    pass.toggle(); // enabled becomes false
    SamplePipeline pipeline2("disabled_filter_test", 10);
    pipeline2.addFilter(&pass);
    // When disabled, Filter::rejects returns true for non-zero readings
    assert(pipeline2.wouldReject(10.0));
    assert(!pipeline2.wouldReject(0.0));

    std::cout << "Test test_would_reject: PASS" << std::endl;
}

// 5. Test ingestion with quality checks and rejection reasons
void test_ingest_quality_and_rejections() {
    SamplePipeline pipeline("quality_test", 5);
    ClampFilter clamp(0.0, 50.0);
    pipeline.addFilter(&clamp);

    assert(pipeline.requireQuality(20) == 1); // Previous was default 1
    assert(pipeline.minimumQuality == 20);

    // Negative minimumQuality should be ignored
    assert(pipeline.requireQuality(-5) == 20);
    assert(pipeline.minimumQuality == 20);

    // Ingest with quality below threshold
    int res = pipeline.ingest(1, 25.0, 15);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.acceptedCount() == 0);

    // Ingest with reading that filter would reject
    res = pipeline.ingest(2, 60.0, 50);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 2);
    assert(pipeline.acceptedCount() == 0);

    // Successful ingest
    res = pipeline.ingest(3, 25.0, 50);
    assert(res == 1);
    assert(pipeline.acceptedCount() == 1);
    assert(pipeline.rejectedCount() == 2);

    Sample latest = pipeline.latest();
    assert(latest.position() == 3);
    assert(approxEqual(latest.value(), 25.0));
    assert(latest.score() == 50);
    assert(latest.isValid());

    std::cout << "Test test_ingest_quality_and_rejections: PASS" << std::endl;
}

// 6. Test capacity saturation and batch ingestion
void test_capacity_saturation_and_batch() {
    int capacity = 3;
    SamplePipeline pipeline("cap_test", capacity);

    std::vector<double> batch = {10.0, 20.0, 30.0, 40.0, 50.0};
    int taken = pipeline.ingestBatch(batch, 80);

    // Only 3 should fit into the channel of capacity 3
    assert(taken == 3);
    assert(pipeline.acceptedCount() == 3);
    assert(pipeline.rejectedCount() == 2); // 40.0 and 50.0 rejected due to full channel
    assert(pipeline.isSaturated());
    assert(pipeline.validSamples() == 3);

    // Test statistics
    assert(approxEqual(pipeline.channelMean(), (10.0 + 20.0 + 30.0) / 3.0));
    assert(approxEqual(pipeline.channelPeak(), 30.0));

    // Ingesting when full should fail
    int res = pipeline.ingest(99, 15.0, 90);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 3);

    std::cout << "Test test_capacity_saturation_and_batch: PASS" << std::endl;
}

// 7. Test UnitScale normalization, denormalization, and describe()
void test_scaling_and_description() {
    SamplePipeline pipeline("voltage", 10);
    pipeline.scale = UnitScale(2.0, 5.0, "V"); // toBase: x * 2 + 5, fromBase: (x - 5) / 2

    // Before data is ingested, describe returns "empty"
    assert(pipeline.describe() == "empty");

    // Normalised (fromBase): (15 - 5) / 2 = 5
    assert(approxEqual(pipeline.normalised(15.0), 5.0));

    // Denormalised (toBase): 5 * 2 + 5 = 15
    assert(approxEqual(pipeline.denormalised(5.0), 15.0));

    pipeline.ingest(1, 12.0, 90);
    assert(pipeline.describe() == "voltage:V");

    std::cout << "Test test_scaling_and_description: PASS" << std::endl;
}

// 8. Test flush functionality
void test_flush() {
    SamplePipeline pipeline("pressure", 4);
    pipeline.ingest(1, 10.0, 70);
    pipeline.ingest(2, 20.0, 70);
    pipeline.ingest(3, -50.0, 0); // Rejection due to quality < 1

    assert(pipeline.acceptedCount() == 2);
    assert(pipeline.rejectedCount() == 1);
    assert(pipeline.validSamples() == 2);

    int flushedSamples = pipeline.flush();
    assert(flushedSamples == 2);
    assert(pipeline.acceptedCount() == 0);
    assert(pipeline.rejectedCount() == 0);
    assert(pipeline.validSamples() == 0);
    assert(pipeline.channel.isEmpty());
    assert(pipeline.describe() == "empty");
    assert(pipeline.channel.channelName() == "pressure");
    assert(pipeline.channel.capacity == 4);

    std::cout << "Test test_flush: PASS" << std::endl;
}

// 9. Test end-to-end multi-filter pipeline workflow
void test_multistage_pipeline_workflow() {
    SamplePipeline pipeline("sensor_flow", 10);
    ClampFilter clamp(0.0, 100.0);
    SmoothFilter smooth(0.5); // 0.5 * new + 0.5 * old

    pipeline.addFilter(&clamp);
    pipeline.addFilter(&smooth);

    // Reading 1: 50.0 -> clamped to 50.0 -> smooth (primed with 50.0) -> 50.0
    pipeline.ingest(0, 50.0, 100);
    assert(approxEqual(pipeline.latest().value(), 50.0));

    // Reading 2: 200.0 -> clamped to 100.0 -> smooth (0.5 * 100 + 0.5 * 50 = 75.0) -> 75.0
    // Note: clamp would reject 200.0 because 200.0 > 100.0
    int res = pipeline.ingest(1, 200.0, 100);
    assert(res == -1);
    assert(pipeline.rejectedCount() == 1);

    // Reading 3: 100.0 (in range) -> clamped to 100.0 -> smooth (0.5 * 100 + 0.5 * 50 = 75.0) -> 75.0
    res = pipeline.ingest(2, 100.0, 100);
    assert(res == 2);
    assert(approxEqual(pipeline.latest().value(), 75.0));

    // Reading 4: 50.0 (in range) -> clamped to 50.0 -> smooth (0.5 * 50 + 0.5 * 75 = 62.5) -> 62.5
    res = pipeline.ingest(3, 50.0, 100);
    assert(res == 3);
    assert(approxEqual(pipeline.latest().value(), 62.5));

    assert(pipeline.validSamples() == 3);
    assert(approxEqual(pipeline.channelMean(), (50.0 + 75.0 + 62.5) / 3.0));

    std::cout << "Test test_multistage_pipeline_workflow: PASS" << std::endl;
}

int main() {
    test_constructors();
    test_filter_management();
    test_process_and_stage();
    test_would_reject();
    test_ingest_quality_and_rejections();
    test_capacity_saturation_and_batch();
    test_scaling_and_description();
    test_flush();
    test_multistage_pipeline_workflow();

    std::cout << "\nAll SamplePipeline tests passed successfully!" << std::endl;
    return 0;
}