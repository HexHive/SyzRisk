#/usr/bin/python

import os
import matplotlib.pyplot as plt
import numpy as np
import findlrcs
import argparse
import json
from scipy.stats.mstats import gmean
import matplotlib.ticker as mtick

import matplotlib
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42

parser = argparse.ArgumentParser()
parser.add_argument('-m', '--mode', type=str, default='pattern')
parser.add_argument('ref_date', type=str)
args = parser.parse_args()


# Weight parsing
weights = {}
with open('weights/' + args.mode + '-' + args.ref_date + '.json', 'r') as f:
    _weights = json.load(f)
    for _w in _weights:
        file_func = _w['file'] + ":" + _w['func']
        weights[file_func] = _w['weight']

data = findlrcs.GetWeightList(args.ref_date, weights)


# RC
n75 = np.quantile(data['be'], 0.75)
norm = np.quantile(data['be'], 0.5)
n25 = np.quantile(data['be'], 0.25)

labels = ['RC', 'BE']
xticks = np.arange(len(labels))
width = 0.5

fig, ax = plt.subplots(1, 2, gridspec_kw={'width_ratios': [1, 4]})
fig.set_size_inches(5.2, 1.96)

rect = ax[0].boxplot(data['rc'], 
            positions=[2.1], 
            notch=False, widths=0.6, 
            showfliers=False,
            patch_artist=True, 
            boxprops=dict(facecolor='C2'),
            medianprops=dict(color='black'))
rect = ax[0].boxplot(data['be'], 
            positions=[2.9], 
            notch=False, widths=0.6, 
            showfliers=False,
            patch_artist=True, 
            boxprops=dict(facecolor='C0'),
            medianprops=dict(color='black'))

ax[0].set_ylim(-0.05, 1.15)
ax[0].set_xlim(1.5, 3.5)
ax[0].set_xticklabels(labels)
ax[0].set_xlabel('(a) Distribution')
ax[0].set_ylabel('Commit Weight')

ALL_MAX=20000

_bars = { c: {'x': [], 'data': []} for c in ['rc', 'be'] }
_bars_combined = { 'x': [], 'data': [] }
count = 0
for d in data['all'][:ALL_MAX]:
    _bars[d[1]]['x'] += [count]
    _bars[d[1]]['data'] += [float(d[0])]
    _bars_combined['x'] += [count]
    _bars_combined['data'] += [float(d[0])]
    count += 1

_rc_w = 80
_bars_step = { 'x': [], 'data': [] }
count = -1 
last_y = 0
for i in range(ALL_MAX):
    if (i in _bars['rc']['x']):
        count = 0
        last_y = _bars['rc']['data'][_bars['rc']['x'].index(i)]
    else:
        if (count >= 0):
            count += 1
        if (count >= _rc_w):
            count = -1 
            last_y = 0
    _bars_step['x'] += [i]
    _bars_step['data'] += [last_y]

ax[1].bar(_bars['be']['x'], height=_bars['be']['data'], 
        width=4, color='C0', linewidth=0.5)
ax[1].bar(_bars['rc']['x'], height=_bars['rc']['data'], 
        width=_rc_w, color='C2')
ax[1].step(_bars_step['x'], _bars_step['data'], 
        linewidth=1, where='pre', color='black')
ax[1].set_xlim(0, ALL_MAX)
ax[1].set_xlabel('(b) Last ' + str(ALL_MAX) + ' Commits, Newest to Oldest')

fig.subplots_adjust(top=0.93)
fig.subplots_adjust(bottom=0.18)
fig.subplots_adjust(left=0.1)
fig.subplots_adjust(right=0.95)

ax[1].legend([plt.Rectangle((0,0),1,1, edgecolor='black', facecolor='C2'), 
                plt.Rectangle((0,0),1,1, edgecolor='black', facecolor='C0')],
            ['Root-cause', 'Benign'], frameon=False, loc='upper right',
            handlelength=1, handleheight=1)

plt.tight_layout(pad=0.6)

os.makedirs('out', exist_ok=True)
plt.savefig('out/' + args.mode + '-' + args.ref_date + '-dist.pdf')
