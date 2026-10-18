#include "Pad1.h"

namespace pad1 {

Unit1::Unit1() {
    state = 1;
}

Unit1::Unit1(int seed) {
    state = seed + 1;
}

int Unit1::value() const {
    return state;
}

int Unit1::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit1::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit1::reset() {
    state = 1;
}

}
