import sys

from analyzer import symbol_extractor

SAMPLE = {
    "Math.h": """
class Vector3 {
    public:
    float x, y, z;
    void Normalize();
};
""",
    "Math.cpp": """
#include "Math.h"
void Vector3::Normalize() {
    x = 0;
}
""",
    "Player.h": """
#include "Math.h"
class Player {
    Vector3 position;
    void Update();
};
""",
    "Player.cpp": """
#include "Player.h"
void Player::Update() {
    position.Normalize();
}
""",
    "Game.cpp": """
#include "Player.h"
void RunGame() {
    Player p;
    p.Update();
}
class Boss : public Player {
};
""",
}

EXPECTED_CALLS = {
    ("Player::Update", "Vector3::Normalize"),
    ("RunGame", "Player::Update"),
}
EXPECTED_INHERITANCE = {("Boss", "Player")}


def main():
    symbols = symbol_extractor.extract_symbols_from_files(SAMPLE)

    classes = sorted(n for n, s in symbols.items() if s["type"] == "class")
    functions = sorted(n for n, s in symbols.items() if s["type"] == "function")

    calls = set()
    for name, sym in symbols.items():
        for target in sym.get("calls", []):
            calls.add((name, target))

    inheritance = set()
    for name, sym in symbols.items():
        for base in sym.get("bases", []):
            if base in symbols:
                inheritance.add((name, base))

    print("extractor version : %s" % symbol_extractor.EXTRACTOR_VERSION)
    print("classes  (%d)      : %s" % (len(classes), ", ".join(classes)))
    print("functions(%d)      : %s" % (len(functions), ", ".join(functions)))
    print("call edges (%d)    :" % len(calls))
    for src, dst in sorted(calls):
        print("    %s -> %s" % (src, dst))
    print("inheritance (%d)   :" % len(inheritance))
    for src, dst in sorted(inheritance):
        print("    %s -> %s" % (src, dst))

    missing_calls = EXPECTED_CALLS - calls
    missing_inh = EXPECTED_INHERITANCE - inheritance
    spurious = calls - EXPECTED_CALLS

    print("\nfidelity against the hand-checked expectation:")
    print("  expected calls recovered : %d/%d"
          % (len(EXPECTED_CALLS) - len(missing_calls), len(EXPECTED_CALLS)))
    print("  expected inheritance     : %d/%d"
          % (len(EXPECTED_INHERITANCE) - len(missing_inh), len(EXPECTED_INHERITANCE)))
    print("  spurious call edges      : %d" % len(spurious))
    for src, dst in sorted(missing_calls):
        print("  MISSING call %s -> %s" % (src, dst))
    for src, dst in sorted(spurious):
        print("  SPURIOUS call %s -> %s" % (src, dst))

    return 0 if not missing_calls and not missing_inh else 1


if __name__ == "__main__":
    sys.exit(main())
