#/usr/bin/python

import matplotlib.pyplot as plt
import numpy as np
import findlrcs
import argparse
import json
from scipy.stats.mstats import gmean
import matplotlib.ticker as mtick
import os
from matplotlib.transforms import Bbox

import matplotlib
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42


modes = [('churn', ''), ('pattern', '-exp30'), ('pattern', '-exp90')]
ref_dates = ['2019-07-01', '2020-07-01', '2021-07-01']

# Weight parsing
datas = []
for ref_date in ref_dates:
    datas_m = {}
    datas += [datas_m]
    for mode in modes:
        weights = {}
        with open('weights/' + mode[0] + '-' + datestr + mode[1] + '.json', 'r') as f:
            _weights = json.load(f)
            for _w in _weights:
                file_func = _w['file'] + ":" + _w['func']
                weights[file_func] = _w['weight']
        data = findlrcs.GetWeightList(ref_date, weights)
        datas_m[mode[0] + mode[1]] = data

# RC
fig, ax = plt.subplots(3, 3)
WIDTH=3.5
HEIGHT=5.4
fig.set_size_inches(WIDTH, HEIGHT)

j = 0
for datas_m in datas:
    i = 0
    for mode, data in datas_m.items():
        n75 = np.quantile(data['be'], 0.75)
        norm = np.quantile(data['be'], 0.5)
        n25 = np.quantile(data['be'], 0.25)

        labels = ['RC', 'BE']
        xticks = np.arange(len(labels))
        width = 0.5

        rc_color = 'C2'
        be_color = 'C0'
        '''
        if (j == 0):
            rc_color = 'C1'
            be_color = 'C0'
        elif (j == 1):
            rc_color = 'sandybrown'
            be_color = (0.5, 0.62, 0.76)
        elif (j == 2):
            rc_color = 'navajowhite'
            be_color = (0.78, 0.83, 0.9)
        '''
        ax[i, j].axhspan(np.quantile(data['rc'], 0.25), np.quantile(data['rc'],
            0.75), facecolor='C2', alpha=0.15)
        #ax[i, j].axhspan(n25, n75, facecolor='C0', alpha=0.15)

        rect = ax[i, j].boxplot(data['rc'], 
                    positions=[2.1], 
                    notch=False, widths=0.6, 
                    showfliers=False,
                    patch_artist=True, 
                    boxprops=dict(facecolor=rc_color),
                    medianprops=dict(color='black'))
        rect = ax[i, j].boxplot(data['be'], 
                    positions=[2.9], 
                    notch=False, widths=0.6, 
                    showfliers=False,
                    patch_artist=True, 
                    boxprops=dict(facecolor=be_color),
                    medianprops=dict(color='black'))

        if (mode == 'churn'):
            ax[i, j].set_ylim(-0.05, 1.25)
            ax[i, j].set_yticks([0, 0.5, 1])
        elif (mode == 'pattern-exp30'):
            ax[i, j].set_ylim(-2, 34)
            ax[i, j].set_yticks([0, 10, 20, 30])
        elif (mode == 'pattern-exp90'):
            ax[i, j].set_ylim(-3, 82)
            ax[i, j].set_yticks([0, 20, 40, 60, 80])
        ax[i, j].set_xlim(1.5, 3.5)
        ax[i, j].set_xticklabels(labels)
        if (j == 0): ax[i, j].set_xlabel('Jul 1, 2019')
        elif (j == 1): ax[i, j].set_xlabel('Jul 1, 2020')
        elif (j == 2): ax[i, j].set_xlabel('Jul 1, 2021')
        #if (i == 2): ax[i, j].set_xlabel('Distribution')
        if (j == 0): ax[i, j].set_ylabel('Commit Weight')
        if (j != 0): ax[i, j].set_yticklabels([])

        ax[i, j].text(0.05, 0.89, 
                "Δ=" + str(round(np.quantile(data['rc'], 0.5) / norm, 3)),
                transform=ax[i, j].transAxes,
                fontsize=8)
        #ax[i, j].axhline(y=np.quantile(data['rc'], 0.5), color='black',
        #        ls='-', linewidth=1)
        i += 1
    j += 1

plt.tight_layout(pad=0.3)

os.makedirs('out', exist_ok=True)

#plt.show()
plt.savefig('out/dist-comb.pdf')

#bbox = Bbox([[0, HEIGHT-1*1.555], [HEIGHT, HEIGHT-0*1.555]])
bbox = Bbox([[0, HEIGHT-1*HEIGHT/3], [WIDTH, HEIGHT-0*HEIGHT/3]])
plt.savefig('out/dist-comb-churn.pdf', bbox_inches=bbox)
#bbox = Bbox([[0, HEIGHT-2*HEIGHT/3], [WIDTH, HEIGHT-1*HEIGHT/3]])
bbox = Bbox([[0, HEIGHT-2*HEIGHT/3], [WIDTH, HEIGHT-1*HEIGHT/3]])
plt.savefig('out/dist-comb-exp30.pdf', bbox_inches=bbox)
#bbox = Bbox([[0, 0], [HEIGHT, HEIGHT-2*HEIGHT/3]])
bbox = Bbox([[0, 0], [WIDTH, HEIGHT-2*HEIGHT/3]])
plt.savefig('out/dist-comb-exp90.pdf', bbox_inches=bbox)
