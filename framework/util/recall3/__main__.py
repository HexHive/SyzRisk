#!/usr/bin/python

import os, sys, re
import json, csv
import argparse
import shutil
import math
import numpy as np 
from scipy.stats import gmean
from statistics import mean


SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
SC_SIZE=200
SIZE_CLASS_MAX = 8
COMMIT_SIZE_CUTOFF = 512

parser = argparse.ArgumentParser()
parser.add_argument('out_path', type=str, help='path to the match result dir.')
parser.add_argument('-s', '--sc-path', type=str, \
        default='/home/gwangmu/Projects/Regression/data/recin/2020/commit-funcs.json')
parser.add_argument('-r', '--rc-path', type=str, \
        default='/home/gwangmu/Projects/Regression/data/recin/2020/rcs.json')
parser.add_argument('--ssinfo-path', type=str, \
        default='/home/gwangmu/Projects/Regression/data/recin/2020/commit-subsys.json')
parser.add_argument('-c', '--corr-path', type=str, \
        default='/home/gwangmu/Projects/Regression/data/likeli/2019/sc-likeli.json')
parser.add_argument('-S', '--ss-path', type=str, \
        default='/home/gwangmu/Projects/Regression/data/likeli/2019/ss-likeli.json')
args = parser.parse_args()

SCINFO_PATH = args.sc_path
RCINFO_PATH = args.rc_path
RESDIR_PATH = args.out_path
CORRFACTOR_PATH = args.corr_path
SSFACTOR_PATH = args.ss_path
SSINFO_PATH = args.ssinfo_path
OUTDIR = 'recall'
PERBUG_OUT_PATH = './' + OUTDIR + '/recall.csv'
BREAK_OUT_PATH = './' + OUTDIR + '/breakdown.csv'
RANKDIST_OUT_PATH = './' + OUTDIR + '/rank.csv'
RANKCORR_OUT_PATH = './' + OUTDIR + '/rankcorr.csv'
RANKSC_OUT_PATH = './' + OUTDIR + '/ranksc.csv'
RANKCOM_OUT_PATH = './' + OUTDIR + '/rankcom.csv'
RANKCOM_CORR_OUT_PATH = './' + OUTDIR + '/rankcom_corr.csv'
RANKMAT_OUT_PATH = './' + OUTDIR + '/rankmat.csv'
COMSIZE_OUT_PATH = './' + OUTDIR + '/comsize.csv'

if (os.path.exists(OUTDIR)):
    shutil.rmtree(OUTDIR)
os.makedirs(OUTDIR, exist_ok=True)

#all_bts = set(['(total)'])
list_bts = [
        'buffer-overflow',
        'deadlock', 
        'kernel-panic',
        'memory-leak', 
        'NULL-dereference', 
        'race-condition',
        'use-after-free', 
        ]
list_pats = [
        'concurr_api',
        'locked_ctx',
        'entering_goto',
        'finalization',
        'inside_goto',
        'mm_api',
        'ptr_arith',
        'ptr_api',
        'ptr_off',
        'global_var',
        'initialization',
        'new_state',
        'struct_mod',
        'chained_deref',
        'ptr_promote',
        'split_fn',
        'ext_stmt',
        'swvar_mod',
        'struct_cast',
        'export_fn',
        'inside_switch',
        'asm_mod',
        'changing_err',
        ]

_list_pats = ['(more_than_1)'] + list_pats + ['(none)']
bin_bt_break = { bt: { pat: 0 for pat in _list_pats } for bt in list_bts }
bin_bt_break['(total)'] = { pat: 0 for pat in _list_pats }

#all_pats = set(['(more_than_1)'])
_list_bts = list_bts + ['(total)', '(all)']
bin_pat_bt = { pat: { bt: 0 for bt in _list_bts } for pat in list_pats }
bin_pat_bt['(total)'] = { bt: 0 for bt in _list_bts }
bin_pat_bt['(gt)'] = { bt: 0 for bt in _list_bts }

bin_rank_be = { n: 0 for n in range(len(list_pats)*SC_SIZE+1) }
bin_rank_rc = { n: 0 for n in range(len(list_pats)*SC_SIZE+1) }

