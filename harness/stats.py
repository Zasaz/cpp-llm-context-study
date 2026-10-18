import itertools
import math
import random
import statistics


def _hypergeom(a, b, c, d):
    n = a + b + c + d
    return (math.comb(a + b, a) * math.comb(c + d, c)) / math.comb(n, a + c)


def fisher_exact(a, b, c, d):
    observed = _hypergeom(a, b, c, d)
    row1, row2 = a + b, c + d
    col1 = a + c
    lo = max(0, col1 - row2)
    hi = min(row1, col1)
    total = 0.0
    for x in range(lo, hi + 1):
        p = _hypergeom(x, row1 - x, col1 - x, row2 - col1 + x)
        if p <= observed * (1.0 + 1e-9):
            total += p
    return min(1.0, total)


def cliffs_delta(x, y):
    if not x or not y:
        return None
    greater = sum(1 for a in x for b in y if a > b)
    lesser = sum(1 for a in x for b in y if a < b)
    return (greater - lesser) / float(len(x) * len(y))


def cliffs_magnitude(delta):
    if delta is None:
        return "undefined"
    d = abs(delta)
    if d < 0.147:
        return "negligible"
    if d < 0.33:
        return "small"
    if d < 0.474:
        return "medium"
    return "large"


def _rank(values):
    order = sorted(range(len(values)), key=lambda i: values[i])
    ranks = [0.0] * len(values)
    i = 0
    while i < len(order):
        j = i
        while j + 1 < len(order) and values[order[j + 1]] == values[order[i]]:
            j += 1
        shared = (i + j) / 2.0 + 1.0
        for k in range(i, j + 1):
            ranks[order[k]] = shared
        i = j + 1
    return ranks


def _u_statistic(x, y):
    combined = list(x) + list(y)
    ranks = _rank(combined)
    r1 = sum(ranks[: len(x)])
    return r1 - len(x) * (len(x) + 1) / 2.0


def mann_whitney(x, y, exact_limit=200000):
    x, y = list(x), list(y)
    if not x or not y:
        return None, None
    n1, n2 = len(x), len(y)
    u_obs = _u_statistic(x, y)
    u_obs = min(u_obs, n1 * n2 - u_obs)

    if math.comb(n1 + n2, n1) <= exact_limit:
        combined = x + y
        idx = range(n1 + n2)
        count = 0
        total = 0
        for pick in itertools.combinations(idx, n1):
            chosen = set(pick)
            a = [combined[i] for i in idx if i in chosen]
            b = [combined[i] for i in idx if i not in chosen]
            u = _u_statistic(a, b)
            u = min(u, n1 * n2 - u)
            total += 1
            if u <= u_obs + 1e-9:
                count += 1
        return u_obs, min(1.0, count / float(total))

    mean_u = n1 * n2 / 2.0
    sd_u = math.sqrt(n1 * n2 * (n1 + n2 + 1) / 12.0)
    if sd_u == 0:
        return u_obs, 1.0
    z = (u_obs - mean_u) / sd_u
    return u_obs, 2.0 * 0.5 * math.erfc(abs(z) / math.sqrt(2))


