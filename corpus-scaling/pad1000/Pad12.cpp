#include "Pad12.h"

namespace pad12 {

Unit12::Unit12() {
    state = 12;
}

Unit12::Unit12(int seed) {
    state = seed + 12;
}

int Unit12::value() const {
    return state;
}

int Unit12::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit12::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit12::reset() {
    state = 12;
}

}
