import os
import shutil
import sys
import tempfile

from analyzer import context_strategies, symbol_extractor
from analyzer.unit_tests import clean_response
from harness.grade import grade
from harness.report import thought_tokens
from harness.stats import (_chi2_sf, bootstrap_saving_ci, cliffs_delta,
                           cochran_q, fisher_exact, friedman,
                           friedman_tie_correction, holm, mann_whitney,
                           mcnemar_exact, permutation_mean_diff, pearson,
                           quartiles, rank_biserial, wilcoxon_floor,
                           wilcoxon_signed_rank)


PROJECT = {
    "Item.h": """
#ifndef ITEM_H
#define ITEM_H
#include <string>
class Item {
public:
    std::string name;
    double unitPrice;
    Item(const std::string& n, double p);
    double totalValue() const;
    bool isInStock() const;
};
#endif
""",
    "Item.cpp": """
#include "Item.h"
Item::Item(const std::string& n, double p) { name = n; unitPrice = p; }
double Item::totalValue() const { return unitPrice; }
bool Item::isInStock() const { return unitPrice > 0.0; }
""",
    "Shape.h": """
#ifndef SHAPE_H
#define SHAPE_H
class Shape {
public:
    virtual ~Shape();
    virtual double area() const;
};
class Circle : public Shape {
public:
    double radius;
    Circle(double r);
    double area() const;
};
#endif
""",
    "Shape.cpp": """
#include "Shape.h"
Shape::~Shape() { }
double Shape::area() const { return 0.0; }
Circle::Circle(double r) { radius = r; }
double Circle::area() const { return 3.14 * radius * radius; }
""",
    "Store.h": """
#ifndef STORE_H
#define STORE_H
#include "Item.h"
#include "Shape.h"
#include <vector>
class Store {
public:
    std::vector<Item> items;
    std::vector<Shape*> shapes;
    Store();
    ~Store();
    double stockValue() const;
    double drawnArea() const;
    int inStockCount() const;
};
#endif
""",
    "Store.cpp": """
#include "Store.h"
Store::Store() { }
Store::~Store() { }
double Store::stockValue() const {
    double total = 0.0;
    for (unsigned int i = 0; i < items.size(); ++i) {
        total = total + items[i].totalValue();
    }
    return total;
}
double Store::drawnArea() const {
    double total = 0.0;
    for (unsigned int i = 0; i < shapes.size(); ++i) {
        total = total + shapes[i]->area();
    }
    return total;
}
int Store::inStockCount() const {
    int n = 0;
    for (unsigned int i = 0; i < items.size(); ++i) {
        if (items[i].isInStock()) { n = n + 1; }
    }
    return n;
}
""",
    "Unrelated.h": """
#ifndef UNRELATED_H
#define UNRELATED_H
class Unrelated {
public:
    int value;
    Unrelated();
    int doubled() const;
};
#endif
""",
    "Unrelated.cpp": """
#include "Unrelated.h"
Unrelated::Unrelated() { value = 0; }
int Unrelated::doubled() const { return value * 2; }
""",
}

FAILURES = []


def check(name, condition, detail=""):
    if condition:
        print("  pass  %s" % name)
    else:
        print("  FAIL  %s   %s" % (name, detail))
        FAILURES.append(name)


def symbols():
    return symbol_extractor.extract_symbols_from_files(PROJECT)


def edges(syms):
    found = set()
    for name, record in syms.items():
        for target in record.get("calls", []):
            found.add((name, target))
    return found


