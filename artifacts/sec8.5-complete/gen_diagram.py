#!/usr/bin/python

import matplotlib.pyplot as plt
from matplotlib.patches import Patch
from matplotlib_venn import venn3
from matplotlib_venn import venn3_unweighted
from matplotlib_venn import venn3_circles

import matplotlib
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42

def ParseData(filename):
    with open(filename, 'r') as f:
        _lines = f.readlines()
    labels = _lines[0].strip().split(',')
    colors = ['darkorange', 'royalblue', 'limegreen', 'forestgreen']
    _data = _lines[1:]
    data = [] 
    for _d in _data:
        dd = map(float, _d.strip().split(','))
        data += [list(zip(labels, dd))]
    return labels, colors, data

# 100 010 110 001 101 011 111
# fig1: churn/sr30/sr90
# fig1: syzkaller/sr30/sr90

def DataToBin(data, idxs):
    bins = []
    for d in data:
        b = 0
        i = 0
        i_real = 0
        for l, v in d:
            if (i in idxs):
                marker = 1 if (v < 10080) else 0
                b |= (marker << i_real)
                i_real += 1
            i += 1
        if (b != 0):
            print(b, d)
            bins += [b]
    return bins

def Draw(bins, labels, colors, ax):
    subsets = [0 for x in range((2 ** len(labels))-1)]
    for b in bins:
        subsets[b-1] += 1

    out = venn3_unweighted(subsets=subsets, ax=ax, set_labels=
        #['' for _ in range(len(labels))], 
        labels,
        set_colors=colors, alpha=0.5)
    #, set_labels=labels
    #venn3_circles(subsets=subsets, ax=ax, linewidth=0.5)
    venn3_circles(subsets=[1,1,1,1,1,1,1], ax=ax, linewidth=0.8)
    for text in out.set_labels:
        text.set_fontsize(11)
    for text in out.subset_labels:
        text.set_fontsize(12)


labels, colors, data = ParseData('data.csv')
bins_123 = DataToBin(data, [1, 2, 3])
bins_023 = DataToBin(data, [0, 2, 3])
fig, ax = plt.subplots(1, 5, gridspec_kw={'width_ratios': [0.1,1,0,1,0.1]})#[0.2, 1, 0.01, 1, 0.1]})
ax[0].axis('off')
ax[2].axis('off')
ax[4].axis('off')
fig.set_size_inches(6.5, 3.08)
Draw(bins_123, labels[1:], colors[1:], ax[1])
Draw(bins_023, [labels[0]] + labels[2:], [colors[0]] + colors[2:], ax[3])
#colors=[colors[1],colors[0]] + colors[2:]
#labels=[labels[1],labels[0]] + labels[2:]
handles=[]
for i in [1, 2, 3]:
    handles += [Patch(facecolor=colors[i], edgecolor='black', label=labels[i])]
#ax[1].legend(handles, labels[1:], loc='lower left', bbox_to_anchor=(-0.08, 0))
ax[1].set_title("(a) with SyzChurn", y=-0.21, size=12)
handles=[]
for i in [0, 2, 3]:
    handles += [Patch(facecolor=colors[i], edgecolor='black', label=labels[i])]
#ax[3].legend(handles, [labels[0]] + labels[2:], loc='lower left', bbox_to_anchor=(-0.08, 0))
ax[3].set_title("(b) with Syzkaller", y=-0.21, size=12)
plt.tight_layout(pad=0, h_pad=0, rect=(0, 0.13, 1, 1))
plt.savefig('diag.pdf')
#plt.show()
