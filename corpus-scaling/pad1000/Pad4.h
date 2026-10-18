#ifndef PAD_4_H
#define PAD_4_H

namespace pad4 {

class Unit4 {
public:
    Unit4();
    explicit Unit4(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
