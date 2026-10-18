#ifndef PAD_31_H
#define PAD_31_H

namespace pad31 {

class Unit31 {
public:
    Unit31();
    explicit Unit31(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
