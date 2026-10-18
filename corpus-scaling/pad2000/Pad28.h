#ifndef PAD_28_H
#define PAD_28_H

namespace pad28 {

class Unit28 {
public:
    Unit28();
    explicit Unit28(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
