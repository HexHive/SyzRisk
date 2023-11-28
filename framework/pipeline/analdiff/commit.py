#!/usr/bin/python

import re, os, sys, git
import numpy as np
import json
from common import log 
from common import extfunc
from common.cstat import *
from types import SimpleNamespace
from datetime import datetime
import shutil
import progressbar


class HistoryStatFinder:
    NULL_HASHOBJ    = None

    def __init__(self, repo_path, start_date='', end_date='', hexshas=[], 
            ignore_merge=False, output_dir='', matcher_list=[],
            superset_path='', ext_path=''):
        self.repo_path = repo_path
        self.repo = git.Repo(repo_path)
        self.ext_path = ext_path
        self.output_dir = output_dir
        
        if (not HistoryStatFinder.NULL_HASHOBJ):
            HistoryStatFinder.NULL_HASHOBJ = \
                    self.repo.git.execute(['git','hash-object','-t','tree','/dev/null']) 

        if (hexshas):
            self.commits = []
            for hexsha in hexshas:
                try:
                    self.commits += [self.repo.commit(hexsha)]
                except Exception as err:
                    log.WARN("cannot find hexsha: {}".format(err))
        else:
            self.commits = list(self.repo.iter_commits(None, "",
                after=start_date, before=end_date))

        if (superset_path):
            try:
                with open(superset_path, 'r') as f:
                    sslist = json.load(f)
                self.commits = list(filter(lambda c: c.hexsha in sslist, self.commits))
            except Exception as err:
                log.FATAL("failed to load superset list: {}".format(err))

        if (ignore_merge):
            self.commits = [ c for c in self.commits if len(c.parents) == 1 ]

        # Import all specified matchers.
        self.mat_mods= dict()

        sys.path.insert(1, os.path.dirname(os.path.abspath(__file__)) + '/matcher')
        for mat_name in matcher_list:
            try:
                self.mat_mods[mat_name] = __import__(mat_name + '.main',
                        fromlist=['matcher'])
            except Exception as err:
                log.FATAL("failed to load matcher '{}': {}".format(mat_name, err))

        # Call 'OnAnalysisBegin' for all matchers.
        for mat in self.mat_mods.values():
            mat.OnAnalysisBegin()

        css = self.AnalyzeCommits(self.commits)

        # Call 'OnAnalysisEnd' for all matchers.
        for mat_name, mat in self.mat_mods.items():
            mat.OnAnalysisEnd(output_dir)

        _ShowHistoryStat(css)

        output_path = output_dir + '/cssp.json'

        try:
            tb = self.repo.active_branch.tracking_branch()
            repo_str = self.repo.remotes[0].url + ':' + \
                    tb.remote_name + '/' + tb.remote_head
        except:
            repo_str = ''
        cssp = CommitStatPack(css, 
                repo=repo_str,
                repo_local=self.repo.working_dir,
                ignore_merge=ignore_merge, hexshas=bool(hexshas),
                start_date=start_date if not hexshas else '', 
                end_date=end_date if not hexshas else '',
                superset_path=superset_path)
        saved = self.SaveCommitStats(cssp, output_path)
        if (saved):
            log.INFO('saved commit stats to \'{}\'.'.format(output_path))
        else:
            log.WARN('failed to save commit stats to \'{}\'.'
                    .format(output_path))

    def SaveCommitStats(self, cssp, output_path):
        with open(output_path, 'w') as f:
            json.dump(cssp, f, indent=2, default=lambda o: o.__dict__)
        return True

    def AnalyzeCommits(self, commits):
        css = []
        if (self.ext_path):
            os.makedirs(self.ext_path, exist_ok=True)
        for ci in progressbar.progressbar(range(len(commits)), redirect_stdout=True):
            # NOTE: in case this script has crashed while doing some
            # time-consuming jobs. un-comment them.
            #if (os.path.exists(self.ext_path + '/' + commits[ci].hexsha[0:12])):
            #    continue
            log.INFO("analyzing commit '" + commits[ci].hexsha + \
                    "' ({}/{})...".format(ci+1, len(commits)))
            css += [self.AnalyzeSingleCommit(commits[ci])]
        return css

    def AnalyzeSingleCommit(self, commit_this):
        PARSE_FILEPATH  = 1
        PARSE_DIFF      = 2

        # Call 'OnCommitBegin' for all matchers.
        for mat in self.mat_mods.values():
            mat.OnCommitBegin(commit_this.hexsha,
                    str(datetime.utcfromtimestamp(commit_this.committed_date)), 
                    commit_this.message)

        commit_old = self.GetPrevCommit(commit_this)
        cs = CommitStat(commit_this.hexsha)

        # Extract functions if necessary.
        if (self.ext_path):
            ext_commit_dir = self.ext_path + '/' + commit_this.hexsha[0:12]
            log.INFO("extracting functions from '" + commit_this.hexsha + "'...")
            extfunc.ExtractFunc(self.repo, ext_commit_dir,
                    reuse_temp_dirs=True, old_commit=commit_old,
                    new_commit=commit_this)

        # Count modified sources.
        diffs = commit_old.diff(commit_this)
        cs.value['src'] = len(diffs)

        # Count everything else.
        def UpdateCommitStat(cs, cache):
            if (len(cache['func']) > 1):
                cache['func'].discard('(unknown)')
            cs.value['func'] += len(cache['func'])
            cs.value['struct'] += len(cache['struct'])
            cs.value['init'] += len(cache['init'])
            cs.value['enum'] += len(cache['enum'])
            cache['func'] = set()
            cache['struct'] = set()
            cache['init'] = set()
            cache['enum'] = set()

        diff_lines = self.repo.git.diff(commit_old, commit_this).split('\n')
        cache = { 'line': dict(), 'func': set(), 'struct': set(), 'init': set(), 'enum': set() }
        state = PARSE_FILEPATH
        for lidx in range(len(diff_lines)):
            line = diff_lines[lidx]

            if (state == PARSE_FILEPATH):
                if (line[0:3] == "+++"):
                    new_filepath_re = re.search("\+\+\+ b/(.*)", line)
                    if (not new_filepath_re):
                        continue

                    cur_filepath = new_filepath_re.group(1)
                    if (cur_filepath[-2:] != ".c" and cur_filepath[-2:] != ".h"): 
                        log.INFO("skipping non-C file '" + cur_filepath + "'...")
                        continue

                    cache['line'][lidx] = (None, None)

                    state = PARSE_DIFF
                    log.INFO("collecting diffs in '" + cur_filepath + "'...")

            elif (state == PARSE_DIFF):
                if (line[0:4] == 'diff'):
                    UpdateCommitStat(cs, cache)
                    state = PARSE_FILEPATH

                # Count added/deleted lines.
                if (line[0:1] == '+'):
                    cs.value['add'] += 1
                elif (line[0:1] == '-'):
                    cs.value['del'] += 1

                (sty, sname, roff) = self.GetEnclosingScopeInfo(diff_lines, lidx, cache['line'])

                # Cache enclosing scopes.
                if (line[0:1] == '+' or line[0:1] == '-'):
                    for xx in range(lidx, lidx+roff): 
                        cache['line'][xx] = (sty, sname)

                    # Call 'OnDiffLine' for all matchers.
                    for mat in self.mat_mods.values():
                        mat.OnDiffLine(line[1:], sty, sname, line[0:1])

                    if (sty and sname and sname not in cache[sty]):
                        cache[sty].add(sname)
                        log.DEBUG("new scope found. {} ({}: {})".format(roff, sty, sname))

        UpdateCommitStat(cs, cache)
        log.DEBUG(cs)

        # Call 'OnCommitEnd' for all matchers.
        for mat in self.mat_mods.values():
            fnset = mat.OnCommitEnd(commit_this.hexsha)
            if (fnset):
                commit_dir = self.output_dir + '/matcher/' + commit_this.hexsha[0:12]
                for fn in fnset:
                    os.makedirs(commit_dir + '/' + fn + '/' + mat.SHORT_NAME, exist_ok=True)

        return cs

    def ExtendUntilOpenBracket(self, diff_lines, rno, ver_char):
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

    def GetEnclosingScopeInfo(self, diff_lines, refno, line_cache):
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
                (ref_line, roff) = self.ExtendUntilOpenBracket(diff_lines, rno, ver_char)
            if (ref_line.find(';') != -1):
                return ('(out)', None, roff)
            log.DEBUG("prototype found: {}{}"
                    .format(ref_line, ' [hd]' if is_header else ''))

            # FIX: some function prototypes separate types and
            # function names in different lines. we just relax the
            # regex to recognize a function prototype without a type.
            #func_re = re.search("^\w.*[\s\*]+([a-zA-Z0-9_]+)\((.*\)\s*{)?", ref_line)
            func_re = re.search("[\w\*]+([a-zA-Z0-9_]+)\((.*\)\s*{)?", ref_line)
            if (func_re and (is_header or func_re.group(2))):
                func_re = re.search("([a-zA-Z0-9_]+)\(", ref_line)
                if (not func_re): 
                    log.FATAL("should be matched at this point. wtf?")
                func_name = func_re.group(1)
                return ('func', func_name, roff)

            struct_re = re.search("^(typedef\s+)?(struct|union)\s+([a-zA-Z0-9_]+)(\s*{)?", ref_line)
            if (struct_re and (is_header or struct_re.group(4))):
                struct_name = struct_re.group(3)
                return ('struct', struct_name, roff)

            enum_re = re.search("^(typedef\s+)?(enum)\s+([a-zA-Z0-9_]+)(\s*{)?", ref_line)
            if (enum_re and (is_header or enum_re.group(4))):
                enum_name = enum_re.group(3)
                return ('enum', enum_name, roff)

            init_re = re.search("^(\w.*)\s+([a-zA-Z0-9_\[\]]+)\s*=\s*{", ref_line)
            if (init_re):
                prefix = list(filter(lambda x: x.strip() == 'struct' or
                    x.strip() == 'union', init_re.group(1).split()))
                if (prefix):
                    init_name = init_re.group(2)
                    return ('init', init_name, roff)

            if (is_header):
                return ('(out)', '', roff)

    def GetPrevCommit(self, commit):
        if (len(commit.parents) >= 1):
            if (len(commit.parents) > 1):
                log.WARN("multiple parents. selecting first...")
            return commit.parents[0]
        elif (not len(commit.parents)):
            return self.repo.tree(HistoryStatFinder.NULL_HASHOBJ)


