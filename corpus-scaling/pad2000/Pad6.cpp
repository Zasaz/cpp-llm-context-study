#include "Pad6.h"

namespace pad6 {

Unit6::Unit6() {
    state = 6;
}

Unit6::Unit6(int seed) {
    state = seed + 6;
}

int Unit6::value() const {
    return state;
}

int Unit6::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit6::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit6::reset() {
    state = 6;
}

}
