#ifndef PAD_9_H
#define PAD_9_H

namespace pad9 {

class Unit9 {
public:
    Unit9();
    explicit Unit9(int seed);
    int value() const;
    int combine(int other) const;
    int scale(int factor) const;
    void reset();
private:
    int state;
};

}

#endif