class HistoryStat:
    def __init__(self, css):
        self.is_inited = False
        if (not css): return

        self.stat = { 'avg':  { k: None for k in CommitStat().value.keys() },
                      'quar': { k: None for k in CommitStat().value.keys() },
                      'sum':  { k: None for k in CommitStat().value.keys() } } 

        for k in CommitStat().value.keys():
            sorted_css = sorted(css, key = lambda x: x.value[k])
            sorted_shas = [cs.commit for cs in sorted_css]
            sorted_vals = [cs.value[k] for cs in sorted_css]
            self.stat['avg'][k] = StatAverage(round(np.mean(sorted_vals), 2),
                                              round(np.std(sorted_vals), 2))
            quars = [ np.percentile(sorted_vals, q) for q in range(0, 101, 25) ]
            shas = []
            for q in quars:
                cs_idx = np.searchsorted(sorted_vals, q)
                shas += [ sorted_shas[cs_idx] ]
            self.stat['quar'][k] = StatQuartile(quars, shas)
            self.stat['sum'][k] = sum(sorted_vals)
        self.is_inited = True

    def Print(self):
        key_to_str = { 'src':   'Sources',
                       'func':  'Functions',
                       'struct':'Structs',
                       'init':  'Initializers',
                       'enum':  'Enum',
                       'add':   'Lines (+)',
                       'del':   '      (-)' }

        if (not self.is_inited): return ''

        barlen = 13 + 14 + len(str(StatAverage())) + len(str(StatQuartile()))
        log.RAW('=' * barlen)
        log.RAW(' {0:<14} {1:<{2}} {3:^{4}}  {5:>6} '
                .format('', 'Average (stdev)', len(str(StatAverage())),
                    'Quartile', len(str(StatQuartile())), 'Total'))
        log.RAW('-' * barlen)
        for k in CommitStat().value.keys():
            log.RAW(' {:<14} {} {}  {:>6} '
                    .format(key_to_str[k], self.stat['avg'][k],
                        self.stat['quar'][k], self.stat['sum'][k]))
            log.RAW(' {:<14} {:{}} {}'
                    .format('', '', len(str(StatAverage())),
                        self.stat['quar'][k].shastr()))
        log.RAW('=' * barlen)


def _ShowHistoryStat(css):
    hs = HistoryStat(css)
    hs.Print()
    log.INFO('total {} commit(s) analyzed.'.format(len(css)))


def AnalyzeHistoryStat(*args, **kwargs):
    HistoryStatFinder(*args, **kwargs)

def ShowCommitStatPack(cssp_path):
    try:
        with open(cssp_path, 'r') as f:
            cssp = json.load(f)
            css = [ CommitStat(**c) for c in cssp['css'] ]
    except Exception as err:
        log.FATAL("failed to load cssp: {}".format(err))

    for k, v in cssp['meta'].items():
        log.RAW("{:16}: {}".format(k, v))
    _ShowHistoryStat(css)
