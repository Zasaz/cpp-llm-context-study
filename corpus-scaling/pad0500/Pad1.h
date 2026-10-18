#ifndef PAD_1_H
#define PAD_1_H

namespace pad1 {

class Unit1 {
public:
    Unit1();
    explicit Unit1(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
