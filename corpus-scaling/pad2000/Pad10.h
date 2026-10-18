#ifndef PAD_10_H
#define PAD_10_H

namespace pad10 {

class Unit10 {
public:
    Unit10();
    explicit Unit10(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
