#include "Pad19.h"

namespace pad19 {

Unit19::Unit19() {
    state = 19;
}

Unit19::Unit19(int seed) {
    state = seed + 19;
}

int Unit19::value() const {
    return state;
}

int Unit19::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit19::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit19::reset() {
    state = 19;
}

}
