#ifndef PAD_19_H
#define PAD_19_H

namespace pad19 {

class Unit19 {
public:
    Unit19();
    explicit Unit19(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
