#include "Pad15.h"

namespace pad15 {

Unit15::Unit15() {
    state = 15;
}

Unit15::Unit15(int seed) {
    state = seed + 15;
}

int Unit15::value() const {
    return state;
}

int Unit15::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit15::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit15::reset() {
    state = 15;
}

}
