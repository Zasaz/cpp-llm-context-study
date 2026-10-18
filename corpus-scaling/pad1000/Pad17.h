#ifndef PAD_17_H
#define PAD_17_H

namespace pad17 {

class Unit17 {
public:
    Unit17();
    explicit Unit17(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
