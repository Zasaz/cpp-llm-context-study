#include "Pad32.h"

namespace pad32 {

Unit32::Unit32() {
    state = 32;
}

Unit32::Unit32(int seed) {
    state = seed + 32;
}

int Unit32::value() const {
    return state;
}

int Unit32::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit32::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit32::reset() {
    state = 32;
}

}
