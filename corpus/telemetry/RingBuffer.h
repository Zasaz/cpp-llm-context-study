#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <vector>

class RingBuffer {
public:
    std::vector<double> cells;
    int head;
    int filled;
    int overwrites;

    RingBuffer();
    RingBuffer(int slots);

    int push(double value);
    double oldest() const;
    double newest() const;
    double at(int offset) const;
    int occupancy() const;
    int slots() const;
    bool isFull() const;
    bool isEmpty() const;
    int overwriteCount() const;
    double total() const;
    double average() const;
    int drain();
};

#endif
