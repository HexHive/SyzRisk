# SyzRisk Artifacts

Evaluation artifacts in the paper "SyzRisk: A Change-Pattern-Based Continuous Kernel Regression Fuzzer" \[ASIA CCS'24\]. 

## Overview

Each directory represents the artifact used in the corresponding section (in the prefix), consisting of:

- Data (e.g., risks, weights, or bug counts)
- Plot-drawing script
- Makefile

You can create figures by typing `make` on each directory if there is a corresponding figure.

## Directory Summary

 - `sec1.syzbot_regbug`: The percentage of regression bugs in Syzbot per year (Figure 1).
 - `sec2.2-weight_dist`: The distribution of AFLChurn-style weights (Figure 3).
 - `sec8.2-pattern_risks`: The risks of the 23 patterns (Figure 10).
 - `sec8.3-rc_weighting`: The comparison between AFLChurn-style-based and pattern-based weight distributions (Figure 11).
 - `sec8.4-tte_comparison`: The TTE comparison between fuzzers (Table 4).
 - `sec8.5-complete`: The intersection of discovered bugs between different fuzzers (Figure 12).

## Note

Please let us know if some figure-drawing scripts don't function correctly.
