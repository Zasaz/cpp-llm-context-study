#ifndef PAD_21_H
#define PAD_21_H

namespace pad21 {

class Unit21 {
public:
    Unit21();
    explicit Unit21(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
