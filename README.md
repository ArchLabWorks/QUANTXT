# QUANTXT

**Sovereign Risk Modeling Engine**  
*C89 Standard· Watcom Open C V1.9 · Real-mode DOS · Intel 8087 FPU*

---

> Built to answer a real question: given current US sovereign conditions, what does the data say the stress probability is?  
> The answer runs on a 1981 coprocessor with a mean calibration error of **0.0038**.

---

## What It Is

QUANTXT is a deterministic, nonlinear, multi-factor macro-financial simulation engine implemented in C89 and executed on IBM XT-class hardware. It computes a sovereign risk index by aggregating 13 economic, political, and market-based factor modules through a weighted nonlinear pipeline with reflexive feedback.

It is current an alpha demo. It is not a tutorial project. The goal is to build a calibrated analytical instrument — built to model US sovereign risk dynamics using real FRED macroeconomic data and market inputs, cross-referenced against contemporary macro frameworks including Paul Tudor Jones's bond market stress thesis and Garrett Jones's structural fiscal narrative.

The engine began as a YAML schema, was validated in Python, and was hardened into C89/8087 production code — with calibration tightening at each step, not loosening. The most constrained environment produced the most accurate results.

---

## Architecture

```
QUANTXT.EXE              — Main engine + DOS text-mode UI
```

# QUANTXT Code Map

## Architecture Overview

```
MAIN
├── INTRO          (splash screen, no deps)
├── BROWSER        (scenario picker UI)
│   └── SCENARIO
├── FILEBRO        (file picker UI, no module deps)
├── SCENARIO       (scenario registry + loader + manual input)
├── MODULES        (M1–M13 stress functions + compute_system_out)  ◄─┐
├── SYSTEM         (SYSTEM_engine, init_default_params)  ────────────┘ ⚠ CYCLE
│   ├── MODULES                                                        ⚠ CYCLE
│   ├── UTIL       (clip01, LOGF, SMOOTH_STEP, PWL, SPL)
│   ├── HISTORY    (run history ring buffer)
│   └── ENGINE     (OLS scoring engine)
├── DASH           (dashboard UI)
├── QXCALIB        (calibration runner)
│   └── ENGINE
└── ENGINE         (run_engine — pure float scorer)
```

---

## Module Reference

### MAIN.C
**Entry point.** Menu loop: browse scenarios, manual input, load .TXT file, run calibration.  
Calls: `run_intro`, `browse_scenarios`, `manual_input`, `browse_txt_files`,
`load_scenario_file_multi`, `fill_state_from_scenario`, `compute_system_out`,
`draw_dashboard`, `qxcalib_run`

---

### INTRO.C / INTRO.H
**CGA splash screen.**  
640×200 mono vector logo, grid animation, BIOS tick timing.  
No dependencies on other QUANTXT modules.  
Exports: `run_intro(void)`

---

### BROWSER.C / BROWSER.H
**Scenario browser UI.**  
Scrollable list of loaded scenarios; returns selected index or -1 (ESC).  
Depends on: `state.h`, `system.h`, `scenario.h`, `colors.h`  
Exports: `browse_scenarios(void)`

---

### FILEBRO.C / FILEBRO.H
**DOS 8.3 file picker.**  
Scans `*.TXT` in working directory; scrollable picker returns filename.  
Depends on: `colors.h` only  
Globals: `file_list[64][13]`, `file_count`  
Exports: `browse_txt_files(char *out_filename)`, `load_txt_file_list(void)`

---

### SCENARIO.C / SCENARIO.H
**Scenario registry + loader.**  
Holds built-in scenarios (`gfc_2008`, `debtceiling_2011`, `today_2026`).  
Loads external `.TXT` packs; provides manual input UI.  
Depends on: `state.h`, `colors.h`  
Globals: `scenarios[256]`, `scenario_count`  
Exports: `fill_state_from_scenario`, `manual_input`, `load_scenario_file_multi`

---

