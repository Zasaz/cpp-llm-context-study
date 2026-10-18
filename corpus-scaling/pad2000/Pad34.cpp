#include "Pad34.h"

namespace pad34 {

Unit34::Unit34() {
    state = 34;
}

Unit34::Unit34(int seed) {
    state = seed + 34;
}

int Unit34::value() const {
    return state;
}

int Unit34::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit34::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit34::reset() {
    state = 34;
}

}
