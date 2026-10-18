# Efficiency and Accuracy of Structural Context Mapping in LLM-Based C++ Development Assistants

Research artefact for the Master's thesis submitted to the Faculty of Computer Science, University of Vienna, 2026.

**Author:** Umair Ahmed (Matrikelnummer 12309368)  
**Supervisor:** Univ.-Prof. Dipl.-Ing. Dr. Helmut Hlavacs

---

## Overview

This repository contains the full source code, evaluation harness, corpus, and campaign results for a study comparing three context-selection strategies for an LLM-based C++ code assistant:

- **S1** – Whole-project concatenation  
- **S2** – Active file only  
- **S3** – Structural selection via a symbol-graph traversal  

The assistant is a Visual Studio Code extension backed by a Flask analysis service. Two measurement campaigns (120 runs each) compared the strategies on four purpose-built C++ projects, using unit-test generation as the task and compilation and execution of generated suites as the oracle.

---

## Repository Structure

```
├── analyzer/           # Context strategies, symbol extractor, architecture view
│   ├── context_strategies.py   # S1/S2/S3/S4/S5 selection logic
│   └── symbol_extractor.py     # Regex-based C++ symbol extractor
├── harness/            # Measurement and analysis modules
│   ├── run.py          # Campaign runner
│   ├── blocked.py      # Friedman/Wilcoxon inference
│   ├── chapter5.py     # Table and figure generation for Chapter 5
│   ├── caching.py      # Caching sensitivity analysis
│   ├── mutants.py      # Seeded-fault (E1) experiment
│   ├── scaling.py      # Controlled scaling (E4) experiment
│   ├── baseline.py     # Include-closure baseline (E2) experiment
│   ├── ablation.py     # Reference-graph ablation
│   └── stats.py        # Statistical primitives (Friedman, rank-biserial, bootstrap)
├── corpus/             # Four bespoke C++ projects (geometry, inventory, scheduler, telemetry)
├── corpus-scaling/     # Five padded geometry variants (152–1682 LOC) for E4
├── results/            # Campaign artefacts (prompts, suites, compile logs, manifests)
│   ├── campaign_20260821T122302/   # Main default-reasoning campaign
│   ├── campaign_20260821T124344/   # Main suppressed-reasoning campaign
│   └── mutants-active-file.json   # E1 seeded-fault results
├── gemini_integration/ # Gemini API client (google-genai 1.47.0)
├── cpp_handler/        # Compile-and-run sandbox
├── verify_fidelity.py  # Extractor fidelity measurement vs. libclang
└── main.py             # Flask service entry point
```

---

## Reproducing the Results

### Prerequisites

```bash
pip install -r requirements.txt
```

Set your Gemini API key in a `.env` file (not included):
```
GEMINI_API_KEY=your_key_here
```

### Run the analysis (no API calls needed — uses archived results)

All tables and figures in Chapter 5 of the thesis can be regenerated from the archived campaign data without re-running the model:

```bash
# Main accuracy and cost tables (Tables 5.1–5.8)
python -m harness.chapter5 results/campaign_20260821T122302 results/campaign_20260821T124344

# Caching sensitivity analysis (Table 5.9)
python -m harness.caching results/campaign_20260821T122302 results/campaign_20260821T124344

# Seeded-fault experiment E1 (Table 5.10)
python -m harness.mutants --results results/mutants-active-file.json

# Controlled scaling experiment E4 (Table 5.11)
python -m harness.scaling corpus-scaling results/campaign_20260821T124344

# Include-closure baseline experiment E2 (Table 5.12)
python -m harness.baseline results/campaign_20260821T122302 results/campaign_20260821T124344
```

### Re-run the campaigns (requires API key and Gemini balance)

```bash
# Default reasoning arm
python -m harness.run corpus results/my_campaign_default

# Suppressed reasoning arm
python -m harness.run corpus results/my_campaign_suppressed --thinking-budget 0
```

### Extractor fidelity (requires libclang and macOS SDK)

```bash
FIDELITY_SDK_PATH=/path/to/MacOSX15.5.sdk python verify_fidelity.py
```

---

## Key Results

| Metric | Finding |
|--------|---------|
| Accuracy (S1 vs S3) | Identical observed suite-success counts across all projects and both campaigns |
| Accuracy (S2) | Halved on projects where the active file depends on headers |
| Cost crossover (default arm) | Between 598 and 1203 LOC; does not survive cached-rate billing |
| Cost advantage (suppressed arm) | S3 cheaper on 3/4 projects; rank-biserial +1.00 |
| Latency reduction | ~40% from suppressing reasoning, not from strategy |
| Extractor fidelity | Precision 1.000, call-edge recall 0.911 on corpus without templates/macros |
| Seeded-fault detection | 26/28 distinct faults detected by ≥1 suite under S1 and S3 |

---

## Citation

If you use this code or data, please cite the thesis:

> Umair Ahmed. *Efficiency and Accuracy of Structural Context Mapping in LLM-Based C++ Development Assistants*. Master's thesis, University of Vienna, Faculty of Computer Science, 2026.

---

## License

This research artefact is made available for academic reproducibility. All rights reserved by the author.
