#ifndef SAMPLE_H
#define SAMPLE_H

class Sample {
public:
    int sequence;
    double raw;
    int quality;
    bool flagged;

    Sample();
    Sample(int position, double reading, int qualityScore);

    int position() const;
    double value() const;
    int score() const;
    bool isValid() const;
    bool isDegraded() const;
    double weighted() const;
    double deviationFrom(double reference) const;
    double bounded(double low, double high) const;
    int flag();
    int promote(int amount);
};

#endif