def test_extractor():
    print("extractor")
    syms = symbols()
    found = edges(syms)

    check(
        "class symbol survives a destructor of the same name",
        syms.get("Store", {}).get("type") == "class"
        and syms["Store"]["file"] == "Store.h",
        "Store resolved to %r" % (syms.get("Store", {}).get("type"),),
    )
    check(
        "destructor is recorded as a method of its class",
        "Store::~Store" in syms and syms["Store::~Store"].get("owner") == "Store",
        "keys present: %s" % [k for k in syms if "~" in k],
    )
    check(
        "member field receiver resolves",
        ("Store::inStockCount", "Item::isInStock") in found,
        "edges from inStockCount: %s"
        % [e for e in found if e[0] == "Store::inStockCount"],
    )
    check(
        "container element receiver resolves through a subscript",
        ("Store::stockValue", "Item::totalValue") in found,
        "edges from stockValue: %s"
        % [e for e in found if e[0] == "Store::stockValue"],
    )
    check(
        "pointer element receiver resolves through a subscript",
        ("Store::drawnArea", "Shape::area") in found,
        "edges from drawnArea: %s" % [e for e in found if e[0] == "Store::drawnArea"],
    )
    check(
        "inheritance is recovered",
        "Circle" in syms and "Shape" in syms.get("Circle", {}).get("bases", []),
        "Circle bases: %s" % (syms.get("Circle", {}).get("bases"),),
    )
    check(
        "an unrelated class produces no edge into it",
        not any(t.startswith("Unrelated") for _s, t in found),
        "unexpected: %s" % [e for e in found if e[1].startswith("Unrelated")],
    )


def test_strategies():
    print("strategies")
    temp = tempfile.mkdtemp()
    try:
        for name, content in PROJECT.items():
            with open(os.path.join(temp, name), "w", encoding="utf-8") as fh:
                fh.write(content)

        picked = {}
        for strategy in ("S1", "S2", "S3"):
            _context, meta = context_strategies.select_context(
                temp, strategy=strategy, active_file="Store.cpp"
            )
            picked[strategy] = set(meta["files_selected_names"])

        check(
            "S2 selects the active file alone",
            picked["S2"] == {"Store.cpp"},
            "%s" % sorted(picked["S2"]),
        )
        check(
            "S1 selects every file",
            picked["S1"] == set(PROJECT),
            "%s" % sorted(picked["S1"]),
        )
        check(
            "S3 sits strictly between S2 and S1",
            len(picked["S2"]) < len(picked["S3"]) < len(picked["S1"]),
            "sizes S2=%d S3=%d S1=%d"
            % (len(picked["S2"]), len(picked["S3"]), len(picked["S1"])),
        )
        check(
            "S3 carries the header of every class it reaches",
            {"Store.h", "Item.h", "Shape.h"}.issubset(picked["S3"]),
            "%s" % sorted(picked["S3"]),
        )
        check(
            "S3 excludes a subsystem the point of interest never calls",
            "Unrelated.cpp" not in picked["S3"]
            and "Unrelated.h" not in picked["S3"],
            "%s" % sorted(picked["S3"]),
        )
        check(
            "a strategy selecting the same files emits the same context",
            context_strategies.select_context(
                temp, strategy="S1", active_file="Store.cpp"
            )[0]
            == context_strategies.select_context(
                temp, strategy="S1", active_file="Store.cpp"
            )[0],
        )
    finally:
        shutil.rmtree(temp, ignore_errors=True)


def test_response_handling():
    print("response handling")
    body = '#include "Store.h"\n#include <cassert>\nint main(){}'

    check(
        "a quoted include survives post-processing",
        '#include "Store.h"' in clean_response("```cpp\n" + body + "\n```"),
        "this defect scored the baseline at zero",
    )
    check(
        "a fenced block is recovered",
        clean_response("```cpp\n" + body + "\n```") == body,
    )
    check(
        "a block introduced by a sentence is recovered",
        clean_response("Here is the suite.\n\n```cpp\n" + body + "\n```") == body,
    )
    check(
        "an untagged fence is recovered",
        clean_response("```\n" + body + "\n```") == body,
    )
    check(
        "trailing prose is discarded",
        clean_response("```cpp\n" + body + "\n```\nHope this helps.") == body,
    )
    check(
        "an unfenced response is passed through",
        clean_response(body) == body,
    )


