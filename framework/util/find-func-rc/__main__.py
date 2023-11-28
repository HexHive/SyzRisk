#!/usr/bin/python

import sys, os
import argparse
import json
import git
from datetime import datetime
from common import log
from common import getstat


parser = argparse.ArgumentParser()

parser.add_argument('repo_path', type=str, default='', help='path to git repo.')
parser.add_argument('info_path', type=str, default='', help='path to json RC info.')
parser.add_argument('--rc-after', type=str, default='2020-01-01 20:00:00')
parser.add_argument('--rc-before', type=str, default='2021-01-01 20:00:00')
parser.add_argument('-m', '--ignore-merge', help="ignore merge commits.", default=True, action='store_true')
parser.add_argument('-i', '--ignore-init-commit', help="ignore init commits.", default=True, action='store_true')
parser.add_argument('-o', '--output', type=str, default='out.json', help="output path to print each commit's stat.")

args = parser.parse_args()


def FindTokenWiseSimilarNames(name_set, cmp_name):
    res = []
    for org_name in name_set:
        org_name_toks = org_name.split('_')
        cmp_name_toks = cmp_name.split('_')
        
        org_off = 0
        cmp_off = 0
        _replaced = False
        _added = False
        _matched = True

        lendiff = abs(len(org_name_toks) - len(cmp_name_toks))
        if (lendiff == 1):
            _added = True
        elif (lendiff > 1):
            continue

        while (org_off < len(org_name_toks) and 
                cmp_off < len(cmp_name_toks)):
            if (org_name_toks[org_off] == cmp_name_toks[cmp_off]):
                org_off += 1
                cmp_off += 1
            elif (cmp_off + 1 < len(cmp_name_toks) and
                    org_name_toks[org_off] == cmp_name_toks[cmp_off + 1]):
                org_off += 1
                cmp_off += 2
                _added = True
            elif (org_off + 1 < len(org_name_toks) and
                    org_name_toks[org_off + 1] == cmp_name_toks[cmp_off]):
                org_off += 2
                cmp_off += 1
                _added = True
            elif (org_off + 1 < len(org_name_toks) and
                    cmp_off + 1 < len(cmp_name_toks) and
                    org_name_toks[org_off+1] == cmp_name_toks[cmp_off+1]):
                org_off += 2
                cmp_off += 2
                _replaced = True
            else:
                _matched = False
                break

            if (_replaced and _added):
                _matched = False
                break

        if (_matched):
            res += [org_name]

    return res


def main():
    repo = git.Repo(args.repo_path)
    with open(args.info_path, 'r') as f:
        rcs = json.load(f)

    rc_funcs_dic = {}
    rc_nr_total_funcs = 0
    rc_nr_commit = 0
    rc_nr_selected = 0
    rc_nr_funcs_per_bt = {}

    i = 0
    _start = False
    for rc in rcs:
        i += 1
        print(str(i) + "/" + str(len(rcs)) + ',' + rc['current_commit'])

        bc = repo.commit(rc['fixed_commit'])
        fc = repo.commit(rc['current_commit'])

        bc_date = datetime.strptime(rc['fixed_date'], "%Y-%m-%d %H:%M:%S")
        fc_date = datetime.strptime(rc['current_date'], "%Y-%m-%d %H:%M:%S")
        after_date = datetime.strptime(args.rc_after, "%Y-%m-%d %H:%M:%S")
        before_date = datetime.strptime(args.rc_before, "%Y-%m-%d %H:%M:%S")
        if (bc_date < after_date or before_date < bc_date):
            continue

        if (args.ignore_merge and 
                (len(bc.parents) > 1 or len(fc.parents) > 1)):
            continue

        if (args.ignore_init_commit and
                (len(bc.parents) == 0 or len(fc.parents) == 0)):
            continue

        _, bdcs, bedges = getstat.GetSingleCommitStat(repo, bc)
        _, fdcs, fedges = getstat.GetSingleCommitStat(repo, fc)

        rc_found = True
        rc_selected = True 

        # Include direct fixes.
        # If there is no exact matches, try searching for "similar"
        # names in token wise and give it a go.
        rc_funcs = bdcs['func'].intersection(fdcs['func']) 
        if (not rc_funcs):
            for ffunc in fdcs['func']:
                rc_funcs.update(FindTokenWiseSimilarNames(bdcs['func'], ffunc))

        # Try include RC->FC callings.
        for bfunc, bedge in bedges.items():
            if (fdcs['func'].intersection(set(bedge))):
                rc_funcs.add(bfunc)

        # Try include FC->RC callings.
        for ffunc, fedge in fedges.items():
            called_bfuncs = bdcs['func'].intersection(set(fedge))
            rc_funcs.update(called_bfuncs)

        # If there is still no matches, just include all modified
        # functions.
        if (not rc_funcs):
            rc_funcs = bdcs['func']
            rc_found = False
            if (len(rc_funcs) == 0):
                continue

        # Ignore commits with too many root-cause functions, which is
        # uncertain whether they are all relevant. Also ignore commits
        # with 'unknown' functions due to the limitation of the
        # script.
        if (len(rc_funcs) > 8 or '(unknown)' in rc_funcs):
            rc_funcs = set()
            rc_selected = False

        rc_nr_commit += 1
        if (rc_selected): rc_nr_selected += 1

        """
        rc_groups = [] 
        for rc_func in rc_funcs:
            _group = list(filter(lambda g: rc_func in g, bgroups))
            assert(len(_group) == 1)
            if (_group[0] not in rc_groups):
                rc_groups += [_group[0]]
        rc_groups = [list(x) for x in rc_groups]
        """
        rc_groups = [[x] for x in rc_funcs]
        if (not rc_found and rc_selected):
            _funcs = [{ 'group': x, 'bug-type': rc['vuln_type'] } for x in rc_groups]
            print(rc_funcs)
            print(_funcs)
            assert(len(_funcs) != 0)

        if (rc['fixed_commit'] not in rc_funcs_dic.keys()):
            rc_funcs_dic[rc['fixed_commit']] = {
                    'fixed-by': rc['current_commit'],
                    'found': rc_found, 
                    'funcs': [{ 'group': x, 'bug-type': rc['vuln_type'] } for x in rc_groups],
                    'selected': rc_selected}
        elif (rc_selected and rc_funcs_dic[rc['fixed_commit']]['selected']):
            # TODO: multiple fixes.
            elem = rc_funcs_dic[rc['fixed_commit']]
            elem['found'] = elem['found'] and rc_found
            for rc_group in rc_groups:
                group_exists = False
                for gbt in elem['funcs']:
                    rc_g = gbt['group']
                    rc_bt = gbt['bug-type']
                    if (rc_group == rc_g):
                        for vt in rc['vuln_type']:
                            if (vt not in rc_bt):
                                rc_bt += [vt]
                    group_exists = True
                    break
                if (not group_exists):
                    elem['funcs'] += [{ 'group': rc_group, 
                        'bug-type': rc['vuln_type'] }]


    rc_funcs_dic['total'] = { 
            'nr_funcs': rc_nr_total_funcs,
            'nr_commits': rc_nr_commit, 
            'nr_commit_selected': rc_nr_selected }

    with open(args.output, 'w') as f:
        json.dump(rc_funcs_dic, f, indent=2)
    

if __name__ == '__main__':
    main()
