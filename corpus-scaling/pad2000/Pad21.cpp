#include "Pad21.h"

namespace pad21 {

Unit21::Unit21() {
    state = 21;
}

Unit21::Unit21(int seed) {
    state = seed + 21;
}

int Unit21::value() const {
    return state;
}

int Unit21::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit21::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit21::reset() {
    state = 21;
}

}