def test_grading():
    print("grading")
    suite = (
        '#include "Store.h"\n#include <cassert>\n'
        "void test_a() { assert(1); }\n"
        "void test_b() { assert(1); }\n"
        "int main() { test_a(); test_b(); }\n"
    )
    clean_output = "Test test_a: PASS\nTest test_b: PASS\n"

    good = grade(
        suite, suite,
        {"compiled": True, "ran": True, "exit_code": 0, "output": clean_output},
    )
    check("a correct suite is fully successful", good["fully_successful"])
    check("a correct suite records no failure kind", good["failure_kind"] is None)

    crashed = grade(
        suite, suite,
        {"compiled": True, "ran": True, "exit_code": -11, "output": clean_output},
    )
    check(
        "a segmentation fault is distinguished from a failed check",
        crashed["failure_kind"] == "crashed"
        and not crashed["fully_successful"],
        "%s" % crashed["failure_kind"],
    )

    aborted = grade(
        suite, suite,
        {"compiled": True, "ran": True, "exit_code": -6, "output": clean_output},
    )
    check(
        "a failed check is distinguished from a crash",
        aborted["failure_kind"] == "assertion_failed",
        "%s" % aborted["failure_kind"],
    )

    empty = (
        "#include <cassert>\nvoid test_a() { }\nint main() { test_a(); }\n"
    )
    degenerate = grade(
        empty, empty,
        {"compiled": True, "ran": True, "exit_code": 0, "output": ""},
    )
    check(
        "a suite that asserts nothing is not a success",
        not degenerate["asserted"] and not degenerate["fully_successful"],
    )

    refusal = grade("", "I cannot help with that request.", None)
    check("a refusal is not counted as a compile failure",
          refusal["refused"] and not refusal["compiled"])

    check(
        "a failure to compile is not fully successful",
        not grade(
            suite, suite,
            {"compiled": False, "ran": False, "exit_code": None, "output": ""},
        )["fully_successful"],
    )


def test_reporting():
    print("reporting")

    check(
        "a recorded thinking count is used as given",
        thought_tokens({"thought_tokens": 900, "total_tokens": 5000,
                        "prompt_tokens": 1000, "output_tokens": 3100}) == 900,
    )
    check(
        "an omitted thinking count means zero when the total accounts for it",
        thought_tokens({"thought_tokens": None, "total_tokens": 4100,
                        "prompt_tokens": 1000, "output_tokens": 3100}) == 0,
        "this defect reported a mean over only the runs that thought",
    )
    check(
        "an omitted thinking count is recovered from the total",
        thought_tokens({"thought_tokens": None, "total_tokens": 5000,
                        "prompt_tokens": 1000, "output_tokens": 3100}) == 900,
    )
    check(
        "a missing total leaves the thinking count unknown rather than zero",
        thought_tokens({"thought_tokens": None, "prompt_tokens": 1000,
                        "output_tokens": 3100}) is None,
    )

    check(
        "Fisher exact reproduces a known two-by-two result",
        abs(fisher_exact(3, 1, 1, 3) - 0.4857142857) < 1e-9,
        "%r" % fisher_exact(3, 1, 1, 3),
    )
    check(
        "Fisher exact returns certainty for an even split",
        abs(fisher_exact(5, 5, 5, 5) - 1.0) < 1e-9,
    )
    check(
        "Mann-Whitney reaches its exact floor for disjoint samples of five",
        abs(mann_whitney([1, 2, 3, 4, 5], [6, 7, 8, 9, 10])[1]
            - 2.0 / 252.0) < 1e-9,
        "%r" % (mann_whitney([1, 2, 3, 4, 5], [6, 7, 8, 9, 10]),),
    )
    check(
        "Mann-Whitney finds no difference between identical samples",
        abs(mann_whitney([1, 2, 3], [1, 2, 3])[1] - 1.0) < 1e-9,
    )
    check(
        "Cliff's delta is total for disjoint samples and zero for identical ones",
        cliffs_delta([1, 2, 3], [4, 5, 6]) == -1.0
        and cliffs_delta([1, 2, 3], [1, 2, 3]) == 0.0,
    )


