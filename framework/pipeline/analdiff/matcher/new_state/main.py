#!/usr/bin/python

import re
import json
from common import log


NAME='New state'
SHORT_NAME='new_state'
DESCRIPTION='Find any new state (i.e., enum field or define) and its usage.'

new_states = set()
conds = set()


def OnAnalysisBegin():
    pass

def OnCommitBegin(hexsha, date, msg):
    new_states.clear()
    conds.clear()

def OnDiffLine(line, scope_type, scope_name, diff_type):
    def_re = re.search("#define\s+(\w+)", line)
    if (def_re):
        new_states.add(def_re.group(1))

    if (scope_type == 'enum'):
        def_re = re.search("(\w+).*,", line)
        if (def_re):
            new_states.add(def_re.group(1))

    if (scope_type == 'func'):
        if (re.search("[^\w](if|case)[^\w]", line)):
            conds.add((line, scope_name))
        elif (re.search("\w+ = [A-Z0-9_]+", line)):
            conds.add((line, scope_name))

def OnCommitEnd(hexsha):
    fn_using_ns = set()
    for (line, scope_name) in conds:
        for ns in new_states:
            if (ns in line):
                fn_using_ns.add(scope_name)
                break
    return fn_using_ns

def OnAnalysisEnd(basedir):
    pass
