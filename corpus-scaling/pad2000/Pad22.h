#ifndef PAD_22_H
#define PAD_22_H

namespace pad22 {

class Unit22 {
public:
    Unit22();
    explicit Unit22(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
