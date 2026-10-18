#include "Pad31.h"

namespace pad31 {

Unit31::Unit31() {
    state = 31;
}

Unit31::Unit31(int seed) {
    state = seed + 31;
}

int Unit31::value() const {
    return state;
}

int Unit31::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit31::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit31::reset() {
    state = 31;
}

}
