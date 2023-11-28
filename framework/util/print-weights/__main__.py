#!/usr/bin/python

import os, sys, re
import math
import argparse
import json
import pattern
import churn 


default_tf = { 'pattern': 'exp365', 'churn': 'inv10' }
TimeTransFuncs = { 'exp365': lambda w, t: w * (2 ** (-t/365)),
                   'exp180': lambda w, t: w * (2 ** (-t/180)),
                   'exp90': lambda w, t: w * (2 ** (-t/90)),
                   'exp30': lambda w, t: w * (2 ** (-t/30)),
                   'inv10': lambda w, t: w * (10 / (t + 9)),
                   'none': lambda w, t: w }


parser = argparse.ArgumentParser()
parser.add_argument('repo_path', type=str)
parser.add_argument('weight_mode', type=str)
parser.add_argument('match_dir', type=str)
parser.add_argument('risk_dir', type=str)
parser.add_argument('end_date', type=str)
parser.add_argument('-t', '--time-trans', type=str, 
        choices=list(TimeTransFuncs.keys()), default='')
args = parser.parse_args()

if (not args.time_trans):
    args.time_trans = default_tf[args.weight_mode]

out_name = '{}-{}-{}'.format(args.weight_mode, args.risk_dir.split('/')[-1], args.time_trans)

file_func_weight=None
if (args.weight_mode == 'pattern'):
    file_func_weight = pattern.PrintWeights(args.repo_path, \
            args.match_dir, args.risk_dir, TimeTransFuncs[args.time_trans],
            args.end_date)
elif (args.weight_mode == 'churn'):
    # NOTE: 'risk_dir' for interval.
    file_func_weight = churn.PrintWeights(args.repo_path, \
            args.risk_dir, TimeTransFuncs[args.time_trans],
            args.end_date)
#elif (args.weight_mode == 'deepjit'):
#    pass


with open(out_name + '.json', 'w') as f:
    json.dump(file_func_weight, f, indent=2)

with open(out_name + '.csv', 'w') as f:
    f.write('file,func,weight\n')
    for r in file_func_weight:
        f.write(r['file'] + ',' + r['func'] + ',' + str(r['weight']) + '\n')
