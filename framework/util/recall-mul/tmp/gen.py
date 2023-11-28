#!/usr/bin/python

import os, sys, re
import json
import git
from progressbar import progressbar
from common import getstat
from datetime import datetime

repo_path = "../../garage/linux"
start_date = "2020-01-01 20:00:00"
end_date = "2021-01-01 20:00:00"
out_path = "semi-commit.json"

SEMI_COMMIT_SIZE = 5

repo = git.Repo(repo_path)
commits = repo.iter_commits(None, "", after=start_date, before=end_date)
commits = list(filter(lambda x: len(x.parents) == 1, commits))

com_to_scoms = {}

for c in progressbar(commits):
    _, dcs, _ = getstat.GetSingleCommitStat(repo, c)
    if (not dcs['func']): continue
    
    semi_commits = []
    semi_commit = []
    for func in dcs['func']:
        semi_commit += [func]
        if (len(semi_commit) >= SEMI_COMMIT_SIZE):
            semi_commits += [semi_commit]
            semi_commit = []
    if (semi_commit):
        semi_commits += [semi_commit]

    com_to_scoms[c.hexsha] = semi_commits

with open(out_path, 'w') as f:
    json.dump(com_to_scoms, f, indent=2)
