#include "Pad17.h"

namespace pad17 {

Unit17::Unit17() {
    state = 17;
}

Unit17::Unit17(int seed) {
    state = seed + 17;
}

int Unit17::value() const {
    return state;
}

int Unit17::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit17::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit17::reset() {
    state = 17;
}

}
