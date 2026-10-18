#ifndef PAD_7_H
#define PAD_7_H

namespace pad7 {

class Unit7 {
public:
    Unit7();
    explicit Unit7(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