def _chi2_sf(x, df):
    if x <= 0:
        return 1.0
    if df % 2 == 0:
        term = math.exp(-x / 2.0)
        total = term
        for i in range(1, df // 2):
            term *= (x / 2.0) / i
            total += term
        return min(1.0, total)
    s = math.sqrt(x)
    total = math.erfc(s / math.sqrt(2.0))
    term = math.exp(-x / 2.0) * s * math.sqrt(2.0 / math.pi)
    for i in range(1, (df - 1) // 2 + 1):
        total += term
        term *= x / (2.0 * i + 1.0)
    return min(1.0, max(0.0, total))


def _friedman_statistic(blocks):
    b = len(blocks)
    k = len(blocks[0])
    totals = [0.0] * k
    for row in blocks:
        ranks = _rank(list(row))
        for j, r in enumerate(ranks):
            totals[j] += r
    stat = (12.0 / (b * k * (k + 1)) * sum(t * t for t in totals)
            - 3.0 * b * (k + 1))
    return stat, totals


def friedman(blocks, exact_limit=200000):
    blocks = [list(row) for row in blocks if row is not None]
    if len(blocks) < 2:
        return None, None, None
    k = len(blocks[0])
    if any(len(row) != k for row in blocks) or k < 3:
        return None, None, None
    b = len(blocks)
    stat, totals = _friedman_statistic(blocks)

    if math.factorial(k) ** b <= exact_limit:
        per_block = [list(itertools.permutations(row)) for row in blocks]
        hits = 0
        total = 0
        for combo in itertools.product(*per_block):
            s, _t = _friedman_statistic(combo)
            total += 1
            if s >= stat - 1e-9:
                hits += 1
        return stat, min(1.0, hits / float(total)), totals

    return stat, _chi2_sf(stat, k - 1), totals


def cochran_q(blocks):
    blocks = [list(row) for row in blocks if row is not None]
    if len(blocks) < 2:
        return None, None
    k = len(blocks[0])
    if any(len(row) != k for row in blocks) or k < 3:
        return None, None
    if any(v not in (0, 1, True, False) for row in blocks for v in row):
        return None, None
    col = [sum(row[j] for row in blocks) for j in range(k)]
    rows = [sum(row) for row in blocks]
    denom = k * sum(rows) - sum(r * r for r in rows)
    if denom == 0:
        return 0.0, 1.0
    q = ((k - 1) * (k * sum(c * c for c in col) - sum(col) ** 2)) / float(denom)
    return q, _chi2_sf(q, k - 1)


def mcnemar_exact(x, y):
    x, y = list(x), list(y)
    if len(x) != len(y) or not x:
        return None, None
    b = sum(1 for a, c in zip(x, y) if a and not c)
    c = sum(1 for a, c2 in zip(x, y) if not a and c2)
    n = b + c
    if n == 0:
        return 0, 1.0
    lo = min(b, c)
    tail = sum(math.comb(n, i) for i in range(lo + 1)) / float(2 ** n)
    return b - c, min(1.0, 2.0 * tail)


def wilcoxon_signed_rank(x, y=None, exact_limit=1048576):
    x = list(x)
    if y is not None:
        y = list(y)
        if len(x) != len(y):
            return None, None
        diffs = [a - b for a, b in zip(x, y)]
    else:
        diffs = x
    diffs = [d for d in diffs if d != 0]
    n = len(diffs)
    if n == 0:
        return None, None

    ranks = _rank([abs(d) for d in diffs])
    w_plus = sum(r for d, r in zip(diffs, ranks) if d > 0)
    w_minus = sum(r for d, r in zip(diffs, ranks) if d < 0)
    w_obs = min(w_plus, w_minus)

    if 2 ** n <= exact_limit:
        hits = 0
        total = 0
        for signs in itertools.product((-1, 1), repeat=n):
            wp = sum(r for s, r in zip(signs, ranks) if s > 0)
            wm = sum(r for s, r in zip(signs, ranks) if s < 0)
            total += 1
            if min(wp, wm) <= w_obs + 1e-9:
                hits += 1
        return w_obs, min(1.0, hits / float(total))

    mean_w = n * (n + 1) / 4.0
    sd_w = math.sqrt(n * (n + 1) * (2 * n + 1) / 24.0)
    if sd_w == 0:
        return w_obs, 1.0
    z = (w_obs - mean_w) / sd_w
    return w_obs, math.erfc(abs(z) / math.sqrt(2))


def wilcoxon_floor(n):
    if n < 1:
        return None
    return min(1.0, 2.0 / (2 ** n))


def holm(pairs):
    usable = [(label, p) for label, p in pairs if p is not None]
    if not usable:
        return [(label, p, None) for label, p in pairs]
    order = sorted(range(len(usable)), key=lambda i: usable[i][1])
    m = len(usable)
    adjusted = [None] * m
    running = 0.0
    for rank, i in enumerate(order):
        value = min(1.0, (m - rank) * usable[i][1])
        running = max(running, value)
        adjusted[i] = running
    out = dict((usable[i][0], adjusted[i]) for i in range(m))
    return [(label, p, out.get(label)) for label, p in pairs]


def pearson(x, y):
    x, y = list(x), list(y)
    if len(x) != len(y) or len(x) < 2:
        return None
    mx, my = statistics.mean(x), statistics.mean(y)
    sxy = sum((a - mx) * (b - my) for a, b in zip(x, y))
    sxx = sum((a - mx) ** 2 for a in x)
    syy = sum((b - my) ** 2 for b in y)
    if sxx <= 0 or syy <= 0:
        return None
    return sxy / math.sqrt(sxx * syy)


def bootstrap_ci(values, confidence=0.95, iterations=10000, seed=12345):
    values = [v for v in values if isinstance(v, (int, float))]
    if len(values) < 2:
        return None, None
    rng = random.Random(seed)
    n = len(values)
    means = []
    for _ in range(iterations):
        means.append(statistics.mean(rng.choice(values) for _ in range(n)))
    means.sort()
    lo = int((1.0 - confidence) / 2.0 * iterations)
    hi = int((1.0 + confidence) / 2.0 * iterations) - 1
    return means[lo], means[hi]


def summarise(values):
    values = [v for v in values if isinstance(v, (int, float))]
    if not values:
        return {"n": 0, "mean": None, "sd": None, "lo": None, "hi": None}
    mean = statistics.mean(values)
    sd = statistics.stdev(values) if len(values) > 1 else 0.0
    lo, hi = bootstrap_ci(values)
    return {"n": len(values), "mean": mean, "sd": sd, "lo": lo, "hi": hi,
            "min": min(values), "max": max(values)}


def permutation_mean_diff(x, y, exact_limit=400000):
    x = [v for v in x if isinstance(v, (int, float))]
    y = [v for v in y if isinstance(v, (int, float))]
    if len(x) < 1 or len(y) < 1:
        return None, None
    pool = x + y
    n = len(x)
    m = len(pool) - n
    total = float(sum(pool))
    observed = abs(statistics.mean(x) - statistics.mean(y))
    arrangements = math.comb(len(pool), n)
    hits = 0
    if arrangements <= exact_limit:
        for combo in itertools.combinations(range(len(pool)), n):
            left = sum(pool[i] for i in combo)
            if abs(left / n - (total - left) / m) >= observed - 1e-12:
                hits += 1
        return observed, hits / float(arrangements)
    rng = random.Random(12345)
    index = list(range(len(pool)))
    for _ in range(exact_limit):
        rng.shuffle(index)
        left = sum(pool[i] for i in index[:n])
        if abs(left / n - (total - left) / m) >= observed - 1e-12:
            hits += 1
    return observed, hits / float(exact_limit)


def bootstrap_saving_ci(baseline, variant, confidence=0.95,
                        iterations=20000, seed=12345):
    baseline = [v for v in baseline if isinstance(v, (int, float))]
    variant = [v for v in variant if isinstance(v, (int, float))]
    if len(baseline) < 2 or len(variant) < 2:
        return None, None
    rng = random.Random(seed)
    savings = []
    for _ in range(iterations):
        a = statistics.mean(rng.choice(baseline) for _ in baseline)
        b = statistics.mean(rng.choice(variant) for _ in variant)
        if a:
            savings.append((a - b) / a * 100.0)
    if not savings:
        return None, None
    savings.sort()
    lo = int((1.0 - confidence) / 2.0 * len(savings))
    hi = int((1.0 + confidence) / 2.0 * len(savings)) - 1
    return savings[lo], savings[hi]


def friedman_tie_correction(blocks):
    blocks = [list(row) for row in blocks if row is not None]
    if not blocks:
        return None
    b = len(blocks)
    k = len(blocks[0])
    if k < 2:
        return None
    adjustment = 0.0
    for row in blocks:
        counts = {}
        for value in row:
            counts[value] = counts.get(value, 0) + 1
        for t in counts.values():
            adjustment += t ** 3 - t
    denominator = b * float(k ** 3 - k)
    if denominator == 0:
        return None
    factor = 1.0 - adjustment / denominator
    return factor if factor > 0 else None


def friedman_corrected(blocks):
    stat, p, totals = friedman(blocks)
    if stat is None:
        return None, None, None, None
    factor = friedman_tie_correction(blocks)
    corrected = stat / factor if factor else None
    return stat, corrected, p, totals


def rank_biserial(x, y=None):
    x = list(x)
    if y is not None:
        y = list(y)
        if len(x) != len(y):
            return None
        diffs = [a - b for a, b in zip(x, y)]
    else:
        diffs = list(x)
    diffs = [d for d in diffs if d != 0]
    if not diffs:
        return None
    ranks = _rank([abs(d) for d in diffs])
    w_plus = sum(r for d, r in zip(diffs, ranks) if d > 0)
    w_minus = sum(r for d, r in zip(diffs, ranks) if d < 0)
    total = w_plus + w_minus
    if total == 0:
        return None
    return (w_plus - w_minus) / total


def rank_biserial_magnitude(value):
    if value is None:
        return "undefined"
    size = abs(value)
    if size < 0.1:
        return "negligible"
    if size < 0.3:
        return "small"
    if size < 0.5:
        return "medium"
    return "large"


def quartiles(values):
    values = sorted(v for v in values if isinstance(v, (int, float)))
    n = len(values)
    if n < 4:
        return None

    def at(p):
        pos = p * (n - 1)
        low = int(math.floor(pos))
        high = int(math.ceil(pos))
        if low == high:
            return values[low]
        return values[low] + (values[high] - values[low]) * (pos - low)

    q1, q2, q3 = at(0.25), at(0.5), at(0.75)
    span = q3 - q1
    lower = q1 - 1.5 * span
    upper = q3 + 1.5 * span
    inside = [v for v in values if lower <= v <= upper]
    outliers = [v for v in values if v < lower or v > upper]
    return {
        "q1": q1, "median": q2, "q3": q3,
        "min": min(inside) if inside else values[0],
        "max": max(inside) if inside else values[-1],
        "outliers": outliers,
    }
