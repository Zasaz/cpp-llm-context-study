#ifndef PAD_2_H
#define PAD_2_H

namespace pad2 {

class Unit2 {
public:
    Unit2();
    explicit Unit2(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
