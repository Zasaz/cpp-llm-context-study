#ifndef PAD_6_H
#define PAD_6_H

namespace pad6 {

class Unit6 {
public:
    Unit6();
    explicit Unit6(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
