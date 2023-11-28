#!/usr/bin/python

import git
import re
from . import log
from . import cstat


NULL_HASHOBJ = None

def _GetPrevCommit(repo, commit):
    global NULL_HASHOBJ
    if (len(commit.parents) >= 1):
        if (len(commit.parents) > 1):
            log.WARN("multiple parents. selecting first...")
        return commit.parents[0]
    elif (not len(commit.parents)):
        if (not NULL_HASHOBJ):
            NULL_HASHOBJ = repo.git.execute(
                    ['git','hash-object','-t','tree','/dev/null']) 
        return repo.tree(NULL_HASHOBJ)

def _ExtendUntilOpenBracket(diff_lines, rno, ver_char):
    retstr = ""
    roff = 0
    for no in range(rno, len(diff_lines)):
        if (diff_lines[no][0:1] != ' ' and diff_lines[no][0:1] != '+' and
                diff_lines[no][0:1] != '-'):
            break
        if (diff_lines[no][0:1] not in ver_char):
            continue
        retstr += diff_lines[no][1:] + ' '
        roff += 1
        if (retstr.find('{') != -1 or retstr.find(';') != -1):
            break
    return (retstr, roff)

def _GetEnclosingScopeInfo(diff_lines, refno, line_cache):
    # NOTE: temporary measurement; if the function name itself was
    # changed, the new name should be considered. Since this entire
    # script relies on the fact that function prototypes begin at the
    # very beginning of lines, check if the first character is
    # non-space and if it is, only consider '+' lines.
    ver_char = [' ', diff_lines[refno][0:1]]
    if (diff_lines[refno][1:2] != ' '):
        ver_char = [' ', '+']

    for rno in reversed(range(refno+1)):
        if (rno in line_cache):
            # 'name' doesn't matter here.
            # To myself from Aug 3th: NOPE.
            return (line_cache[rno][0], line_cache[rno][1], 1)

        is_header = False
        ref_line = diff_lines[rno]
        roff = 1

        if (ref_line[0:1] == '@'):
            # If it's a header, extract the following comment.
            is_header = True
            header_re = re.search("@@ .* @@ (.*)", ref_line)
            if (header_re):
                ref_line = header_re.group(1)
                # If the following comment is a label, we cannot find out
                # the true function name. Just keep it unknown.
                label_re = re.search("^[A-Za-z0-9_]+:", ref_line)
                if (label_re):
                    return ('func', '(unknown)', roff)
            else:
                return ('(out)', '', roff)
        else:
            # If it's a plain source line, cut out the first character.
            ref_line = ref_line[1:]

        if (ref_line[0:1] == '}'):
            return ('(out)', None, roff)

        if (len(ref_line) == 0 or not re.match('[a-zA-Z_]', ref_line) or 
                ref_line.find(':') != -1):
            continue

        if (not is_header):
            (ref_line, roff) = _ExtendUntilOpenBracket(diff_lines, rno, ver_char)
        if (ref_line.find('{') == -1 and ref_line.find(';') != -1):
            return ('(out)', None, roff)
        log.DEBUG("prototype found: {}{}"
                .format(ref_line, ' [hd]' if is_header else ''))

        func_re = re.search("([a-zA-Z0-9_]+)\s*\((.*\)\s*{)?", ref_line)
        if (func_re and (is_header or func_re.group(2))):
            func_re = re.search("([a-zA-Z0-9_]+)\s*\(", ref_line)
            if (not func_re): 
                log.FATAL("should be matched at this point. wtf?")
            func_name = func_re.group(1)
            return ('func', func_name, roff)

        struct_re = re.search("^(struct|union)\s+([a-zA-Z0-9_]+)(\s*{)?", ref_line)
        if (struct_re and (is_header or struct_re.group(3))):
            struct_name = struct_re.group(2)
            return ('struct', struct_name, roff)

        init_re = re.search("^(\w.*)\s+([a-zA-Z0-9_\[\]]+)\s*=\s*{", ref_line)
        if (init_re):
            prefix = list(filter(lambda x: x.strip() == 'struct' or
                x.strip() == 'union', init_re.group(1).split()))
            if (prefix):
                init_name = init_re.group(2)
                return ('init', init_name, roff)

        if (is_header):
            return ('(out)', '', roff)


