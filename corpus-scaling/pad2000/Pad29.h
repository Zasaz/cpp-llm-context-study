#ifndef PAD_29_H
#define PAD_29_H

namespace pad29 {

class Unit29 {
public:
    Unit29();
    explicit Unit29(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
