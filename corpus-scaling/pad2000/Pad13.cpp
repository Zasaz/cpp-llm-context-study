#include "Pad13.h"

namespace pad13 {

Unit13::Unit13() {
    state = 13;
}

Unit13::Unit13(int seed) {
    state = seed + 13;
}

int Unit13::value() const {
    return state;
}

int Unit13::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit13::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit13::reset() {
    state = 13;
}

}
