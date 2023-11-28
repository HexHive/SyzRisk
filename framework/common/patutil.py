#!/usr/bin/python

import os, sys, re
import math

SIZE_CLASS_MAX = 8

def GetMatchedPatterns(pat_path, hexsha, funcname):
    matdir_path = '{}/{}/{}'.format(pat_path, hexsha[0:12], funcname)
    if (not os.path.exists(matdir_path)):
        return []
    else:
        pats = []
        for mat_dir in next(os.walk(matdir_path))[1]:
            pats += [mat_dir]
        return pats

def GetSubsysesFromFilepaths(paths):
    res = set()
    for path in paths:
        ss = path.split('/')
        if (ss): 
            if (len(ss) == 1): res.add('(global)')
            elif (ss[0] != 'Documentation'): res.add(ss[0])
    return res

def GetSizeClass(size): 
    return min(SIZE_CLASS_MAX, math.floor(math.log2(size))+1)
