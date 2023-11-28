#!/usr/bin/python

import re
import json
from common import log


NAME='Synchronization operation'
SHORT_NAME='synch_op'
DESCRIPTION='Find any synchronization operations.'


matched_commits = set()
matched_funcs_cu = set()
matched_funcs_fu = set()

encountered_funcs = set()
current_hexsha = ''
current_commit_taken = False


def OnAnalysisBegin():
    pass

def OnCommitBegin(hexsha, msg):
    global current_hexsha
    global current_commit_taken
    current_hexsha = hexsha 
    current_commit_taken = False
    encountered_funcs.clear()

def OnDiffLine(line, scope_type, scope_name, diff_type):
    if (scope_type == 'func'):
        encountered_funcs.add(scope_name)
        synch_re = re.search('(lock|wake|wait|concurrent|sleep)\w*\(', line)
        if (synch_re):
            log.SPECIAL(SHORT_NAME + ": found synch operation: {}".format(line))
            global current_commit_taken
            matched_funcs_fu.add(scope_name)
            current_commit_taken = True

def OnCommitEnd(hexsha):
    if (current_commit_taken):
        matched_commits.add(current_hexsha)
        matched_funcs_cu.update(encountered_funcs)

def OnAnalysisEnd(basedir):
    log.SPECIAL(SHORT_NAME + ": matched {} commits.".format(len(matched_commits)))
    log.SPECIAL(SHORT_NAME + ":  - including {} functions.".format(len(matched_funcs_cu)))
    log.SPECIAL(SHORT_NAME + ": matched {} functions.".format(len(matched_funcs_fu)))

    with open(basedir + '/matched_commits.json', 'w') as f:
        json.dump(list(matched_commits), f, indent=2)

    with open(basedir + '/matched_funcs_fu.json', 'w') as f:
        json.dump(list(matched_funcs_fu), f, indent=2)

    with open(basedir + '/matched_funcs_cu.json', 'w') as f:
        json.dump(list(matched_funcs_cu), f, indent=2)