def test_paired_statistics():
    print("paired statistics")

    import math

    check(
        "chi-square survival matches the closed form at two degrees of freedom",
        abs(_chi2_sf(2.0, 2) - math.exp(-1.0)) < 1e-12,
        "%r" % _chi2_sf(2.0, 2),
    )
    check(
        "chi-square survival matches the closed form at one degree of freedom",
        abs(_chi2_sf(3.841458821, 1) - 0.05) < 1e-6,
        "%r" % _chi2_sf(3.841458821, 1),
    )

    same = [[1, 2, 3]] * 4
    stat, p, totals = friedman(same)
    check(
        "Friedman reaches its exact floor when four blocks agree completely",
        abs(stat - 8.0) < 1e-9 and abs(p - 6.0 / 1296.0) < 1e-9,
        "stat=%r p=%r totals=%r" % (stat, p, totals),
    )
    _s, p_flat, _t = friedman([[1, 1, 1]] * 4)
    check(
        "Friedman finds no difference when every block is tied",
        abs(p_flat - 1.0) < 1e-9,
        "%r" % p_flat,
    )
    tied_stat, tied_p, _t = friedman([[1, 2, 2], [1, 2, 2]])
    check(
        "Friedman conditions the exact distribution on the observed ties",
        abs(tied_stat - 3.0) < 1e-9 and abs(tied_p - 1.0 / 3.0) < 1e-9,
        "stat=%r p=%r" % (tied_stat, tied_p),
    )

    check(
        "the Wilcoxon floor is unreachable at four blocks and reachable at six",
        abs(wilcoxon_floor(4) - 0.125) < 1e-12
        and abs(wilcoxon_floor(6) - 0.03125) < 1e-12,
    )
    _w, p_four = wilcoxon_signed_rank([1, 2, 3, 4], [2, 4, 6, 8])
    check(
        "Wilcoxon signed-rank attains its floor when four pairs all agree",
        abs(p_four - 0.125) < 1e-9,
        "%r" % p_four,
    )
    _w, p_tied = wilcoxon_signed_rank([1, 2, 3], [1, 2, 3])
    check(
        "Wilcoxon signed-rank is undefined when every difference is zero",
        p_tied is None,
        "%r" % p_tied,
    )

    q, p_q = cochran_q([[1, 0, 0]] * 6)
    check(
        "Cochran's Q matches a hand computation for six one-hot blocks",
        abs(q - 12.0) < 1e-9 and abs(p_q - math.exp(-6.0)) < 1e-12,
        "q=%r p=%r" % (q, p_q),
    )
    check(
        "Cochran's Q is undefined when no block discriminates",
        cochran_q([[1, 1, 1]] * 5) == (0.0, 1.0),
    )
    check(
        "Cochran's Q refuses non-binary input",
        cochran_q([[1, 2, 3]] * 4) == (None, None),
    )

    x = [True] * 10 + [False] * 2 + [True] * 5
    y = [False] * 10 + [True] * 2 + [True] * 5
    _d, p_mc = mcnemar_exact(x, y)
    check(
        "McNemar exact matches a hand computation for ten against two",
        abs(p_mc - 2.0 * (1 + 12 + 66) / 4096.0) < 1e-12,
        "%r" % p_mc,
    )
    check(
        "McNemar exact is certain when there are no discordant pairs",
        mcnemar_exact([1, 1, 0], [1, 1, 0]) == (0, 1.0),
    )

    adjusted = holm([("a", 0.01), ("b", 0.02), ("c", 0.03)])
    check(
        "Holm scales the smallest p by the number of comparisons",
        abs(adjusted[0][2] - 0.03) < 1e-12,
        "%r" % (adjusted,),
    )
    check(
        "Holm never lets an adjusted value fall below an earlier one",
        adjusted[0][2] <= adjusted[1][2] <= adjusted[2][2],
        "%r" % (adjusted,),
    )
    check(
        "Holm carries a missing p-value through untouched",
        holm([("a", None), ("b", 0.04)])[0][2] is None,
    )

    check(
        "Pearson r is one on an exactly increasing linear relation",
        abs(pearson([1, 2, 3, 4], [3, 5, 7, 9]) - 1.0) < 1e-12,
        "%r" % pearson([1, 2, 3, 4], [3, 5, 7, 9]),
    )
    check(
        "Pearson r is minus one when the relation is exactly decreasing",
        abs(pearson([1, 2, 3, 4], [9, 7, 5, 3]) + 1.0) < 1e-12,
        "%r" % pearson([1, 2, 3, 4], [9, 7, 5, 3]),
    )
    check(
        "Pearson r matches the hand computation on a non-degenerate case",
        abs(pearson([1, 2, 3], [1, 3, 2]) - 0.5) < 1e-12,
        "%r" % pearson([1, 2, 3], [1, 3, 2]),
    )
    check(
        "Pearson r is undefined when one variable does not vary",
        pearson([1, 2, 3], [5, 5, 5]) is None,
    )


