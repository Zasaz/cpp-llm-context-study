#include "Pad5.h"

namespace pad5 {

Unit5::Unit5() {
    state = 5;
}

Unit5::Unit5(int seed) {
    state = seed + 5;
}

int Unit5::value() const {
    return state;
}

int Unit5::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit5::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit5::reset() {
    state = 5;
}

}
