#!/usr/bin/python

import os, sys, re
import math
import git
from datetime import datetime
from common import log
from common import funcdb 
from progressbar import progressbar


def PrintWeights(repo_path, risk_dir, TimeTrans, end_date):
    # NOTE: the interval of commits follows that of 'risk_dir'.
    with open(risk_dir + '/start_date', 'r') as f:
        start_date = f.readlines()[0].strip()
    log.INFO("start_date: " + start_date)
    if (not end_date):
        with open(risk_dir + '/end_date', 'r') as f:
            end_date = f.readlines()[0].strip()
    log.INFO("end_date: " + end_date)
    end_date_p = datetime.strptime(end_date, "%Y-%m-%d")

    repo = git.Repo(repo_path)
    commits = repo.iter_commits(None, "", after=start_date, before=end_date)
    commits = list(filter(lambda x: len(x.parents) == 1, commits))

    def JoinFileFunc(fpath, func):
        return fpath + ':' + func

    log.INFO("calculating churn...")
    file_func_to_churn = dict()
    for c in progressbar(commits):
        funcfiles = funcdb.TryGetFuncFileMap(c.hexsha)
        if (not funcfiles): continue
        for func, fpath in funcfiles.items():
            file_func = JoinFileFunc(fpath, func)
            if (file_func in file_func_to_churn.keys()):
                file_func_to_churn[file_func] += 1
            else:
                file_func_to_churn[file_func] = 1

    log.INFO("calculating weights...")
    file_func_to_weight = dict()
    for c in progressbar(commits):
        funcfiles = funcdb.TryGetFuncFileMap(c.hexsha)
        if (not funcfiles): continue
        _commit_date_p = c.committed_datetime.replace(tzinfo=None) 
        elapsed_time = (end_date_p - _commit_date_p).days
        for func, fpath in funcfiles.items():
            file_func = JoinFileFunc(fpath, func) 
            # NOTE: log(c+1), because log(1)=0 doesn't make sense.
            weight = math.log(file_func_to_churn[file_func]+1)
            weight = TimeTrans(weight, elapsed_time)
            if (weight != 0):
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