def test_caching_statistics():
    print("caching sensitivity statistics")

    check(
        "permutation test returns 1.0 when the two samples are identical",
        permutation_mean_diff([1.0, 2.0, 3.0], [1.0, 2.0, 3.0])[1] == 1.0,
        "%r" % (permutation_mean_diff([1.0, 2.0, 3.0], [1.0, 2.0, 3.0]),),
    )
    check(
        "permutation test reaches its exact floor on perfect separation",
        abs(permutation_mean_diff([1.0] * 4, [9.0] * 4)[1] - 2.0 / 70.0) < 1e-12,
        "%r" % (permutation_mean_diff([1.0] * 4, [9.0] * 4),),
    )
    check(
        "permutation test is symmetric in its arguments",
        permutation_mean_diff([1.0, 4.0, 5.0], [2.0, 8.0, 9.0])[1]
        == permutation_mean_diff([2.0, 8.0, 9.0], [1.0, 4.0, 5.0])[1],
    )
    lo, hi = bootstrap_saving_ci([10.0] * 8, [5.0] * 8)
    check(
        "bootstrap saving interval collapses to a point without variation",
        abs(lo - 50.0) < 1e-9 and abs(hi - 50.0) < 1e-9,
        "%r" % ((lo, hi),),
    )
    lo, hi = bootstrap_saving_ci([10.0, 12.0, 8.0, 11.0], [9.0, 7.0, 10.0, 6.0])
    check(
        "bootstrap saving interval brackets the observed saving",
        lo <= 22.5 <= hi,
        "%r" % ((lo, hi),),
    )
    check(
        "bootstrap saving interval is undefined for a single observation",
        bootstrap_saving_ci([1.0], [2.0]) == (None, None),
    )


def test_blocked_effect_sizes():
    print("blocked effect sizes and tie correction")

    check(
        "tie correction is one when no block contains a tie",
        abs(friedman_tie_correction([[1, 2, 3], [3, 1, 2]]) - 1.0) < 1e-12,
        "%r" % friedman_tie_correction([[1, 2, 3], [3, 1, 2]]),
    )
    corpus = [[10, 10, 10], [10, 2, 10], [10, 7, 10], [7, 1, 7]]
    factor = friedman_tie_correction(corpus)
    check(
        "tie correction falls below one when blocks contain ties",
        factor is not None and factor < 1.0,
        "%r" % factor,
    )
    stat, _p, _t = friedman(corpus)
    corrected = stat / factor
    check(
        "the corpus accuracy blocks give 3.375 uncorrected and 6.000 corrected",
        abs(stat - 3.375) < 1e-9 and abs(corrected - 6.0) < 1e-9,
        "%r %r" % (stat, corrected),
    )
    prompts = [[1624, 805, 1331], [1747, 762, 1747],
               [5010, 1417, 3122], [9909, 1494, 5174]]
    pstat, _pp, _pt = friedman(prompts)
    pcorr = pstat / friedman_tie_correction(prompts)
    check(
        "the corpus prompt blocks give 7.125 uncorrected and 7.600 corrected",
        abs(pstat - 7.125) < 1e-9 and abs(pcorr - 7.6) < 1e-9,
        "%r %r" % (pstat, pcorr),
    )
    check(
        "rank-biserial is plus one when every pair moves the same way",
        abs(rank_biserial([5, 6, 7, 8], [1, 2, 3, 4]) - 1.0) < 1e-12,
        "%r" % rank_biserial([5, 6, 7, 8], [1, 2, 3, 4]),
    )
    check(
        "rank-biserial is minus one when every pair moves the other way",
        abs(rank_biserial([1, 2, 3, 4], [5, 6, 7, 8]) + 1.0) < 1e-12,
    )
    check(
        "rank-biserial is undefined when no paired difference is non-zero",
        rank_biserial([3, 3, 3], [3, 3, 3]) is None,
    )
    check(
        "rank-biserial respects the matching rather than the margins",
        rank_biserial([1, 4], [2, 3]) == 0.0,
        "%r" % rank_biserial([1, 4], [2, 3]),
    )
    q = quartiles([1, 2, 3, 4, 5, 6, 7, 8, 9, 10])
    check(
        "quartiles interpolate the median of an even sample",
        abs(q["median"] - 5.5) < 1e-12,
        "%r" % q,
    )
    check(
        "quartiles flag a point beyond one and a half interquartile ranges",
        quartiles([1] * 9 + [100])["outliers"] == [100],
        "%r" % quartiles([1] * 9 + [100]),
    )
    check(
        "quartiles are undefined below four observations",
        quartiles([1, 2, 3]) is None,
    )


