#include "Channel.h"

Channel::Channel() {
    name = "unnamed";
    capacity = 16;
    dropped = 0;
}

Channel::Channel(const std::string& channelName, int limit) {
    name = channelName;
    capacity = limit > 0 ? limit : 1;
    dropped = 0;
}

int Channel::append(const Sample& sample) {
    if (isFull()) {
        dropped = dropped + 1;
        return -1;
    }
    samples.push_back(sample);
    return count();
}

int Channel::count() const {
    return static_cast<int>(samples.size());
}

int Channel::validCount() const {
    int total = 0;
    for (unsigned int i = 0; i < samples.size(); ++i) {
        if (samples[i].isValid()) {
            total = total + 1;
        }
    }
    return total;
}

int Channel::droppedCount() const {
    return dropped;
}

bool Channel::isFull() const {
    return count() >= capacity;
}

bool Channel::isEmpty() const {
    return samples.empty();
}

double Channel::sum() const {
    double total = 0.0;
    for (unsigned int i = 0; i < samples.size(); ++i) {
        total = total + samples[i].value();
    }
    return total;
}

double Channel::mean() const {
    int n = count();
    if (n == 0) {
        return 0.0;
    }
    return sum() / static_cast<double>(n);
}

double Channel::peak() const {
    if (samples.empty()) {
        return 0.0;
    }
    double best = samples[0].value();
    for (unsigned int i = 1; i < samples.size(); ++i) {
        double current = samples[i].value();
        if (current > best) {
            best = current;
        }
    }
    return best;
}

double Channel::trough() const {
    if (samples.empty()) {
        return 0.0;
    }
    double best = samples[0].value();
    for (unsigned int i = 1; i < samples.size(); ++i) {
        double current = samples[i].value();
        if (current < best) {
            best = current;
        }
    }
    return best;
}

double Channel::range() const {
    return peak() - trough();
}

double Channel::meanInBase() const {
    return scale.toBase(mean());
}

Sample Channel::sampleAt(int position) const {
    if (position < 0 || position >= count()) {
        return Sample();
    }
    return samples[static_cast<unsigned int>(position)];
}

Sample Channel::newest() const {
    return sampleAt(count() - 1);
}

int Channel::purgeDegraded() {
    std::vector<Sample> kept;
    int removed = 0;
    for (unsigned int i = 0; i < samples.size(); ++i) {
        if (samples[i].isDegraded()) {
            removed = removed + 1;
        } else {
            kept.push_back(samples[i]);
        }
    }
    samples = kept;
    return removed;
}

std::string Channel::channelName() const {
    return name;
}

int Channel::rescale(const UnitScale& replacement) {
    scale = replacement;
    return count();
}
