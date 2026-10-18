#include "Pad24.h"

namespace pad24 {

Unit24::Unit24() {
    state = 24;
}

Unit24::Unit24(int seed) {
    state = seed + 24;
}

int Unit24::value() const {
    return state;
}

int Unit24::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit24::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit24::reset() {
    state = 24;
}

}
