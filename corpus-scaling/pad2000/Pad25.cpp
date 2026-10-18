#include "Pad25.h"

namespace pad25 {

Unit25::Unit25() {
    state = 25;
}

Unit25::Unit25(int seed) {
    state = seed + 25;
}

int Unit25::value() const {
    return state;
}

int Unit25::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit25::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit25::reset() {
    state = 25;
}

}
