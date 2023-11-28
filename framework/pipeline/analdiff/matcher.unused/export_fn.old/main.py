#!/usr/bin/python

import re
import json
from common import log


NAME='Exported function'
SHORT_NAME='export_fn'
DESCRIPTION='Find any exported function.'

old_static_fns = set()
exported_fns = set()


def OnAnalysisBegin():
    pass

def OnCommitBegin(hexsha, msg):
    new_stmts.clear()
    old_stmts.clear()

def OnDiffLine(line, scope_type, scope_name, diff_type):
    if (diff_type == '-'):
        if (line[0:1] == ' '): continue
        static_re = re.search("static .*[ \*](\w+)\(", line)
        if (static_re):
            old_static_fns.add(static_re.group(1))
    elif (diff_type == '+'):
        if (line[0:1] == ' '): continue
        global_re = re.search("(?!static).*[ \*](\w+)\(", line)
        if (global_re and global_re.group(1) in old_static_fn):
            exported_fns.add(global_re.group(1))

def OnCommitEnd(hexsha):
    return exported_fns

def OnAnalysisEnd(basedir):
    pass
