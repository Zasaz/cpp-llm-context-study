#ifndef PAD_24_H
#define PAD_24_H

namespace pad24 {

class Unit24 {
public:
    Unit24();
    explicit Unit24(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
