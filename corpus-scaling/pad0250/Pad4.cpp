#include "Pad4.h"

namespace pad4 {

Unit4::Unit4() {
    state = 4;
}

Unit4::Unit4(int seed) {
    state = seed + 4;
}

int Unit4::value() const {
    return state;
}

int Unit4::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit4::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit4::reset() {
    state = 4;
}

}
