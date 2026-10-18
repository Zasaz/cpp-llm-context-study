#ifndef PAD_15_H
#define PAD_15_H

namespace pad15 {

class Unit15 {
public:
    Unit15();
    explicit Unit15(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
