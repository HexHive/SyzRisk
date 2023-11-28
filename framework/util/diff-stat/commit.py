#!/usr/bin/python

import re, os, sys, git
import numpy as np
import json
from common import log 
from common import getstat
from common.cstat import *
from types import SimpleNamespace


class HistoryStatFinder:
    def __init__(self, repo_path, start_date='', end_date='', hexshas=[], 
            ignore_merge=True, output_path='', filter_path='',
            superset_path=''):
        self.repo = git.Repo(repo_path)

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

        if (filter_path):
            abs_filter_path = os.path.abspath(filter_path)
            filter_dir = os.path.dirname(abs_filter_path)
            filter_name = os.path.basename(abs_filter_path)
            if (filter_name[-3:] == '.py'):
                filter_name = filter_name[:-3]

            try:
                sys.path.insert(1, filter_dir)
                filter_mod = __import__(filter_name)
            except Exception as err:
                log.FATAL("failed to load custom commit filter: {}".format(err))

            filtered_cs = []
            for ci in range(len(self.commits)):
                log.INFO("filtering commits... ({}/{})".format(ci+1, len(self.commits)))
                res = filter_mod.CommitFilter(self.commits[ci])
                if (res): filtered_cs += [ self.commits[ci] ]
            self.commits = filtered_cs

        css = self.GetCommitStats(self.commits)
        _ShowHistoryStat(css)

        if (output_path):
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
                    filter_path=filter_path,
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

    def GetCommitStats(self, commits):
        css = []
        for ci in range(len(commits)):
            log.INFO("analyzing commit '" + commits[ci].hexsha + \
                    "' ({}/{})...".format(ci+1, len(commits)))
            css += [getstat.GetSingleCommitStat(self.repo, commits[ci])[0]]
        return css


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
