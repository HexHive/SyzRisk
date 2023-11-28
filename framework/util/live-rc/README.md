# Live Root Cause Identifier

This tool is designed to identify kernel regression bugs in Linux.

## Prerequisites

- Minimum python version: 3.8+

```bash
pip install GitPython
pip install progressbar2
pip install numpy
```

## How to Run

1) Run `[main.py](http://main.py)` with three options: `--init`, `--db`, `--repo`

- `--init`: generate `gitDB.json` file which contains a git commit history
- `--db`: a path of DB (to be saved)
- `--repo`: a path of Linux kernel

```bash
$ python3 main.py --init --db /path/to/db --repo /path/to/linux

[+] Collect git log...
100% (1060172 of 1060172) |##################################| Elapsed Time: 0:02:50 Time:  0:02:50

$ ls /path/to/db
gitDB.json ...
```

2) After obtaining `gitDB.json`, run `[main.py](http://main.py)` with three options: `--db`, `--repo`, `--date`

- `--date`: a reference date

```bash
$ python3 main.py --db /path/to/db --repo /path/to/linux --date 2021-10-03

[+] Analyze live root causes...
[+] saved 2323 commits in liveRC[2021-10-03].json

$ ls /path/to/db
gitDB.json liveRC[2022-01-03].json ...
```

