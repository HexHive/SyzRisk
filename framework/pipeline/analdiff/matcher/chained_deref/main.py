#!/usr/bin/python

import re
import json
from common import log


NAME='Chained dereference'
SHORT_NAME='chained_deref'
DESCRIPTION='Find any modified chained dereference.'

fn_with_cd = set()


def OnAnalysisBegin():
    pass

def OnCommitBegin(hexsha, date, msg):
    fn_with_cd.clear()

def OnDiffLine(line, scope_type, scope_name, diff_type):
    if (scope_type == 'func'):
        if (re.search("->\w+->", line)):
            fn_with_cd.add(scope_name)

def OnCommitEnd(hexsha):
    return fn_with_cd

def OnAnalysisEnd(basedir):
    pass