def test_grading_boundary():
    print("grading boundary, adversarial fixtures")

    trivial = (
        "#include <cassert>\n#include <iostream>\n"
        "void test_placeholder() {\n"
        "    assert(true);\n"
        '    std::cout << "Test test_placeholder: PASS\\n";\n'
        "}\n"
        "int main() { test_placeholder(); }\n"
    )
    graded = grade(
        trivial, trivial,
        {"compiled": True, "ran": True, "exit_code": 0,
         "output": "Test test_placeholder: PASS\n"},
    )
    check(
        "the trivial suite is graded successful, which is the documented bound",
        graded["fully_successful"],
        "%r" % graded,
    )

    empty = "int main() { return 0; }\n"
    graded = grade(empty, empty,
                   {"compiled": True, "ran": True, "exit_code": 0, "output": ""})
    check(
        "a suite declaring no test function is not successful",
        not graded["fully_successful"] and graded["test_functions"] == 0,
    )

    silent = (
        "#include <cassert>\n"
        "void test_a() { assert(1); }\n"
        "void test_b() { assert(1); }\n"
        "int main() { test_a(); test_b(); }\n"
    )
    graded = grade(silent, silent,
                   {"compiled": True, "ran": True, "exit_code": 0,
                    "output": "Test test_a: PASS\n"})
    check(
        "a suite printing fewer markers than it declares functions fails",
        not graded["fully_successful"] and graded["pass_lines"] == 1,
        "%r" % graded["pass_lines"],
    )

    outside = (
        "#include <cassert>\n#include <iostream>\n"
        "void test_a() { assert(1); }\n"
        'int main() { test_a(); std::cout << "PASS\\nPASS\\n"; }\n'
    )
    graded = grade(outside, outside,
                   {"compiled": True, "ran": True, "exit_code": 0,
                    "output": "PASS\nPASS\n"})
    check(
        "markers printed outside a test function still satisfy the count",
        graded["fully_successful"],
        "%r" % graded["pass_lines"],
    )

    refusal = "I cannot generate that test suite."
    graded = grade("", refusal, {"compiled": False, "ran": False,
                                 "exit_code": None, "output": ""})
    check(
        "a refusal is separated from a compile failure",
        graded["refused"] and not graded["compiled"],
    )


def main():
    print("instrument self-test, extractor %s, strategies %s\n"
          % (symbol_extractor.EXTRACTOR_VERSION,
             context_strategies.STRATEGY_VERSION))
    test_extractor()
    test_strategies()
    test_response_handling()
    test_grading()
    test_grading_boundary()
    test_reporting()
    test_paired_statistics()
    test_caching_statistics()
    test_blocked_effect_sizes()

    if FAILURES:
        print("\n%d check(s) failed: %s" % (len(FAILURES), ", ".join(FAILURES)))
        return 1
    print("\nall checks passed")
    return 0


if __name__ == "__main__":
    sys.exit(main())
