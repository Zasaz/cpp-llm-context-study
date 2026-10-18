#include "Pad29.h"

namespace pad29 {

Unit29::Unit29() {
    state = 29;
}

Unit29::Unit29(int seed) {
    state = seed + 29;
}

int Unit29::value() const {
    return state;
}

int Unit29::combine(int other) const {
    int total = state;
    for (int i = 0; i < 3; ++i) {
        total = total + other;
    }
    return total;
}

int Unit29::scale(int factor) const {
    int total = 0;
    for (int i = 0; i < factor; ++i) {
        total = total + state;
    }
    return total;
}

void Unit29::reset() {
    state = 29;
}

}
