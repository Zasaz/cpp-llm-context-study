#ifndef PAD_25_H
#define PAD_25_H

namespace pad25 {

class Unit25 {
public:
    Unit25();
    explicit Unit25(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
