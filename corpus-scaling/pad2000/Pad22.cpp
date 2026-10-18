#include "Pad22.h"

namespace pad22 {

Unit22::Unit22() {
    state = 22;
}

Unit22::Unit22(int seed) {
    state = seed + 22;
}

int Unit22::value() const {
    return state;
}

int Unit22::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit22::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit22::reset() {
    state = 22;
}

}
