#!/usr/bin/python

import os, sys, re
import json
import git
from progressbar import progressbar
from common import getstat
from datetime import datetime

repo_path = "../../garage/linux"
hexsha_path = "../../data/rc/linux-rc-info.json"
start_date = "2020-01-01 20:00:00"
end_date = "2021-01-01 20:00:00"
out_path = "selected_rcs.json"

SEMI_COMMIT_SIZE = 5

repo = git.Repo(repo_path)
with open(hexsha_path, 'r') as f:
    rc_infos = json.load(f)
    rc_sha_bts = {}
    for rci in rc_infos:
        if (rci['fixed_commit'] not in rc_sha_bts):
            rc_sha_bts[rci['fixed_commit']] = set()
        rc_sha_bts[rci['fixed_commit']].update(set(rci['vuln_type']))

for sha in rc_sha_bts.keys():
    rc_sha_bts[sha] = list(rc_sha_bts[sha])

commits = repo.iter_commits(None, "", after=start_date, before=end_date)
commits = list(filter(lambda x: str(x) in rc_sha_bts.keys(), commits))
commits = list(filter(lambda x: len(x.parents) == 1, commits))

selected_rcs = {}

for c in progressbar(commits):
    _, dcs, _ = getstat.GetSingleCommitStat(repo, c)
    if (not dcs['func']): continue
    
    if (len(dcs['func']) <= SEMI_COMMIT_SIZE):
        selected_rcs[str(c)] = rc_sha_bts[str(c)]

with open(out_path, 'w') as f:
    json.dump(selected_rcs, f, indent=2)
