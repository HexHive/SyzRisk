#!/usr/bin/python

from json import JSONEncoder


class StatAverage:
    def __init__(self, avg=0, stdev=0):
        self.avg = avg
        self.stdev = stdev

    def __repr__(self):
        stdev_str = "({:>4.2f})".format(self.stdev)
        return "{:>6.2f} {:<10}".format(self.avg, stdev_str)

class StatQuartile:
    def __init__(self, qs=[0 for _ in range(5)], shas=['0' for _ in range(5)]):
        self.qs = qs
        self.shas = shas

    def __repr__(self):
        return self.quarstr()

    def quarstr(self):
        retstr = "" 
        for q in self.qs: retstr += " {:>6}   <-> ".format(int(q)) 
        retstr = retstr[:-5]
        return retstr

    def shastr(self):
        retstr = "" 
        for s in self.shas: retstr += "({:>7})     ".format(s[0:7]) 
        retstr = retstr[:-5]
        return retstr


class CommitStat:
    def __init__(self, commit=None, value=None):
        self.commit = commit
        if (value):
            self.value = value
        else:
            self.value = { 'src':    0,  # Number of modified sources.
                           'func':   0,  # Number of modified functions.
                           'struct': 0,  # Number of modified struct defs.
                           'init':   0,  # Number of modified global struct inits.
                           'enum':   0,  # Number of modified enums.
                           'add':    0,  # Number of added lines.
                           'del':    0 } # Number of deleted lines.

    def __repr__(self):
        hexsha = self.commit.hexsha if self.commit else '(unknown commit)'
        retstr = '=== {} ==='.format(hexsha) + '\n'
        for k, v in self.value.items():
            retstr += ' {}: {}'.format(k, v) + '\n'
        return retstr[:-1]

class CommitStatPack():
    def __init__(self, css, **kwargs):
        self.meta = kwargs
        self.css = css
