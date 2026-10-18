#ifndef PAD_11_H
#define PAD_11_H

namespace pad11 {

class Unit11 {
public:
    Unit11();
    explicit Unit11(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
