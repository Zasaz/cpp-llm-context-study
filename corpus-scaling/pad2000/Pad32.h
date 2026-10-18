#ifndef PAD_32_H
#define PAD_32_H

namespace pad32 {

class Unit32 {
public:
    Unit32();
    explicit Unit32(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
