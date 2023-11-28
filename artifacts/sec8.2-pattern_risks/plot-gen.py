import matplotlib.pyplot as plt
import numpy as np

import matplotlib
matplotlib.rcParams['pdf.fonttype'] = 42
matplotlib.rcParams['ps.fonttype'] = 42

with open('data-gen.csv', 'r') as f:
    lines = f.readlines()
labels = lines[0].strip().split(',')[1:]
lines = lines[1:]
data = {}
for line in lines:
    _toks = line.split(',')
    data[_toks[0]] = list(map(float, _toks[1:]))

#labels = ['G1', 'G2', 'G3', 'G4', 'G5']
#men_means = [20, 34, 30, 35, 27]
#women_means = [25, 32, 34, 20, 25]

#plt.style.use('seaborn-v0_8-notebook')

x = np.arange(len(labels))*7  # the label locations
width = 0.8  # the width of the bars

fig, ax = plt.subplots()
fig.set_size_inches(11, 1.9) #7

rects = []
i = 0
for l, d in data.items():
    _color = 'C' + str(i)
    if (i == 5): _color = 'black'
    rects += [ax.bar(0.5+x + (-3 + i) * width, d, width, label=l, 
        color=_color, edgecolor='black')]
    i += 1

ax.set_ylabel('General Risk')
ax.set_xticks(x, labels)
#ax.set_xticks(x, labels, rotation='vertical')
#ax.set_xticks(x, labels, rotation=45, ha='right', rotation_mode='anchor')
ax.set_xlim(-7+1, 7*24-1)
ax.set_ylim(0, 4)
ax.legend(bbox_to_anchor=(0, 1.02, 1, 0.2), loc='lower center', ncol=99)
ax.axhline(y=1, color='black', ls=':', linewidth=1)

ax.text(0.12, 0.88, "6.2", fontsize=9, transform=ax.transAxes)
ax.text(0.16, 0.88, "4.8", fontsize=9, transform=ax.transAxes)
ax.text(0.25, 0.88, "7.5", fontsize=9, transform=ax.transAxes)

fig.tight_layout()

plt.savefig('risk-gen.pdf')
#plt.show()
