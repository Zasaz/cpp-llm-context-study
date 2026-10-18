#ifndef PAD_20_H
#define PAD_20_H

namespace pad20 {

class Unit20 {
public:
    Unit20();
    explicit Unit20(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
