#include "Pad16.h"

namespace pad16 {

Unit16::Unit16() {
    state = 16;
}

Unit16::Unit16(int seed) {
    state = seed + 16;
}

int Unit16::value() const {
    return state;
}

int Unit16::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit16::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit16::reset() {
    state = 16;
}

}
