#ifndef PAD_23_H
#define PAD_23_H

namespace pad23 {

class Unit23 {
public:
    Unit23();
    explicit Unit23(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
