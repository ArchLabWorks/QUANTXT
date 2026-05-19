# 📐 Full Mathematical Diagram of QuantXT

Below is the complete pipeline:

```
 ┌──────────────────────────────────────────────────────────────┐
 │                        Scenario Vector                        │
 │                     x = (x₁, …, xₙ)                           │
 └───────────────┬──────────────────────────────────────────────┘
                 │  xⱼ ← scenario_fieldⱼ
                 ▼
 ┌──────────────────────────────────────────────────────────────┐
 │                     Normalization Layer                      │
 └───────────────┬──────────────────────────────────────────────┘
                 │  Apply one of:
                 │   • clip01(xⱼ) = min(1, max(0, xⱼ))
                 │   • LOGF(xⱼ) = log(1 + a|xⱼ|) sign(xⱼ)
                 │   • SMOOTH_STEP(xⱼ; a,b)
                 │   • PWL(xⱼ)
                 │   • SPL(xⱼ)
                 ▼
         Normalized Inputs:  ẋⱼ ∈ [0,1]
                 ▼
 ┌──────────────────────────────────────────────────────────────┐
 │                     Factor Modules M₁…M₁₃                    │
 └───────────────┬──────────────────────────────────────────────┘
                 │  For each module i:
                 │
                 │   Fᵢ = Σⱼ  wᵢⱼ Tᵢⱼ(ẋⱼ)
                 │        + Σⱼₖ vᵢⱼₖ Tᵢⱼₖ(ẋⱼ, ẋₖ)
                 │
                 │  Examples:
                 │   • M₁ (Inflation): w₁π SMOOTH_STEP(ẋπ) + …
                 │   • M₂ (Credit Stress): w₂cs PWL(ẋspread) + …
                 ▼
         Factor Vector:  F = (F₁, …, F₁₃)
                 ▼
 ┌──────────────────────────────────────────────────────────────┐
 │                     System Aggregation                       │
 └───────────────┬──────────────────────────────────────────────┘
                 │
                 │  Linear Mix:
                 │     L = Σᵢ αᵢ Fᵢ
                 │
                 │  Nonlinear Shaping:
                 │     L' = SMOOTH_STEP(L; a,b)
                 │
                 │  Optional Convexity:
                 │     L'' = (L')^γ
                 │
                 │  Clamping:
                 │     S = clip01(L'')
                 ▼
 ┌──────────────────────────────────────────────────────────────┐
 │                     Final Stress Score S                     │
 │                         S ∈ [0,1]                            │
 └──────────────────────────────────────────────────────────────┘
```

---

# 🔍 Expanded Mathematical Form (Fully Connected)

Below is the same diagram expressed as a single composite function:

\[
S = \text{clip01}\!\left(
\left[
\text{SMOOTH\_STEP}\!\left(
\sum_{i=1}^{13} \alpha_i
\left(
\sum_j w_{ij} T_{ij}(\tilde{x}_j)
+
\sum_{j,k} v_{ijk} T_{ijk}(\tilde{x}_j,\tilde{x}_k)
\right)
; a,b
\right)
\right]^\gamma
\right)
\]

Where:

- \(\tilde{x}_j = N_j(x_j)\) is one of  
  **clip01**,  
  **LOGF**,  
  **SMOOTH_STEP**,  
  **PWL**,  
  **SPL**.

- \(T_{ij}\) and \(T_{ijk}\) are transforms from the same family.

- \(M_i\) are the 13 factor modules.

- SYSTEM is the nonlinear aggregator.

---

# 🧩 Layer‑by‑Layer Dependency Graph

```
x₁ ──┐
x₂ ──┼──► Normalization ──► ẋ₁…ẋₙ ──┐
…    │                               │
xₙ ──┘                               ▼
                             ┌────────────────┐
                             │  Modules M₁…M₁₃│
                             └────────────────┘
                                      │
                                      ▼
                               F₁…F₁₃ (vector)
                                      │
                                      ▼
                           Linear Mix: L = Σ αᵢFᵢ
                                      │
                                      ▼
                         Nonlinear Step: L' = smooth(L)
                                      │
                                      ▼
                         Convexity: L'' = (L')^γ
                                      │
                                      ▼
                          Final Clamp: S = clip01(L'')
```

---

# 🧠 What This Diagram Shows

This is the **entire mathematical architecture** of QuantXT:

- A **normalization front‑end** that stabilizes inputs  
- A **multi‑factor nonlinear middle layer**  
- A **smooth, convex, bounded output layer**  

It is mathematically equivalent to:

- A shallow neural network  
- With custom activation functions  
- And a final sigmoid‑like output  

But implemented in deterministic C89 with 8087 FP constraints.
