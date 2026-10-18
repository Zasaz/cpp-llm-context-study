#include "Pad30.h"

namespace pad30 {

Unit30::Unit30() {
    state = 30;
}

Unit30::Unit30(int seed) {
    state = seed + 30;
}

int Unit30::value() const {
    return state;
}

int Unit30::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit30::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit30::reset() {
    state = 30;
}

}
