#!/usr/bin/python

import re
import json
from common import log


NAME='Splitted function'
SHORT_NAME='split_fn'
DESCRIPTION='Find any splitted function.'

fn_having_new_start = ''
splitted_fn = set()


def OnAnalysisBegin():
    pass

def OnCommitBegin(hexsha, date, msg):
    global fn_having_new_start
    fn_having_new_start = '' 
    splitted_fn.clear()

def OnDiffLine(line, scope_type, scope_name, diff_type):
    global fn_having_new_start
    if (scope_type == 'func'):
        if (diff_type == '+'):
            if (line[0:1] == '{'):
                fn_having_new_start = scope_name
            elif (line[0:1] == '}'):
                fn_having_new_start = ''

        if (fn_having_new_start and fn_having_new_start != scope_name):
            splitted_fn.add(fn_having_new_start)

def OnCommitEnd(hexsha):
    return splitted_fn 

def OnAnalysisEnd(basedir):
    pass
