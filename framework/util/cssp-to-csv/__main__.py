#!/usr/bin/python

import sys, os
import argparse
import json
import numpy as np
from common import log
from common import cstat


parser = argparse.ArgumentParser()
parser.add_argument('cssp', type=str, help='path to cssp.')
parser.add_argument('-o', '--out', type=str, default='', help='path to output file.')

args = parser.parse_args()


def main():
    if (not args.out):
        args.out = os.path.basename(args.cssp)
        args.out = os.path.splitext(args.out)[0] + '.csv'

    try:
        with open(args.cssp, 'r') as f:
            cssp = json.load(f)
    except Exception as err:
        log.FATAL('failed to open cssp: {}'.format(err))

    try:
        fout = open(args.out, 'w')
    except Exception as err:
        log.FATAL('failed to open out: {}'.format(err))

    # Write commit stats
    if (len(cssp['css']) > 0):
        key_to_str = { 'src':   'Sources',
                       'func':  'Functions',
                       'struct':'Structs',
                       'init':  'Initializers',
                       'add':   'Lines (+)',
                       'del':   'Lines (-)' }

        # Use the 0th element, as we don't want to mess the order of values.
        head_str = 'Commit SHA,'
        for k in cssp['css'][0]['value'].keys():
            head_str += key_to_str[k] + ','
        fout.write(head_str + '\n')

        for cs in cssp['css']:
            row_str = cs['commit'] + ','
            for v in cs['value'].values():
                row_str += str(v) + ','
            fout.write(row_str + '\n')

    # Write basic statistics
    fout.write('\n')
    if (len(cssp['css']) > 0):
        ops = [ ('Total', lambda l: sum(l)),
                ('Average', lambda l: round(np.mean(l), 2)),
                ('Stdev',   lambda l: round(np.std(l), 2)),
                ('Median',  lambda l: np.percentile(l, 50)),
                ('Min',     lambda l: np.percentile(l, 0)),
                ('Max',     lambda l: np.percentile(l, 100)) ]

        for head_str, op in ops:
            row_str = head_str + ","
            for k in cssp['css'][0]['value'].keys():
                l = [ x['value'][k] for x in cssp['css'] ]
                row_str += str(op(l)) + ","
            fout.write(row_str + '\n')

    # Write metadata
    fout.write('\n')
    for k, v in cssp['meta'].items():
        fout.write(k + ',' + str(v) + '\n')

    fout.close()


if __name__ == '__main__':
    main()
