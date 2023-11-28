#!/usr/bin/python

import re
import json
from common import log


NAME='Exported function'
SHORT_NAME='export_fn'
DESCRIPTION='Find any new exports of existing functions.'

old_static_fns = set()
new_global_fns = set()
new_decl_fns = set()

def OnAnalysisBegin():
    pass

def OnCommitBegin(hexsha, date, msg):
    old_static_fns.clear()
    new_global_fns.clear()
    new_decl_fns.clear()

def OnDiffLine(line, scope_type, scope_name, diff_type):
    if (scope_type == 'func'):
        proto_re = re.search("(static)?[\w\*\s]+[\*\s]\w+\(", line)
        if (proto_re):
            if (diff_type == '-' and proto_re.group(1)):
                old_static_fns.add(scope_name)
            elif (diff_type == '+' and not proto_re.group(1)):
                new_global_fns.add(scope_name)
    else:
        if (diff_type == '+'):
            decl_re = re.search("[\w\*\s]+[\*\s](\w+)\(", line)
            if (decl_re):
                new_decl_fns.add(decl_re.group(1))

def OnCommitEnd(hexsha):
    new_fns = new_global_fns - old_static_fns
    export_fns = new_decl_fns - new_fns
    return export_fns

def OnAnalysisEnd(basedir):
    pass
