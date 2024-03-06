# SyzRisk

SyzRisk [ASIA CCS'24] is a change-pattern-based kernel regression fuzzer that prioritizes potentially risky code changes in the recent patch commits.

## Directory Overview

- `artifact`: Artifact data used in the paper [ASIA CCS'24].
- `framework`: All pre-fuzzing components.
    - Root-cause collection (Linux)
    - Pattern creation
    - Risk estimation
    - Weight calculation
- `syzkaller`: Syzkaller with regression fuzzing support.

## Reference

 - Gwangmu Lee, Duo Xu, Solmaz Salimi, Byoungyoung Lee, and Mathias Payer. 2024. "SyzRisk: A Change-Pattern-Based Continuous Kernel Regression Fuzzer." In ACM ASIA Conference on Computer and Communications Security (ASIA CCS ’24), July 01–05, 2024, Singapore. [[paper]](./paper.pdf)

```bibtex
@inproceedings{lee2024syzrisk,
  title = {{SyzRisk}: A Change-Pattern-Based Continuous Kernel Regression Fuzzer}, 
  author = {Gwangmu Lee, Duo Xu, Solmaz Salimi, Byoungyoung Lee, and Mathias Payer},
  booktitle = {ACM ASIA Conference on Computer and Communications Security (ASIA CCS ’24)},
  year = {2024}
}
```
