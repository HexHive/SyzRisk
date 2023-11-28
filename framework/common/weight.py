#!/usr/bin/python

import os, sys, re
import json
import math
import git
from common import funcdb
from common import patutil

class PatternWeightCalculator:
    def __init__(self, pat_path, risk_dir, repo=None):
        self.pat_path = pat_path
        self.repo = repo
        with open(risk_dir + '/sc-risk.json', 'r') as f:
            self.sc_factor = json.load(f)
        with open(risk_dir + '/ss-risk.json', 'r') as f:
            self.ss_factor = json.load(f)

    ### If you want to calculate weights manually...

    def GetWeightRaw(self, pats, nr_com_funcs, subsyses):
        risk = 1
        for pat in pats:
            size_class = patutil.GetSizeClass(nr_com_funcs)
            factor_0 = self.sc_factor[pat]["0"]
            factor = self.sc_factor[pat][str(size_class)]
            factor_ss = max([1] + 
                    [self.ss_factor[pat][ss] for ss in subsyses \
                            if ss in self.ss_factor[pat].keys()])
            risk *= max(1, factor_0, factor, factor_ss) 
        return max(1, math.floor((-math.exp(-risk/256)+1)*256))
        # FIXME: why floor????
        #    # TODO: 2.11 is the average general liklihood (at least it's
        #    # supposed to be). Fix it to reflect the actual average data.
        #    risk += math.log(max(0, factor_0, factor, factor_ss), 2.11) ** 3 
        #return min(800, max(0, math.floor(risk*10)))

    ### Below only works if "repo" was provided.

    def SetCommit(self, commit):
        self._commit = commit
        funcfile = funcdb.TryGetFuncFileMap(commit.hexsha)
        if (funcfile):
            self._subsyses = patutil.GetSubsysesFromFilepaths(funcfile.values())
            self._nr_com_funcs = len(funcfile.keys())
            return funcfile
        else:
            return {} 

    def GetWeight(self, funcname):
        pats = patutil.GetMatchedPatterns(self.pat_path, self._commit.hexsha, funcname)
        return self.GetWeightRaw(pats, self._nr_com_funcs, self._subsyses)
