#ifndef PAD_3_H
#define PAD_3_H

namespace pad3 {

class Unit3 {
public:
    Unit3();
    explicit Unit3(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
