#ifndef PAD_12_H
#define PAD_12_H

namespace pad12 {

class Unit12 {
public:
    Unit12();
    explicit Unit12(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
