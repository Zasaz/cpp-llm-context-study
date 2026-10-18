#include "Pad20.h"

namespace pad20 {

Unit20::Unit20() {
    state = 20;
}

Unit20::Unit20(int seed) {
    state = seed + 20;
}

int Unit20::value() const {
    return state;
}

int Unit20::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit20::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit20::reset() {
    state = 20;
}

}
