#include "Pad33.h"

namespace pad33 {

Unit33::Unit33() {
    state = 33;
}

Unit33::Unit33(int seed) {
    state = seed + 33;
}

int Unit33::value() const {
    return state;
}

int Unit33::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit33::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit33::reset() {
    state = 33;
}

}
