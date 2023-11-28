#!/usr/bin/python

import re
import json
from common import log
from difflib import SequenceMatcher


NAME='Extracted statement'
SHORT_NAME='ext_stmt'
DESCRIPTION='Find any extracted statement.'

total_funcs = set()
new_stmts = {}
old_stmts = {}


def OnAnalysisBegin():
    pass

def OnCommitBegin(hexsha, date, msg):
    total_funcs.clear()
    new_stmts.clear()
    old_stmts.clear()

def OnDiffLine(line, scope_type, scope_name, diff_type):
    if (scope_type == 'func'):
        total_funcs.add(scope_name)
        line_strip = line.strip()
        if (not line_strip): return
        line_strip = line_strip.replace('&', '')
        line_strip = line_strip.replace('->', '.')
        line_strip = line_strip.replace('*', '')
        if (diff_type == '+'):
            if (scope_name not in new_stmts.keys()):
                new_stmts[scope_name] = set()
            new_stmts[scope_name].add(line_strip)
        elif (diff_type == '-'):
            if (scope_name not in old_stmts.keys()):
                old_stmts[scope_name] = set()
            old_stmts[scope_name].add(line_strip)

def OnCommitEnd(hexsha):
    # FIXME: temporary measurement to avoid quadratic growth of complexity.
    # Needs a better measurement than cutting of up to 16-func commits.
    if (len(total_funcs) > 16): return set()

    fn_with_es = set()
    for ofunc, ostmts in old_stmts.items():
        for nfunc, nstmts in new_stmts.items():
            # FIXME: temporary measurement to avoid quadratic growth of complexity.
            # Needs a better measurement than cutting of up to 16-func commits.
            if (len(ostmts) > 32 or len(nstmts) > 32): continue

            # TODO: commenting-out same-function restriction.
            #       does it work for intra-functional moving?
            if (ofunc == nfunc): continue
            #print(ofunc + " <=> " + nfunc)
            matched_ostmts = set()
            matched_nstmts = set()
            # Match old stmts to new ones.
            for ostmt in ostmts:
                for nstmt in nstmts:
                    sim_ratio = SequenceMatcher(None, ostmt, nstmt).ratio()
                    if (sim_ratio > 0.98):
                        #print("  " + ostmt + " <=> " + nstmt + " ({}%)".format(sim_ratio * 100))
                        matched_ostmts.add(ostmt)
                        matched_nstmts.add(nstmt)
                        break
            sim_old = len(matched_ostmts) / len(ostmts)
            sim_new = len(matched_nstmts) / len(nstmts)
            #print(" " + ofunc + " <=> " + nfunc + " ({}%, {}%)".format(sim_old * 100, sim_new * 100))
            if (sim_old > 0.8 or sim_new > 0.8):
                #print(" !!! ")
                fn_with_es.add(ofunc)
                fn_with_es.add(nfunc)
    return fn_with_es

def OnAnalysisEnd(basedir):
    pass