### ENGINE.C / ENGINE.H
**OLS scoring engine.**  
Pure function: takes `State`, returns `float` score in [0, 1].  
19 OLS-calibrated weights (RMSE 0.020, 30-point precedent set).  
Depends on: `state.h` only  
Exports: `run_engine(const State *st)`

---

### MODULES.C / MODULES.H  ⚠
**Stress sub-modules M1–M13 + `compute_system_out`.**  
Each M-function is pure, deterministic, returns a [0,1] score.  
Evaluation order matters: M3→M1, M5→M2, M9→M6.  
Depends on: `state.h`, `system.h` ← **causes cycle with SYSTEM**  
Contains a local `static clip01()` that duplicates `UTIL.C` — should use `util.h`.  
Exports: `M1`–`M13`, `compute_system_out(const State*, SystemOut*)`

| Function | Inputs | Notes |
|---|---|---|
| M1 | int_rev, M3_score | Interest burden |
| M2 | tail_risk, liq_gap, M5_score | Liquidity/tail risk |
| M3 | xdate, cbo_deficit | X-date + fiscal |
| M4 | usd_reserve_share | USD reserve decline |
| M5 | ofr, hy_spread | Credit stress |
| M6 | sahm, M9_score, lagged_ai | Cycle + AI reflexivity |
| M7 | dxy_mom, oil_price | Dollar + oil shock |
| M9 | ai_capex | AI capex stabilizer |
| M10 | geopolitical_risk | Geo risk |
| M13 | investor_sentiment | Sentiment (contrarian) |

---

### SYSTEM.C / SYSTEM.H  ⚠
**Full nonlinear macro-risk engine.**  
Runs M-modules in two passes, applies AI damping, EMA smoothing,
regime classification, and PONR probability.  
Depends on: `modules.h`, `util.h`, `history.h`, `engine.h` ← **causes cycle with MODULES**  
Exports: `init_default_params(Params*)`, `SYSTEM_engine(..., SystemOut*)`

**Two-pass evaluation:**
```
Pass 1 (independent): M3, M4, M5, M7, M9, M10, M13
Pass 2 (dependent):   M1(←M3), M2(←M5), M6(←M9)
```

---

### DASH.C / DASH.H
**XT-style dashboard UI.**  
Centered 60-column layout; risk bars with color thresholds; two-page display.  
⚠ `draw_dashboard()` calls itself recursively on page-flip — stack risk on 16KB DOS stack.  
Depends on: `state.h`, `system.h`, `colors.h`  
Exports: `draw_dashboard(const SystemOut*, const State*)`

---

### QXCALIB.C / QXCALIB.H
**Calibration runner.**  
Parses `CALIB.TXT` key=value precedent blocks, runs `run_engine` against each,
computes SSE/MSE, writes `CALIB_RESULT.TXT`.  
Depends on: `engine.h`, `state.h`  
Exports: `qxcalib_run(precedent_file, result_file, QXCalibResult*)`

---

### UTIL.C / UTIL.H
**Math utilities.**  
No QUANTXT dependencies — safe to include anywhere.  
Exports:

| Function | Signature | Purpose |
|---|---|---|
| `clip01` | `(double x)` | Clamp to [0, 1] |
| `LOGF` | `(x, mid, k)` | Logistic sigmoid |
| `SMOOTH_STEP` | `(x, th[], n, k)` | Hermite multi-threshold step |
| `PWL` | `(x, bp[], slopes[], n)` | Piecewise linear |
| `SPL` | `(x, knots[], vals[], n, irreversible, *state)` | Linear spline + optional ratchet |

---

### HISTORY.C / HISTORY.H
**Run history ring buffer.**  
Stores up to 256 entries of (M8, M9, M10) per run.  
No QUANTXT dependencies.  
Exports: `history_reset(History*)`, `history_append(History*, m8, m9, m10)`

---

### STATE.H
**Shared data types — no logic, no dependencies.**  
Defines `Scenario` (16 fields + name) and `State` (19 fields including `lagged_ai`, `infl`, `unemp`, `gdp`).

