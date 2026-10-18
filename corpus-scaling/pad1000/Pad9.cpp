#include "Pad9.h"

namespace pad9 {

Unit9::Unit9() {
    state = 9;
}

Unit9::Unit9(int seed) {
    state = seed + 9;
}

int Unit9::value() const {
    return state;
}

int Unit9::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit9::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit9::reset() {
    state = 9;
}

}
