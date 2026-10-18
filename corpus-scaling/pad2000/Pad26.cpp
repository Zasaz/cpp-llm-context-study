#include "Pad26.h"

namespace pad26 {

Unit26::Unit26() {
    state = 26;
}

Unit26::Unit26(int seed) {
    state = seed + 26;
}

int Unit26::value() const {
    return state;
}

int Unit26::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit26::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit26::reset() {
    state = 26;
}

}