### COLORS.H
**CGA/EGA/VGA color constants.**  
Standard palette (0–15) + semantic UI aliases (`COL_NORMAL`, `COL_HEADER`, etc.).  
No dependencies.

---

## Known Issues

| # | File | Issue | Fix |
|---|---|---|---|
| 1 | `SYSTEM.H` ↔ `MODULES.H` | **Circular dependency** — `MODULES.H` includes `system.h` for `SystemOut`; `SYSTEM.C` includes `modules.h` | Move `SystemOut`/`Params` to `state.h`, or move `compute_system_out` into `SYSTEM.C` |
| 2 | `MODULES.C` | **Duplicate `clip01`** — local `static` copy shadows `UTIL.C` version | Drop local copy, add `#include "util.h"` |
| 3 | `DASH.C` | **Recursive `draw_dashboard()`** — page-flip calls itself, consuming stack | Restructure with internal loop or `goto` |

---

## Data Flow (happy path)

```
startup
  └─ run_intro()
  └─ main menu loop
       ├─ browse_scenarios() / manual_input() / load_scenario_file_multi()
       │       └─ fills State
       ├─ compute_system_out(State → SystemOut)   [MODULES]
       │       └─ M3,M4,M5,M7,M9,M10,M13  (pass 1)
       │       └─ M1,M2,M6               (pass 2)
       │       └─ weighted sum → raw_stress
       │       └─ AI damping → M8
       │       └─ regime classification
       └─ draw_dashboard(SystemOut, State)         [DASH]
```

---

## File Index

| File | Role | Deps (user headers) |
|---|---|---|
| `STATE.H` | Types only | — |
| `COLORS.H` | Constants only | — |
| `UTIL.H/C` | Math helpers | — |
| `HISTORY.H/C` | Ring buffer | — |
| `ENGINE.H/C` | OLS scorer | `state.h` |
| `INTRO.H/C` | Splash screen | — |
| `FILEBRO.H/C` | File picker | `colors.h` |
| `SCENARIO.H/C` | Scenario registry | `state.h`, `colors.h` |
| `MODULES.H/C` | M1–M13 + aggregator | `state.h`, `system.h` ⚠ |
| `SYSTEM.H/C` | Full risk engine | `state.h`, `modules.h` ⚠, `util.h`, `history.h`, `engine.h` |
| `DASH.H/C` | Dashboard UI | `state.h`, `system.h`, `colors.h` |
| `BROWSER.H/C` | Scenario browser UI | `state.h`, `system.h`, `scenario.h`, `colors.h` |
| `QXCALIB.H/C` | Calibration runner | `state.h`, `engine.h` |
| `MAIN.C` | Entry point | all of the above |

---

## Mathematical Model

**Pipeline:** `x → normalize(x) → Fᵢ → S`

### Normalization Layer (`UTIL.C`)
| Transform | Function |
|-----------|----------|
| `clip01` | Bounds inputs to [0,1] |
| `LOGF` | Log compression `y = log(1+a\|x\|)·sign(x)` |
| `SMOOTH_STEP` | Cubic S-curve `s(t) = 3z² - 2z³` |
| `PWL` | Piecewise linear between calibrated breakpoints |
| `SPL` | Spline-like smoothing across multiple knots |

### Factor Layer (`MODULES.C`) — M1–M13
Each module is a pure function. Includes cross-factor interaction terms:

```
Fᵢ = Σⱼ wᵢⱼ Tᵢⱼ(x̃ⱼ)  +  Σⱼ,ₖ vᵢⱼₖ Tᵢⱼₖ(x̃ⱼ, x̃ₖ)
```

| Module | Factor |
|--------|--------|
| M1 | Inflation pressure |
| M2 | Credit stress |
| M3 | Liquidity conditions |
| M4 | Policy stance |
| M5 | Growth expectations |
| M6 | External shocks |
| M7 | Volatility |
| M9 | Sentiment |
| M10 | Structural risk |
| M13 | Higher-order nonlinear composite |

