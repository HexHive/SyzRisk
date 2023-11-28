#!/usr/bin/python

import re
import json
from common import log


NAME='Pointer promotion'
SHORT_NAME='ptr_promote'
DESCRIPTION='Find any struct variable promoted to a pointer.'

new_ptr_var = set()
fn_with_pp = set()


def OnAnalysisBegin():
    pass

def OnCommitBegin(hexsha, date, msg):
    new_ptr_var.clear()
    fn_with_pp.clear()

def OnDiffLine(line, scope_type, scope_name, diff_type):
    if (scope_type == 'func'):
        padded_line = ' '*5 + line + ' '*5
        if (diff_type == '-'):
            scala_re = re.search("(.{5})(\w+)\.(.{5})", padded_line)
            if (scala_re):
                #print("scala: " + scala_re.group(2))
                new_ptr_var.add(scala_re.group(1) + scala_re.group(2) + '->' + scala_re.group(3))
        elif (diff_type == '+'):
            ptr_re = re.search("(.{5}(\w+)->.{5})", padded_line)
            if (ptr_re and ptr_re.group(1) in new_ptr_var):
                #print("new_ptr: " + ptr_re.group(2))
                fn_with_pp.add(scope_name)

def OnCommitEnd(hexsha):
    return fn_with_pp

def OnAnalysisEnd(basedir):
    pass
