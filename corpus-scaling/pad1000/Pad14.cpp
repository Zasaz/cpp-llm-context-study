#include "Pad14.h"

namespace pad14 {

Unit14::Unit14() {
    state = 14;
}

Unit14::Unit14(int seed) {
    state = seed + 14;
}

int Unit14::value() const {
    return state;
}

int Unit14::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit14::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit14::reset() {
    state = 14;
}

}