### System Layer (`SYSTEM.C`)
```
L   = Σᵢ αᵢ Fᵢ                    — weighted linear mix
L'  = SMOOTH_STEP(L; a, b)         — nonlinear shaping
L'' = (L')ᵞ                        — optional convexity
S   = clip01(L'')                   — S ∈ [0,1]
```

**Reflexive feedback:** M8 output at period *t* feeds forward into period *t+1*, capturing the self-reinforcing dynamics of sovereign stress episodes. Damping functions stabilize the loop.

### Outputs
| Output | Description |
|--------|-------------|
| `S` | Stress score ∈ [0,1] |
| Regime | Low / Medium / High Risk |
| PONR | Point-of-no-return crisis threshold probability |

---

## Calibration

Calibrated against 60 scenarios spanning the full US economic cycle — from early expansion through severe crisis and back through recovery — using FRED macroeconomic series and market data.

| Metric | Value |
|--------|-------|
| Scenarios (initial) | 30 |
| Mean error (initial) | 0.135473 |
| Scenarios (current) | 60 |
| Total error (current) | 0.228393 |
| **Mean error (current)** | **0.003807** |
| Error reduction | ~35× improvement |
| Data sources | FRED + market data |

At mean error **0.0038** on a 0–1 scale, the model is hitting targets with sub-percent accuracy across the full economic cycle.

---

## SCENARIO

Scenarios are in the following format. The goal will be to soon unify the Scenario and Calibration data formats. 

# name int_rev debt_gdp usd_reserve_share cbo_deficit xdate sahm tail_risk liq_gap ofr hy_spread dxy_mom oil_price ai_capex geopolitical_risk investor_sentiment

managed_norm_1 22 70 65 3 90 0.2 1.2 1.0 0.95 600 5 80 1.0 0.5 0.4
managed_norm_2 23 72 66 3.2 88 0.18 1.1 1.05 0.96 580 4 82 1.1 0.45 0.42
managed_norm_3 21 68 64 2.8 92 0.22 1.3 0.98 0.94 620 6 78 0.9 0.55 0.38

---

## Input Vector — 19-Field Sovereign Risk Schema

| Field | Description | Range |
|-------|-------------|-------|
| `int_rev` | Interest as share of revenue | 0.0 – 1.0 |
| `debt_gdp` | Debt as share of GDP | 0.0 – 2.0 |
| `usd_reserve_share` | USD share of global reserves | % |
| `cbo_deficit` | CBO projected deficit | % GDP |
| `xdate` | Days to debt ceiling breach | 0 – 360 |
| `sahm` | Sahm rule recession indicator | 0.0 – 1.0 |
| `tail_risk` | Tail risk index | 0.0 – 1.0 |
| `liq_gap` | Liquidity gap index | 0.0 – 1.0 |
| `ofr` | OFR financial stress index | 0.0 – 1.0 |
| `hy_spread` | High yield spread index | 0.0 – 1.0 |
| `dxy_mom` | DXY momentum index | 0.0 – 1.0 |
| `oil_price` | Crude oil price | USD/barrel |
| `ai_capex` | AI capital expenditure index | 0.0 – 5.0 |
| `lagged_ai` | Prior period AI capex | 0.0 – 5.0 |
| `geopolitical_risk` | Geopolitical risk index | 0.0 – 1.0 |
| `investor_sentiment` | Investor sentiment index | 0.0 – 1.0 |
| `infl` | Inflation rate | 0.0 – 1.0 |
| `unemp` | Unemployment rate | 0.0 – 1.0 |
| `gdp` | GDP growth rate | -0.1 to 0.1 |

---

## Calibration File Format

```
# Scenario Name
int_rev=0.32
debt_gdp=0.95
usd_reserve_share=57.0
cbo_deficit=11.0
xdate=20
sahm=0.40
tail_risk=0.90
liq_gap=0.85
ofr=0.90
hy_spread=0.88
dxy_mom=0.70
oil_price=60.0
ai_capex=0.15
lagged_ai=0.20
geopolitical_risk=0.75
investor_sentiment=0.15
infl=0.02
unemp=0.12
gdp=-0.05
target=0.92
```

