#include "RingBuffer.h"

RingBuffer::RingBuffer() {
    cells.resize(8, 0.0);
    head = 0;
    filled = 0;
    overwrites = 0;
}

RingBuffer::RingBuffer(int slotCount) {
    int size = slotCount > 0 ? slotCount : 1;
    cells.resize(static_cast<unsigned int>(size), 0.0);
    head = 0;
    filled = 0;
    overwrites = 0;
}

int RingBuffer::push(double value) {
    if (isFull()) {
        overwrites = overwrites + 1;
    }
    cells[static_cast<unsigned int>(head)] = value;
    head = (head + 1) % slots();
    if (filled < slots()) {
        filled = filled + 1;
    }
    return filled;
}

double RingBuffer::oldest() const {
    if (isEmpty()) {
        return 0.0;
    }
    int index = (head - filled + slots()) % slots();
    return cells[static_cast<unsigned int>(index)];
}

double RingBuffer::newest() const {
    if (isEmpty()) {
        return 0.0;
    }
    int index = (head - 1 + slots()) % slots();
    return cells[static_cast<unsigned int>(index)];
}

double RingBuffer::at(int offset) const {
    if (offset < 0 || offset >= filled) {
        return 0.0;
    }
    int index = (head - filled + offset + slots()) % slots();
    return cells[static_cast<unsigned int>(index)];
}

int RingBuffer::occupancy() const {
    return filled;
}

int RingBuffer::slots() const {
    return static_cast<int>(cells.size());
}

bool RingBuffer::isFull() const {
    return filled >= slots();
}

bool RingBuffer::isEmpty() const {
    return filled == 0;
}

int RingBuffer::overwriteCount() const {
    return overwrites;
}

double RingBuffer::total() const {
    double sum = 0.0;
    for (int i = 0; i < filled; ++i) {
        sum = sum + at(i);
    }
    return sum;
}

double RingBuffer::average() const {
    if (filled == 0) {
        return 0.0;
    }
    return total() / static_cast<double>(filled);
}

int RingBuffer::drain() {
    int previous = filled;
    head = 0;
    filled = 0;
    for (unsigned int i = 0; i < cells.size(); ++i) {
        cells[i] = 0.0;
    }
    return previous;
}
