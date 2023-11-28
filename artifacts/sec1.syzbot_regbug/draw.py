#!/usr/bin/python

import matplotlib.pyplot as plt
import numpy as np
import matplotlib.ticker as mtick
from matplotlib.legend_handler import HandlerTuple
from matplotlib.patches import Rectangle
from matplotlib.patches import Patch

import matplotlib
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42

# Data parsing
lines = {}
lines_per = {}
with open('lines.csv', 'r') as f:
    _ls = f.readlines()
    _ls = [x.strip().split(',') for x in _ls]
    labels = _ls[0][1:]
    for lidx in range(len(labels)):
        lines[labels[lidx]] = {}
        for y, num in zip([int(x[0]) for x in _ls[1:]], [int(x[lidx+1]) for x in _ls[1:]]):
            lines[labels[lidx]][y] = num
    lines_per['added'] = {}
    for y, num in lines['added'].items():
        lines_per['added'][y] = num / lines['total'][y]
    lines_per['deleted'] = {}
    for y, num in lines['deleted'].items():
        lines_per['deleted'][y] = num / lines['total'][y]

regs = {}
regs_per = {}
regs_tot = {}
years = []
with open('regs.csv', 'r') as f:
    _rs = f.readlines()
    _rs = [x.strip().split(',') for x in _rs]
    years = [y[0] for y in _rs[1:]]
    labels = _rs[0][1:]
    for ridx in range(len(labels)):
        regs[labels[ridx]] = {}
        for y, num in zip([int(x[0]) for x in _rs[1:]], [int(x[ridx+1]) for x in _rs[1:]]):
            regs[labels[ridx]][y] = num
            if (not y in regs_tot.keys()):
                regs_tot[y] = 0
            regs_tot[y] += num
    for l in range(len(labels)):
        ll = labels[l]
        regs_per[ll] = {}
        for y in regs[ll].keys():
            regs_per[ll][y] = regs[ll][y] / regs_tot[y]

# Draw
plt.rcParams['font.size'] = 8 
fig, ax = plt.subplots(1)
fig.set_size_inches(4.3, 2.3)

plt.rcParams['hatch.linewidth'] = 0.05
#plt.rcParams['hatch.color'] = 'lightgray'

_bot = [0 for _ in years]
_i = 0
plots = []
plot_nr = None
for l, nums in regs_per.items():
    if (_i == len(regs_per) - 1):
        lb = "non-reg"
        _color = 'whitesmoke' 
        hatch = '////////'
        #plot_nr = ax.scatter([], [], marker='s', facecolor=_color,
        #        edgecolor='k', s=32, hatch=hatch)
        #plot_nr = Rectangle(xy=(0, 0), width=50, height=8, linewidth=1,
        #        facecolor='whitesmoke', edgecolor='k', fill=True, hatch=hatch)
        plot_nr = Patch(facecolor='whitesmoke', edgecolor='k', hatch=hatch)
    else:
        lb = "<"+l+"yr"
        _color = (0.95+_i*0.016,0.8+_i*0.06,0.4+_i*0.15)
        hatch = ''
        plots += ax.plot([], [], 's', mfc=_color, mec='k', markersize=6.5)
    b = ax.bar(nums.keys(), nums.values(), facecolor=_color, bottom=_bot, 
            edgecolor='k', linewidth=0.5, hatch=hatch)
    _bot = [sum(x) for x in zip(_bot, nums.values())]
    _i += 1

fill_add = {}
fill_add[list(lines_per['added'].keys())[0]-0.5] = list(lines_per['added'].values())[0]
for y, num in lines_per['added'].items():
    fill_add[y+0.5] = num

ax.fill_between(fill_add.keys(), fill_add.values(), step='pre', alpha=0.4, color='forestgreen')
ax.step(fill_add.keys(), fill_add.values(), where='pre', color='forestgreen')

fill_del = {}
fill_del[list(lines_per['deleted'].keys())[0]-0.5] = list(lines_per['deleted'].values())[0]
for y, num in lines_per['deleted'].items():
    fill_del[y+0.5] = num

