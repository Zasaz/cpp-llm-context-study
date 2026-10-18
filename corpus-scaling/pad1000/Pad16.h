#ifndef PAD_16_H
#define PAD_16_H

namespace pad16 {

class Unit16 {
public:
    Unit16();
    explicit Unit16(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
