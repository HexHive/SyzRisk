#!/usr/bin/python

import re
import json
from common import log


NAME='Structure modification'
SHORT_NAME='struct_mod'
DESCRIPTION='Find any access of modified structure fields.'


mod_fields = set()
acc_fields = set()
encountered_funcs = set()


def OnAnalysisBegin():
    pass

def OnCommitBegin(hexsha, date, msg):
    mod_fields.clear()
    acc_fields.clear() 
    encountered_funcs.clear()

def OnDiffLine(line, scope_type, scope_name, diff_type):
    if (scope_type == 'struct'):
        decl_re = re.search("(struct |union |enum |unsigned |signed )?[A-Za-z0-9_]+[\*\s]+[\(\*]*([A-Za-z0-9_]+)[\)]*.*;", line)
        if (decl_re):
            mod_fields.add(decl_re.group(2))
    elif (scope_type == 'func'):
        encountered_funcs.add(scope_name)
        fields = re.findall('(?:\.|->)(\w+)(?:[^\w]|$)', line)
        for field in fields:
            acc_fields.add((field, scope_name))

def OnCommitEnd(hexsha):
    fields = set(filter(lambda x: x[0] in mod_fields, acc_fields))
    funcs = set()
    for (_, func) in fields:
        funcs.add(func)
    return funcs

def OnAnalysisEnd(basedir):
    pass
