#!/usr/bin/python

import os, sys, re
import math
import git
from datetime import datetime
from common import patutil
from common import weight as w 
from progressbar import progressbar


def PrintWeights(repo_path, match_dir, risk_dir, TimeTrans, end_date):
    # NOTE: the interval of commits follows that of 'risk_dir'.
    with open(risk_dir + '/start_date', 'r') as f:
        start_date = f.readlines()[0].strip()
    if (not end_date):
        with open(risk_dir + '/end_date', 'r') as f:
            end_date = f.readlines()[0].strip()
    end_date_p = datetime.strptime(end_date, "%Y-%m-%d")

    repo = git.Repo(repo_path)
    commits = repo.iter_commits(None, "", after=start_date, before=end_date)
    commits = list(filter(lambda x: len(x.parents) == 1, commits))

    prc = w.PatternWeightCalculator(match_dir, risk_dir, repo=repo)

    file_func_to_weight = dict()
    for c in progressbar(commits):
        funcfiles = prc.SetCommit(c)
        _commit_date_p = c.committed_datetime.replace(tzinfo=None) 
        elapsed_time = (end_date_p - _commit_date_p).days
        for func, fpath in funcfiles.items():
            weight = prc.GetWeight(func)
            weight = TimeTrans(weight, elapsed_time)  # NOTE: time transformation.
            if (weight != 0):
                file_func = fpath + ':' + func
                if (file_func in file_func_to_weight.keys()):
                    file_func_to_weight[file_func] = \
                            max(file_func_to_weight[file_func], weight)
                else:
                    file_func_to_weight[file_func] = weight

    file_func_weight = []
    for file_func, weight in file_func_to_weight.items():
        fpath, func = file_func.split(':')
        file_func_weight += [{ "file": str(fpath), "func": str(func), "weight": weight }]

    return file_func_weight