bin_rank_corr_be = { n: 0 for n in range(len(list_pats)*50+1) }
bin_rank_corr_rc = { n: 0 for n in range(len(list_pats)*50+1) }

bin_rank_sc_be = { n: 0 for n in range(len(list_pats)*SC_SIZE+1) }
bin_rank_sc_rc = { n: 0 for n in range(len(list_pats)*SC_SIZE+1) }

bin_rank_commit_be = { n: 0 for n in range(len(list_pats)*100+1) }
bin_rank_commit_rc = { n: 0 for n in range(len(list_pats)*100+1) }

bin_rank_commit_corr_be = { n: 0 for n in range(len(list_pats)*100+1) }
bin_rank_commit_corr_rc = { n: 0 for n in range(len(list_pats)*100+1) }

bin_rank_mat_be = { n : 0 for n in range(len(list_pats)*10 + 1) }
bin_rank_mat_rc = { n : 0 for n in range(len(list_pats)*10 + 1) }

bin_com_size_be = { n : 0 for n in range(1000) }
bin_com_size_rc = { n : 0 for n in range(1000) }

unmatched_info = dict()

#ENERGY_CAP = 10000
#POWER_FACTOR = 0.690
#def GetEnergy(rank):
    #return min(ENERGY_CAP, (math.exp(rank)-1)*POWER_FACTOR)
def GetEnergy(rank):
    return 1 / (1 + math.exp(0.15 * (29 - rank)))


def GetMatchedPatterns(hexsha, funcname):
    matdir_path = '{}/{}/{}'.format(RESDIR_PATH, hexsha[0:12], funcname)
    if (not os.path.exists(matdir_path)):
        return []
    else:
        pats = []
        for mat_dir in next(os.walk(matdir_path))[1]:
            pats += [mat_dir]
            #pats += [mat_dir] * \
            #        max(1, len(os.listdir(matdir_path + '/' + mat_dir)))
        return pats

def GetNumMatched(hexsha, funcname):
    matdir_path = '{}/{}/{}'.format(RESDIR_PATH, hexsha[0:12], funcname)
    n = 0
    if (not os.path.exists(matdir_path)):
        return 0
    else:
        pats = next(os.walk(matdir_path))[1]
        for pat in pats:
            new_path = matdir_path + '/{}/new'.format(pat)
            if (os.path.exists(new_path)):
                with open(new_path, 'r') as f:
                    n += max(1, sum(1 for _ in f))
            old_path = matdir_path + '/{}/old'.format(pat)
            if (os.path.exists(old_path)):
                with open(old_path, 'r') as f:
                    n += max(1, sum(1 for _ in f))
        return n

def UpdateBins(pats, bts, group_size):
    pats = list(set(pats))
    
    for bt in bts:
        bin_pat_bt['(gt)'][bt] += group_size
    if (bts):
        bin_pat_bt['(gt)']['(total)'] += group_size
    bin_pat_bt['(gt)']['(all)'] += group_size

    for pat in pats:
        for bt in bts:
            if (pat not in bin_pat_bt.keys()):
                bin_pat_bt[pat] = dict()
            if (bt not in bin_pat_bt[pat].keys()):
                bin_pat_bt[pat][bt] = 0
            bin_pat_bt[pat][bt] += group_size
        if (bts):
            bin_pat_bt[pat]['(total)'] += group_size
        bin_pat_bt[pat]['(all)'] += group_size
    if (pats):
        if ('(total)' not in bin_pat_bt.keys()):
            bin_pat_bt['(total)'] = dict()
        for bt in bts:
            if (bt not in bin_pat_bt['(total)'].keys()):
                bin_pat_bt['(total)'][bt] = 0
            bin_pat_bt['(total)'][bt] += group_size
        if (bts):
            if ('(total)' not in bin_pat_bt['(total)'].keys()):
                bin_pat_bt['(total)']['(total)'] = 0
            bin_pat_bt['(total)']['(total)'] += group_size
        bin_pat_bt['(total)']['(all)'] += group_size

    one_pat = '(none)'
    if (len(pats) == 1):
        one_pat = pats[0]
    elif (len(pats) > 1):
        one_pat = '(more_than_1)'

    for bt in bts:
        if (bt not in bin_bt_break.keys()):
            bin_bt_break[bt] = dict()
        if (one_pat not in bin_bt_break[bt].keys()):
            bin_bt_break[bt][one_pat] = 0
        bin_bt_break[bt][one_pat] += group_size
    if ('(total)' not in bin_bt_break.keys()):
        bin_bt_break['(total)'] = dict()
    if (one_pat not in bin_bt_break['(total)'].keys()):
        bin_bt_break['(total)'][one_pat] = 0
    bin_bt_break['(total)'][one_pat] += group_size

