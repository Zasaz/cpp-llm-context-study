#include "Pad28.h"

namespace pad28 {

Unit28::Unit28() {
    state = 28;
}

Unit28::Unit28(int seed) {
    state = seed + 28;
}

int Unit28::value() const {
    return state;
}

int Unit28::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit28::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit28::reset() {
    state = 28;
}

}
