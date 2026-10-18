#ifndef PAD_27_H
#define PAD_27_H

namespace pad27 {

class Unit27 {
public:
    Unit27();
    explicit Unit27(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