def main():
    csize_energy_be = []
    csize_energy_rc = []
    csize_rank_ind_be = []
    csize_rank_ind_rc = []

    with open(RCINFO_PATH, 'r') as f:
        selected_rcs = json.load(f)

    with open(SCINFO_PATH, 'r') as f:
        com_funcs = json.load(f)

    with open(CORRFACTOR_PATH, 'r') as f:
        corr_factor = json.load(f)

    with open(SSINFO_PATH, 'r') as f:
        ss_info = json.load(f)

    with open(SSFACTOR_PATH, 'r') as f:
        ss_factor = json.load(f)

    def GetSizeClass(size): 
        return min(SIZE_CLASS_MAX, math.floor(math.log2(size))+1)
    def GetCorrRank(pats, hexsha, com_funcs_size):
        subsyses = ss_info[hexsha]
        rank = 0
        for pat in pats:
            size_class = GetSizeClass(com_funcs_size)
            factor_0 = corr_factor[pat]["0"]
            factor = corr_factor[pat][str(size_class)]
            factor_ss = max(ss_factor[pat][ss] for ss in subsyses)
            rank += math.log(max(0, factor_0, factor, factor_ss), 2.11) ** 3 
            # NOTE: 2.11 -> geomean of general likeli ( 2.11 --> 1 )
            #rank += math.log(factor_0)
            #rank += math.log(factor)
            #rank += math.log(factor_ss)
            #rank += (min(10, max(0, factor_0-1))**5/(2**2)) 
            #rank += (min(10, max(0, factor-1))**5/(2**2))
            #rank += (min(10, max(0, factor_ss-1))**2/(2**0))
        return min(800, max(0, math.floor(rank*10)))

    for hexsha, sc in com_funcs.items():
        if (hexsha in selected_rcs.keys()): continue
        ranks = []
        commit_pats = []
        com_funcs_size = len(sc)
        if (com_funcs_size > COMMIT_SIZE_CUTOFF): continue

        pats = [] 
        for func in sc:
            _pats = GetMatchedPatterns(hexsha, func)
            rank = min(len(list_pats)*50-1, GetCorrRank(_pats, hexsha, com_funcs_size))
            ranks += [rank]
            #if (com_funcs_size == 1):
            bin_rank_corr_be[rank] += 1
            pats += _pats
            csize_rank_ind_be += [(com_funcs_size, rank)]
        UpdateBins(pats, [], len(sc))
        bin_rank_be[len(pats)] += len(sc)
        bin_rank_sc_be[len(pats)] += 1

        _n = 0
        for func in sc:
            _n += GetNumMatched(hexsha, func)
        _n = min(_n, len(list_pats) * 10)
        bin_rank_mat_be[_n] += len(sc) 

        commit_pats += pats
        #commit_pats = list(set(commit_pats))
        n_commit_pats = min(len(commit_pats), len(list_pats)*100)
        bin_rank_commit_be[n_commit_pats] += 1 
        com_funcs_size = min(1000-1, com_funcs_size)
        bin_com_size_be[com_funcs_size] += 1
        crank = min(len(list_pats)*50-1, GetCorrRank(commit_pats, hexsha, com_funcs_size))
        bin_rank_commit_corr_be[crank] += 1
        csize_energy_be += [(com_funcs_size, sum(ranks), sum(GetEnergy(rank) for rank in ranks))]

    nr_funcs = 0
    for hexsha, bt in selected_rcs.items():
        ranks = []
        commit_pats = [] 
        sc = com_funcs[hexsha]
        com_funcs_size = len(sc)
        if (com_funcs_size > COMMIT_SIZE_CUTOFF): continue

        pats = []
        for func in sc:
            _pats = GetMatchedPatterns(hexsha, func)
            rank = min(len(list_pats)*50-1, GetCorrRank(_pats, hexsha, com_funcs_size))
            ranks += [rank]
            #if (com_funcs_size == 1):
            bin_rank_corr_rc[rank] += 1
            pats += _pats
            csize_rank_ind_rc += [(com_funcs_size, rank)]
        UpdateBins(pats, bt, len(sc))
        if (not pats):
            for _bt in bt:
                if (_bt not in unmatched_info.keys()):
                    unmatched_info[_bt] = []
                unmatched_info[_bt] += [(hexsha, sc)]
        bin_rank_rc[len(pats)] += len(sc)
        bin_rank_sc_rc[len(pats)] += 1
        nr_funcs += len(sc)

        _n = 0
        for func in sc:
            _n += GetNumMatched(hexsha, func)
        _n = min(_n, len(list_pats) * 10)
        bin_rank_mat_rc[_n] += len(sc)

        commit_pats += pats
        #commit_pats = list(set(commit_pats))
        n_commit_pats = min(len(commit_pats), len(list_pats)*100)
        bin_rank_commit_rc[n_commit_pats] += 1 
        com_funcs_size = min(1000-1, com_funcs_size)
        bin_com_size_rc[min(len(sc), 1000-1)] += 1
        crank = min(len(list_pats)*50-1, GetCorrRank(commit_pats, hexsha, com_funcs_size))
        bin_rank_commit_corr_rc[crank] += 1
        csize_energy_rc += [(com_funcs_size, sum(ranks), sum(GetEnergy(rank) for rank in ranks))]

    with open(PERBUG_OUT_PATH, 'w') as f:
        header = ['PATTERN'] + list_bts + ['(total)', '(all)']
        w = csv.DictWriter(f, header)
        w.writerow({ x: x for x in header })
        for pat, bts in bin_pat_bt.items():
            row = { 'PATTERN': pat }
            row.update(bts)
            w.writerow(row)

    with open(BREAK_OUT_PATH, 'w') as f:
        header = ['BUG_TYPE', '(more_than_1)'] + list_pats + ['(none)']
        w = csv.DictWriter(f, header)
        w.writerow({ x: x for x in header })
        for bt, pats in bin_bt_break.items():
            row = { 'BUG_TYPE': bt }
            row.update(pats)
            w.writerow(row)

    with open(RANKDIST_OUT_PATH, 'w') as f:
        header = ['RANK'] + list(range(len(list_pats)*SC_SIZE+1)) 
        w = csv.DictWriter(f, header)
        w.writerow({ x: x for x in header })
        row_rc = { 'RANK': 'rc' }
        row_rc.update(bin_rank_rc)
        w.writerow(row_rc)
        row_be = { 'RANK': 'be' }
        row_be.update(bin_rank_be)
        w.writerow(row_be)

    with open(RANKCORR_OUT_PATH, 'w') as f:
        header = ['RANK'] + list(range(len(list_pats)*50+1)) 
        w = csv.DictWriter(f, header)
        w.writerow({ x: x for x in header })
        row_rc = { 'RANK': 'rc' }
        row_rc.update(bin_rank_corr_rc)
        w.writerow(row_rc)
        row_be = { 'RANK': 'be' }
        row_be.update(bin_rank_corr_be)
        w.writerow(row_be)

    with open(RANKSC_OUT_PATH, 'w') as f:
        header = ['RANK'] + list(range(len(list_pats)*SC_SIZE+1)) 
        w = csv.DictWriter(f, header)
        w.writerow({ x: x for x in header })
        row_rc = { 'RANK': 'rc' }
        row_rc.update(bin_rank_sc_rc)
        w.writerow(row_rc)
        row_be = { 'RANK': 'be' }
        row_be.update(bin_rank_sc_be)
        w.writerow(row_be)

    with open(RANKCOM_OUT_PATH, 'w') as f:
        header = ['RANK'] + list(range(len(list_pats)*100+1)) 
        w = csv.DictWriter(f, header)
        w.writerow({ x: x for x in header })
        row_rc = { 'RANK': 'rc' }
        row_rc.update(bin_rank_commit_rc)
        w.writerow(row_rc)
        row_be = { 'RANK': 'be' }
        row_be.update(bin_rank_commit_be)
        w.writerow(row_be)

    with open(RANKCOM_CORR_OUT_PATH, 'w') as f:
        header = ['RANK'] + list(range(len(list_pats)*100+1)) 
        w = csv.DictWriter(f, header)
        w.writerow({ x: x for x in header })
        row_rc = { 'RANK': 'rc' }
        row_rc.update(bin_rank_commit_corr_rc)
        w.writerow(row_rc)
        row_be = { 'RANK': 'be' }
        row_be.update(bin_rank_commit_corr_be)
        w.writerow(row_be)

    with open(RANKMAT_OUT_PATH, 'w') as f:
        mat_max = max(list(bin_rank_mat_be.keys()) + list(bin_rank_mat_rc.keys()))
        header = ['RANK'] + list(range(mat_max+1)) 
        w = csv.DictWriter(f, header)
        w.writerow({ x: x for x in header })
        row_rc = { 'RANK': 'rc' }
        for k in range(mat_max+1): 
            if (k in bin_rank_mat_rc.keys()):
                row_rc[k] = bin_rank_mat_rc[k]
            else:
                row_rc[k] = 0
        w.writerow(row_rc)
        row_be = { 'RANK': 'be' }
        for k in range(mat_max+1): 
            if (k in bin_rank_mat_be.keys()):
                row_be[k] = bin_rank_mat_be[k]
            else:
                row_be[k] = 0
        w.writerow(row_be)

    with open(COMSIZE_OUT_PATH, 'w') as f:
        header = ['SIZE'] + list(range(1000)) 
        w = csv.DictWriter(f, header)
        w.writerow({ x: x for x in header })
        row_rc = { 'SIZE': 'rc' }
        row_rc.update(bin_com_size_rc)
        w.writerow(row_rc)
        row_be = { 'SIZE': 'be' }
        row_be.update(bin_com_size_be)
        w.writerow(row_be)

    with open('energy_be.csv', 'w') as f:
        header = ['SIZE', 'RANK', 'ENERGY']
        w = csv.DictWriter(f, header)
        w.writerow({ x: x for x in header })
        for e in csize_energy_be:
            w.writerow({ 'SIZE': e[0], 'RANK': e[1], 'ENERGY': e[2] })

    with open('energy_rc.csv', 'w') as f:
        header = ['SIZE', 'RANK', 'ENERGY']
        w = csv.DictWriter(f, header)
        w.writerow({ x: x for x in header })
        for e in csize_energy_rc:
            w.writerow({ 'SIZE': e[0], 'RANK': e[1], 'ENERGY': e[2] })

    with open('rank_ind_be.csv', 'w') as f:
        header = ['SIZE', 'RANK']
        w = csv.DictWriter(f, header)
        w.writerow({ x: x for x in header })
        for e in csize_rank_ind_be:
            w.writerow({ 'SIZE': e[0], 'RANK': e[1] })

    with open('rank_ind_rc.csv', 'w') as f:
        header = ['SIZE', 'RANK']
        w = csv.DictWriter(f, header)
        w.writerow({ x: x for x in header })
        for e in csize_rank_ind_rc:
            w.writerow({ 'SIZE': e[0], 'RANK': e[1] })

    if (os.path.exists('./unmatched')):
        shutil.rmtree('./unmatched')
    os.makedirs('./unmatched', exist_ok=True)

    for bt, ilist in unmatched_info.items():
        last_sha = ''
        nr_shas = 0
        nr_unmatched_funcs = 0
        with open('./unmatched/' + bt, 'w') as f:
            f.write('No.\tRC Hexsha\tSemi-commit Functions\n')
            for pair in ilist:
                if (last_sha != pair[0]):
                    last_sha = pair[0]
                    nr_shas += 1
                    f.write(str(nr_shas) + '\t' + pair[0])
                else:
                    f.write('\t\t')
                f.write(str(pair[1]))
                f.write('\n')
                nr_unmatched_funcs += len(pair[1])
            f.write('\t{}\t\t\t{}'.format(nr_shas, nr_unmatched_funcs))


    print("Total {} ground-truth functions".format(nr_funcs))


if __name__ == '__main__':
    main()
