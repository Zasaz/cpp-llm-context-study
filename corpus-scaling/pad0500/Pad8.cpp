#include "Pad8.h"

namespace pad8 {

Unit8::Unit8() {
    state = 8;
}

Unit8::Unit8(int seed) {
    state = seed + 8;
}

int Unit8::value() const {
    return state;
}

int Unit8::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit8::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit8::reset() {
    state = 8;
}

}
