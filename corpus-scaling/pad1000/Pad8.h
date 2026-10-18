#ifndef PAD_8_H
#define PAD_8_H

namespace pad8 {

class Unit8 {
public:
    Unit8();
    explicit Unit8(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
