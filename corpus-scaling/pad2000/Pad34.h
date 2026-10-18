#ifndef PAD_34_H
#define PAD_34_H

namespace pad34 {

class Unit34 {
public:
    Unit34();
    explicit Unit34(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
