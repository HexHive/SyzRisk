#!/usr/bin/python

import re
import json
from datetime import datetime
from common import log


NAME='Elapsed Date Collector'
SHORT_NAME='elapsed_date'
DESCRIPTION='Collect elapsed day to the latest modification.'

fn_to_edate = {}
fn_to_churn = {}
funcs = set()
ref_date = datetime.strptime("2021-01-01", "%Y-%m-%d")
edays = 0
fix_hexshas = set()


def OnAnalysisBegin():
    with open('/home/gwangmu/Projects/Regression/data/rc/linux-rc-info.json', 'r') as f:
        rcinfo = json.load(f)

    for ri in rcinfo:
        fix_hexshas.add(ri['current_commit'])
    for ri in rcinfo:
        if (ri['fixed_commit'] in fix_hexshas):
            fix_hexshas.remove(ri['fixed_commit'])

def OnCommitBegin(hexsha, date, msg):
    global edays
    global is_fix_commit 
    mod_time = datetime.strptime(date, "%Y-%m-%d %H:%M:%S")
    edays = (ref_date - mod_time).days
    funcs.clear()
    is_fix_commit = (hexsha in fix_hexshas) 

def OnDiffLine(line, scope_type, scope_name, diff_type):
    if (is_fix_commit): return
    if (scope_type == 'func'):
        if (scope_name not in funcs):
            funcs.add(scope_name)
            if (scope_name in fn_to_edate.keys()):
                fn_to_churn[scope_name] += 1
                fn_to_edate[scope_name] = min(fn_to_edate[scope_name], edays)
            else:
                fn_to_churn[scope_name] = 1
                fn_to_edate[scope_name] = edays

def OnCommitEnd(hexsha):
    return set() 

def OnAnalysisEnd(basedir):
    _fn_to_edate = {}
    for fn, edate in fn_to_edate.items():
        _fn_to_edate[fn] = str(edate)

    with open(basedir + '/elapsed_date.json', 'w') as f:
        json.dump(_fn_to_edate, f, indent=2)
        
    _fn_to_churn = {}
    for fn, churn in fn_to_churn.items():
        _fn_to_churn[fn] = str(churn)

    with open(basedir + '/churn.json', 'w') as f:
        json.dump(_fn_to_churn, f, indent=2)
