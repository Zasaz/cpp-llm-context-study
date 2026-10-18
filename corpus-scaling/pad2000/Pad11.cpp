#include "Pad11.h"

namespace pad11 {

Unit11::Unit11() {
    state = 11;
}

Unit11::Unit11(int seed) {
    state = seed + 11;
}

int Unit11::value() const {
    return state;
}

int Unit11::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit11::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit11::reset() {
    state = 11;
}

}
