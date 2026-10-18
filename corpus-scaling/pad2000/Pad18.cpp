#include "Pad18.h"

namespace pad18 {

Unit18::Unit18() {
    state = 18;
}

Unit18::Unit18(int seed) {
    state = seed + 18;
}

int Unit18::value() const {
    return state;
}

int Unit18::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit18::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit18::reset() {
    state = 18;
}

}
