#ifndef PAD_5_H
#define PAD_5_H

namespace pad5 {

class Unit5 {
public:
    Unit5();
    explicit Unit5(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
