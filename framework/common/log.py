#!/usr/bin/python

from termcolor import colored
import sys

outfile = sys.stdout

def OUT_FILE(path):
    global outfile
    outfile = open(path, 'w')

def RAW(*args):
    print(' '.join(map(str, args)), file=outfile)
    pass

def DEBUG(*args):
    #print('debug: ', end='')
    #print(' '.join(map(str, args)))
    pass

def INFO(*args):
    print('info: ', end='', file=outfile)
    print(' '.join(map(str, args)), file=outfile)
    pass

def SPECIAL(*args):
    print('info: ', end='', file=outfile)
    print(colored(' '.join(map(str, args)), 'red'), file=outfile)

def WARN(*args):
    print('warning: ', end='', file=outfile)
    print(' '.join(map(str, args)), file=outfile)
    pass

def ERROR(*args):
    print('error: ', end='', file=outfile)
    print(' '.join(map(str, args)), file=outfile)
    pass

def FATAL(*args):
    print('fatal: ', end='', file=outfile)
    print(' '.join(map(str, args)), file=outfile)
    exit()
    pass
