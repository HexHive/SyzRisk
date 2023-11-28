#!/usr/bin/python

import re
import json
from common import log


NAME='Struct casting'
SHORT_NAME='struct_cast'
DESCRIPTION='Find any struct-involving casting.'

fn_using_sc = set()


def OnAnalysisBegin():
    pass

def OnCommitBegin(hexsha, date, msg):
    fn_using_sc.clear()

def OnDiffLine(line, scope_type, scope_name, diff_type):
    if (scope_type == 'func'):
        if (re.search("[^\w]container_of\(", line) or 
            re.search("(struct|union) \w+(\*|[\s\*]+\*|\*[\s\*]+)\w+ = \w+\([^,]+\);", line) or
            re.search("\(.*(struct|union) \w+ \*\)", line)): 
            fn_using_sc.add(scope_name)

def OnCommitEnd(hexsha):
    return fn_using_sc

def OnAnalysisEnd(basedir):
    pass
