#!/usr/bin/python

import re
import json
from common import log


NAME='Extract modified lines'
SHORT_NAME='ext_modlines'
DESCRIPTION='Collect modified lines.'

commit_to_modlines = {}
current_hexsha = ""


def OnAnalysisBegin():
    pass

def OnCommitBegin(hexsha, date, msg):
    global current_hexsha 
    current_hexsha = hexsha
    commit_to_modlines[hexsha] = []

def OnDiffLine(line, scope_type, scope_name, diff_type):
    # NOTE: only modifications in .c/.h files will be considered.
    # NOTE: <scope_type> is "func", if this modification is inside a function.
    commit_to_modlines[current_hexsha] += \
            [{ 'scope_type': scope_type, 'scope_name': scope_name,
               'diff_type': diff_type, 'line': line }]

def OnCommitEnd(hexsha):
    return set() 

def OnAnalysisEnd(basedir):
    with open(basedir + '/modlines.json', 'w') as f:
        json.dump(commit_to_modlines, f, indent=2)
