#include "Pad2.h"

namespace pad2 {

Unit2::Unit2() {
    state = 2;
}

Unit2::Unit2(int seed) {
    state = seed + 2;
}

int Unit2::value() const {
    return state;
}

int Unit2::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit2::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit2::reset() {
    state = 2;
}

}
