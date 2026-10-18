#ifndef PAD_30_H
#define PAD_30_H

namespace pad30 {

class Unit30 {
public:
    Unit30();
    explicit Unit30(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
