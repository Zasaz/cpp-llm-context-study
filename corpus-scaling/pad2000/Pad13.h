#ifndef PAD_13_H
#define PAD_13_H

namespace pad13 {

class Unit13 {
public:
    Unit13();
    explicit Unit13(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
