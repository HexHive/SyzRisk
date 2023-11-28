#!/usr/bin/python

import re
import json
from common import log


NAME='Modified assembly'
SHORT_NAME='asm_mod'
DESCRIPTION='Find any modified assembly.'

mod_fns = set()
select_all_fns = False
asm_fns = set()


def OnAnalysisBegin():
    pass

def OnCommitBegin(hexsha, date, msg):
    global select_all_fns
    mod_fns.clear()
    select_all_fns = False
    asm_fns.clear()

def OnDiffLine(line, scope_type, scope_name, diff_type):
    global select_all_fns

    if (scope_type == 'func'):
        mod_fns.add(scope_name)

    inasm_re = re.search("__asm__", line)
    macro_re= re.search(".macro", line)
    rawasm_re = re.search("^\s*\w+\s+\w+,\s*\w+\s*$", line)
    if (inasm_re or macro_re or rawasm_re):
        if (rawasm_re): print(rawasm_re.group(0))
        if (scope_type == 'func'):
            asm_fns.add(scope_name)
        else:
            select_all_fns = True

def OnCommitEnd(hexsha):
    if (select_all_fns):
        return mod_fns
    else:
        return asm_fns

def OnAnalysisEnd(basedir):
    pass
