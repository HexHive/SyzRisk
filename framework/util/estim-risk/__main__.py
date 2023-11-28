#!/usr/bin/python

import os, sys, re
import json, csv
import argparse
import math
import git
from progressbar import progressbar
from datetime import datetime
from common import log
from common import patutil
from common import funcdb


parser = argparse.ArgumentParser()
parser.add_argument('repo_path', type=str)
parser.add_argument('pat_path', type=str)
parser.add_argument('rc_path', type=str)
parser.add_argument('out_dir', type=str)
parser.add_argument('--after', type=str)
parser.add_argument('--before', type=str)
args = parser.parse_args()


SC_RATIO_OUT_PATH = '{}/sc-risk.csv'.format(args.out_dir)
SC_RATIO_JSON_OUT_PATH = '{}/sc-risk.json'.format(args.out_dir)
SS_RATIO_OUT_PATH = '{}/ss-risk.csv'.format(args.out_dir)
SS_RATIO_JSON_OUT_PATH = '{}/ss-risk.json'.format(args.out_dir)

RATIO_MAX = 10
RATIO_MIN = 0.5 
SIZE_CLASS_MAX = 8
SIZE_CLASS_STEP = 2 
GENERAL_MAX = 3

list_def_pats = [ 'concurr_api', 'locked_ctx', 'entering_goto', 'finalization',
        'inside_goto', 'mm_api', 'ptr_arith', 'ptr_api', 'ptr_off',
        'global_var', 'initialization', 'new_state', 'struct_mod',
        'chained_deref', 'ptr_promote', 'split_fn', 'ext_stmt', 'swvar_mod',
        'struct_cast', 'export_fn', 'inside_switch', 'asm_mod', 'changing_err']


# Collect all RCs of interest and their bug types.
log.INFO("collecting RCs and bug types...")

with open(args.rc_path, 'r') as f:
    rc_infos = json.load(f)
    
after_date = datetime.strptime(args.after.split()[0], "%Y-%m-%d")
before_date = datetime.strptime(args.before.split()[0], "%Y-%m-%d")

rcsha_to_bugts = {}
for rci in rc_infos:
    rci_cur_date = datetime.strptime(rci['current_date'].split()[0], "%Y-%m-%d")
    rci_fxd_date = datetime.strptime(rci['fixed_date'].split()[0], "%Y-%m-%d")
    if (rci_cur_date > before_date or rci_fxd_date < after_date):
        continue
    if (rci['fixed_commit'] not in rcsha_to_bugts):
        rcsha_to_bugts[rci['fixed_commit']] = set()
    rcsha_to_bugts[rci['fixed_commit']].update(set(rci['vuln_type']))

for sha in rcsha_to_bugts.keys():
    rcsha_to_bugts[sha] = list(rcsha_to_bugts[sha])


# Collect pattern stats.
log.INFO("collecting pattern stats...")

bin_sc_rc = { p: { c: 0 for c in range(0, SIZE_CLASS_MAX+1) } for p in list_def_pats }
bin_sc_rc_num = { c: 0 for c in range(0, SIZE_CLASS_MAX+1) }
bin_sc_be = { p: { c: 0 for c in range(0, SIZE_CLASS_MAX+1) } for p in list_def_pats }
bin_sc_be_num = { c: 0 for c in range(0, SIZE_CLASS_MAX+1) }

bin_ss_rc = { p: { '(all)': 0 } for p in list_def_pats }
bin_ss_rc_num = { '(all)': 0 }
bin_ss_be = { p: { '(all)': 0 } for p in list_def_pats }
bin_ss_be_num = { '(all)': 0 }
all_subsyses = set(['(all)'])

repo = git.Repo(args.repo_path)
commits = repo.iter_commits(None, "", after=args.after, before=args.before)
commits = list(filter(lambda x: len(x.parents) == 1, commits))

