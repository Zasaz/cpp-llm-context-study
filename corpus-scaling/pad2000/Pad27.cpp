#include "Pad27.h"

namespace pad27 {

Unit27::Unit27() {
    state = 27;
}

Unit27::Unit27(int seed) {
    state = seed + 27;
}

int Unit27::value() const {
    return state;
}

int Unit27::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit27::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit27::reset() {
    state = 27;
}

}
