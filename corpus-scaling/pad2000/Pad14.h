#ifndef PAD_14_H
#define PAD_14_H

namespace pad14 {

class Unit14 {
public:
    Unit14();
    explicit Unit14(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