for c in progressbar(commits):
    # Find commit functions and subsystems. 
    funcfile = funcdb.TryGetFuncFileMap(c.hexsha)
    if (not funcfile): continue

    com_funcs = list(funcfile.keys())
    size_class = min(patutil.GetSizeClass(len(com_funcs)), SIZE_CLASS_MAX)
    com_subsyses = patutil.GetSubsysesFromFilepaths(funcfile.values())

    for ss in com_subsyses:
        if (ss not in bin_ss_be_num.keys() or ss not in bin_ss_rc_num.keys()):
            bin_ss_be_num[ss] = 0
            bin_ss_rc_num[ss] = 0
            for pat in list_def_pats:
                bin_ss_be[pat][ss] = 0
                bin_ss_rc[pat][ss] = 0
            all_subsyses.add(ss)

    # Find matched patters.
    pats = [] 
    for func in com_funcs:
        pats += patutil.GetMatchedPatterns(args.pat_path, c.hexsha, func)
    pats = list(set(pats))

    # Collect raw size-class stats.
    if (c.hexsha not in rcsha_to_bugts.keys()):
        bin_sc = bin_sc_be
        bin_sc_num = bin_sc_be_num
    else:
        bin_sc = bin_sc_rc
        bin_sc_num = bin_sc_rc_num

    bin_sc_num[size_class] += 1
    if (size_class <= GENERAL_MAX):
        bin_sc_num[0] += 1
    for pat in pats:
        bin_sc[pat][size_class] += 1
        if (size_class <= GENERAL_MAX):
            bin_sc[pat][0] += 1

    # Collect raw subsystem stats.
    if (size_class >= GENERAL_MAX):
        if (c.hexsha not in rcsha_to_bugts.keys()):
            bin_ss = bin_ss_be
            bin_ss_num = bin_ss_be_num
        else:
            bin_ss = bin_ss_rc
            bin_ss_num = bin_ss_rc_num

        bin_ss_num['(all)'] += 1
        for ss in com_subsyses:
            bin_ss_num[ss] += 1
        for pat in pats:
            bin_ss[pat]['(all)'] += 1
            for ss in com_subsyses:
                bin_ss[pat][ss] += 1


# Estimate size-class risk.
log.INFO("estimating size-class risk...")

bin_sc_ratio = { p: { c: 0 for c in range(0, SIZE_CLASS_MAX+1) } for p in list_def_pats }

for pat in list_def_pats:
    for size_class in range(0, SIZE_CLASS_MAX+1):
        if (not bin_sc_rc[pat][size_class]):
            bin_sc_ratio[pat][size_class] = 1 
        elif (not bin_sc_be[pat][size_class]):
            bin_sc_ratio[pat][size_class] = RATIO_MAX
        else:
            rc_ratio = bin_sc_rc[pat][size_class] / bin_sc_rc_num[size_class]
            be_ratio = bin_sc_be[pat][size_class] / bin_sc_be_num[size_class]
            bin_sc_ratio[pat][size_class] = max(RATIO_MIN, min(RATIO_MAX, rc_ratio / be_ratio))

with open(SC_RATIO_OUT_PATH, 'w') as f:
    header = ['PATTERN'] + list(range(0, SIZE_CLASS_MAX+1))
    w = csv.DictWriter(f, header)
    w.writerow({ x: x for x in header })
    for pat, list_sc in bin_sc_ratio.items():
        row = { 'PATTERN': pat }
        row.update(list_sc)
        w.writerow(row)

with open(SC_RATIO_JSON_OUT_PATH, 'w') as f:
    json.dump(bin_sc_ratio, f, indent=2)


# Estimate subsystem risk.
log.INFO("estimating subsystem risk...")

bin_ss_ratio = { p: { s: 0 for s in all_subsyses } for p in list_def_pats }

for pat in list_def_pats:
    for ss in all_subsyses:
        if (not bin_ss_rc[pat][ss]):
            bin_ss_ratio[pat][ss] = 1 
        elif (not bin_ss_be[pat][ss]):
            bin_ss_ratio[pat][ss] = RATIO_MAX
        else:
            rc_ratio = bin_ss_rc[pat][ss] / bin_ss_rc_num[ss]
            be_ratio = bin_ss_be[pat][ss] / bin_ss_be_num[ss]
            bin_ss_ratio[pat][ss] = max(RATIO_MIN, min(RATIO_MAX, rc_ratio / be_ratio))

with open(SS_RATIO_OUT_PATH, 'w') as f:
    header = ['PATTERN'] + list(all_subsyses)
    w = csv.DictWriter(f, header)
    w.writerow({ x: x for x in header })
    for pat, list_sc in bin_ss_ratio.items():
        row = { 'PATTERN': pat }
        row.update(list_sc)
        w.writerow(row)

with open(SS_RATIO_JSON_OUT_PATH, 'w') as f:
    json.dump(bin_ss_ratio, f, indent=2)
