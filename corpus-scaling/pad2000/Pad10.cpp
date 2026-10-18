#include "Pad10.h"

namespace pad10 {

Unit10::Unit10() {
    state = 10;
}

Unit10::Unit10(int seed) {
    state = seed + 10;
}

int Unit10::value() const {
    return state;
}

int Unit10::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit10::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit10::reset() {
    state = 10;
}

}
