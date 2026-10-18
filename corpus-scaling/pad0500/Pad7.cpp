#include "Pad7.h"

namespace pad7 {

Unit7::Unit7() {
    state = 7;
}

Unit7::Unit7(int seed) {
    state = seed + 7;
}

int Unit7::value() const {
    return state;
}

int Unit7::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit7::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit7::reset() {
    state = 7;
}

}