Scenarios are separated by blank lines. Comments begin with `#` or `;`.

---

## Build

```bash
In Open Watcom V1.9 Use the attached maker files rules
Making Sure Your op stack=16384 and -fp87 is enabled
```

**Requirements:**
- Watcom C/C++ (`wcc` / `wlink`)
- Real-mode DOS (or DOSBox for modern systems)
- Memory model: large (`-ml`)
- 8087 recommended for full precision

**System requirements:**
- IBM PC/XT or compatible (8087 for CoMath Processor acceleration)
- 256 KB RAM minimum
- CGA graphics
- DOS 3.3+

---

## Usage

| Key | Action |
|-----|--------|
| `F1` | Load scenario file |
| `F2` | Browse scenario files |
| `ENTER` | Run engine |
| `G` | Open QXGraph visualization |
| `C` | Calibration mode |
| `ESC` | Quit |

---

## Development History

| Phase | Technology | Purpose |
|-------|-----------|---------|
| I | YAML | Parameter schema; model contract definition; PTJ-Weimar benchmark |
| II | Python 3 | Mathematical validation; scenario runner; output format lock-in |
| III | C89 / 8087 | Production engine; deterministic execution; calibration hardening |

The flat-file TXT format served as the binding data contract across all three phases. AI Scalped scenarios feed directly into `QUANTXT.EXE` without transformation.

---

## Why C89 on XT Hardware

The constraint is the point.

An engineer who can build a calibrated, nonlinear, multi-factor sovereign risk engine within the constraints of a 1981 coprocessor understands the computation — not just the tools. No runtime. No garbage collector. No dynamic loader. No abstraction noise between the mathematics and the metal.

The 8087's 80-bit extended precision and real-mode determinism produced *tighter* calibration than Python floating point. The constraints proved that a big idea could be ported to one of the earliest iterations of home PC.

> *If it works on an IBM XT, it works anywhere.*

---

## File Structure

```
QUANTXT/
├── MAIN.C          — Orchestrator
├── ENGINE.C        — Computational hub
├── MODULES.C       — M1–M13 factor modules
├── SYSTEM.C        — Aggregation + regime classification
├── SCENARIO.C      — Scenario loader → STATE struct
├── FILEBRO.C       — File browser + I/O
├── DASH.C          — ASCII dashboard renderer
├── QXCALIB.C       — Calibration engine
├── HISTORY.C       — Scenario state tracking
├── UTIL.C          — Math transforms (clip01, LOGF, SMOOTH_STEP, PWL, SPL)
├── STATE.H         — Core struct definitions
├── ENGINE.H        — Engine interface
├── MODULES.H       — Module interfaces
├── SYSTEM.H        — System layer interface
├── QXCALIB.H       — Calibration interface
├── UTIL.H          — Utility function declarations
├── COLORS.H        — CGA color constants
├── QUANTXT.MK      — Watcom makefile
├── CALIB.TXT       — 60-scenario calibration dataset

```

---

## Status

- [x] Core engine — stable
- [x] 60-scenario calibration — mean error 0.0038
- [x] YAML → Python → C89/8087 evolution complete
- [x] CGA QXGraph visualization operational
- [x] Multi-scenario TXT loader
- [x] Header architecture hardened (debugging sprint complete)
- [x] Multi variable scenario simulations used for calibrating the dataset, and more to be performed over time.
- [ ] Developing Forecast modeling on sequential daily data
- [ ] Improving Regime transition validation on real observed path
- [ ] Current US environment live stress assessment
- [ ] QXGraph multi-parameter overlay + zoom

---

## Author

**ArchitectureLabs**
Systems engineer. 16 years industrial operations experience.  Building quantitative tools at the intersection of macro-financial analysis and constraint-driven systems engineering.

---

*QUANTXT v1.1 — May 19 2026*
