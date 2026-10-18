#ifndef PAD_18_H
#define PAD_18_H

namespace pad18 {

class Unit18 {
public:
    Unit18();
    explicit Unit18(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