def GetSingleCommitStat(repo, commit_this):
    PARSE_FILEPATH  = 1
    PARSE_DIFF      = 2

    commit_old = _GetPrevCommit(repo, commit_this)
    cs = cstat.CommitStat(commit_this.hexsha)
    dcs = { 'func': set(), 'struct': set(), 'init': set(), 'funcfile': dict() } 

    edges = {}

    # Count modified sources.
    diffs = commit_old.diff(commit_this)
    cs.value['src'] = len(diffs)

    # Count everything else.
    def UpdateCommitStat(cs, cache, filepath):
        if (len(cache['func']) > 1):
            cache['func'].discard('(unknown)')
        cs.value['func'] += len(cache['func'])
        cs.value['struct'] += len(cache['struct'])
        cs.value['init'] += len(cache['init'])
        dcs['func'].update(cache['func'])
        dcs['struct'].update(cache['struct'])
        dcs['init'].update(cache['init'])
        for f in cache['func']:
            if (f in dcs['funcfile'].keys() and \
                    dcs['funcfile'][f].split('.')[-1] == '.c'):
                continue
            dcs['funcfile'][f] = filepath
        cache['func'] = set()
        cache['struct'] = set()
        cache['init'] = set()
        cache['funcfile'] = dict()

    diff_lines = repo.git.diff(commit_old, commit_this).split('\n')
    cache = { 'line': dict(), 'func': set(), 'struct': set(), 'init': set(), 'funcfile': dict() }
    state = PARSE_FILEPATH
    cur_filepath = ''
    for lidx in range(len(diff_lines)):
        line = diff_lines[lidx]

        if (state == PARSE_FILEPATH):
            if (line[0:3] == "+++"):
                new_filepath_re = re.search("\+\+\+ b/(.*)", line)
                if (not new_filepath_re):
                    continue

                cur_filepath = new_filepath_re.group(1)
                if (cur_filepath[-2:] != ".c" and cur_filepath[-2:] != ".h"): 
                    #log.INFO("skipping non-C file '" + cur_filepath + "'...")
                    continue

                cache['line'][lidx] = ("(out)", "")

                state = PARSE_DIFF
                #log.INFO("collecting diffs in '" + cur_filepath + "'...")

        elif (state == PARSE_DIFF):
            if (line[0:4] == 'diff'):
                UpdateCommitStat(cs, cache, cur_filepath)
                state = PARSE_FILEPATH

            # Count added/deleted lines.
            if (line[0:1] == '+'):
                cs.value['add'] += 1
            elif (line[0:1] == '-'):
                cs.value['del'] += 1

            (sty, sname, roff) = _GetEnclosingScopeInfo(diff_lines, lidx, cache['line'])

            # Cache enclosing scopes.
            if (line[0:1] == '+' or line[0:1] == '-'):
                for xx in range(lidx, lidx+roff): 
                    cache['line'][xx] = (sty, sname)
                if (sty != '(out)' and sname and sname != '(something)' and
                        sname not in cache[sty]):
                    cache[sty].add(sname)
                    log.DEBUG("new scope found. {} ({}: {})".format(roff, sty, sname))
                if (sty == 'func' and sname and sname != '(something)'):
                    cache['funcfile'][sname] = cur_filepath 

            ### Callgraph generation ###
            if (sty == 'func' and sname != '(something)'):
                if (sname not in edges.keys()):
                    edges[sname] = []
                call_re = re.search('(\w+)\s*\(', line[1:])
                if (call_re):
                    callname = call_re.group(1)
                    if (callname not in [sname, "if", "for", "while", "switch"]):
                        #print('[v] {} -> {}'.format(sname, callname))
                        edges[sname] += [callname]

            _edges = {} 
            for k in edges.keys():
                _edges[k] = list(set((filter(lambda x: x in edges.keys(), edges[k]))))
            edges = _edges
            ### Callgraph generation ###

    """
    ### Grouping ###
    _edges = {} 
    for k in edges.keys():
        _edges[k] = list(set((filter(lambda x: x in edges.keys(), edges[k]))))
    groups = []
    for f, es in _edges.items():
        def FindGroup(f):
            nonlocal groups
            for group in groups:
                if (f in group):
                    return group
            return None
        def AddToGroup(f, group):
            group.add(f)
        def CreateGroup(f):
            nonlocal groups
            ng = set([f])
            groups += [ng]
            return ng
        def MergeGroup(g1, g2):
            nonlocal groups
            g1.update(g2)
            groups.remove(g2)

        cur_group = None
        cur_group = FindGroup(f)
        if (not cur_group):
            cur_group = CreateGroup(f) 
        for e in es:
            tail_group = FindGroup(e)
            if (tail_group != cur_group):
                if (not tail_group):
                    AddToGroup(e, cur_group)
                else:
                    MergeGroup(cur_group, tail_group)
    ### Grouping ###
    """

    UpdateCommitStat(cs, cache, cur_filepath)
    log.DEBUG(cs)
    return (cs, dcs, edges)#, groups)
