#include "Pad23.h"

namespace pad23 {

Unit23::Unit23() {
    state = 23;
}

Unit23::Unit23(int seed) {
    state = seed + 23;
}

int Unit23::value() const {
    return state;
}

int Unit23::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit23::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit23::reset() {
    state = 23;
}

}
