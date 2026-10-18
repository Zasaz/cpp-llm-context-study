#ifndef PAD_33_H
#define PAD_33_H

namespace pad33 {

class Unit33 {
public:
    Unit33();
    explicit Unit33(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
