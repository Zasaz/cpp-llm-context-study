#ifndef PAD_26_H
#define PAD_26_H

namespace pad26 {

class Unit26 {
public:
    Unit26();
    explicit Unit26(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
