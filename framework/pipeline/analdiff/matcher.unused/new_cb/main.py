#!/usr/bin/python

import re
import json
from common import log


NAME='New callback'
SHORT_NAME='new_cb'
DESCRIPTION='Find any new registration of callback functions.'


possible_cbs = set()
encountered_funcs = set()


def OnAnalysisBegin():
    pass

def OnCommitBegin(hexsha, msg):
    possible_cbs.clear()
    encountered_funcs.clear() 

def OnDiffLine(line, scope_type, scope_name, diff_type):
    if (scope_type == 'func'):
        encountered_funcs.add(scope_name)
    elif (scope_type == 'init'):
        cbreg_re = re.search('([a-zA-Z]\w*),', line)
        if (cbreg_re):
            possible_cbs.add(cbreg_re.group(1))

def OnCommitEnd(hexsha):
    cbs = set(filter(lambda x: x in encountered_funcs, possible_cbs))
    return cbs

def OnAnalysisEnd(basedir):
    pass