ax.fill_between(fill_del.keys(), fill_del.values(), step='pre', alpha=0.4, color='orangered')
ax.step(fill_del.keys(), fill_del.values(), where='pre', color='crimson')

syaxis = ax.secondary_yaxis('right')
ax.yaxis.set_major_formatter(mtick.PercentFormatter(1.0))
ax.set_ylim(0, 1)
syaxis.yaxis.set_major_formatter(mtick.PercentFormatter(1.0))
syaxis.set_ylim(0, 1)
ax.set_xlim(int(years[0])- 0.5, int(years[-1])+0.5)
ax.set_xlabel('Year')
ax.set_ylabel('% of Bugs', fontsize=9)
syaxis.set_ylabel('% of Changed Code', fontsize=9)

for axis in ['top','bottom','left','right']:
    ax.spines[axis].set_linewidth(0.5)
for axis in ['top','bottom','left','right']:
    syaxis.spines[axis].set_linewidth(0.5)

l1 = ax.legend([tuple(plots), plot_nr], ['Under {1,2,3+} yrs', 'Unknown'], 
    handler_map={tuple: HandlerTuple(ndivide=None)},
    #bbox_to_anchor=(-0.02, 1.02, 0.42, 0.2), loc='lower center', ncol=2,
    bbox_to_anchor=(-0.035, 1.04, 0.42, 0.2), loc='lower center', ncol=2,
    title="$\\bf{Root}$ $\\bf{Cause}$ $\\bf{Age}$ $\\bf{on}$ $\\bf{First}$ $\\bf{Detection}$", fontsize=8, columnspacing=0.7)
l1.get_title().set_fontsize(8)
l2 = ax.legend([Patch(facecolor='limegreen', edgecolor='forestgreen'), 
    Patch(facecolor='lightcoral', edgecolor='firebrick')], 
    ['Added', 'Deleted'], 
    handler_map={tuple: HandlerTuple(ndivide=None)},
    #bbox_to_anchor=(0.46, 1.02, 1, 0.2), loc='lower center', ncol=2,
    bbox_to_anchor=(0.465, 1.044, 1, 0.2), loc='lower center', ncol=2,
    title="$\\bf{Lines}$", fontsize=8, columnspacing=0.7)
l2.get_title().set_fontsize(8)
ax.add_artist(l1)
ax.add_artist(l2)


#patches = [patches[0]] + [Patch(facecolor='limegreen')] + \
#          [patches[1]] + [Patch(facecolor='limegreen')] + \
#          [patches[2]] + [Patch(facecolor='lightcoral')] + \
#          [patches[3]] + [Patch(facecolor='lightcoral')]
#
#ax.legend(handles=patches, labels=['', '', '', '', '', '',
#        '% of Regression Bugs ({0,1,2+}-year-old, Undecisive)', '% of {Added,Deleted} Lines of Code'], 
#    handletextpad=0.5, handlelength=1.0, columnspacing=-0.5,
#    bbox_to_anchor=(0, 1.02, 1, 0.2), loc='lower center', ncol=4)
#ax.legend(bbox_to_anchor=(0, 1.02, 1, 0.2), loc='lower center', ncol=99)
plt.tight_layout()
plt.subplots_adjust(top=0.78)
plt.subplots_adjust(left=0.2, right=0.8)
'''
ax[1].bar(_bars['be']['x'], height=_bars['be']['data'], 
        width=4, color='C0', linewidth=0.5)
#ax[1].step(_bars_combined['x'], _bars_combined['data'], 
#        linewidth=1, where='mid', color='C0')
ax[1].bar(_bars['rc']['x'], height=_bars['rc']['data'], 
        width=_rc_w, color='C1')
ax[1].step(_bars_step['x'], _bars_step['data'], 
        linewidth=1, where='pre', color='black')
ax[1].set_xlim(0, ALL_MAX)
ax[1].set_xlabel('(b) Last ' + str(ALL_MAX) + ' Commits, Newest to Oldest', fontsize=11)
#ax[1].bar(_bars['be']['x'], height=_bars['be']['data'], color='C1', edgecolor='black')
'''

#plt.show()
plt.savefig('reg-vs-ch.pdf')
