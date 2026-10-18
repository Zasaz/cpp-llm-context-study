#include "Pad3.h"

namespace pad3 {

Unit3::Unit3() {
    state = 3;
}

Unit3::Unit3(int seed) {
    state = seed + 3;
}

int Unit3::value() const {
    return state;
}

int Unit3::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit3::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit3::reset() {
    state = 3;
}

}
