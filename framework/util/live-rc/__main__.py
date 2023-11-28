import argparse
import os
from typing import get_args
from parse import *


def main():

    parser = argparse.ArgumentParser(description='Identify live root cause using git log')

    parser.add_argument('--init', dest='getDB', action='store_const',
                        const=get_log, default=open_log,
                        help='make a gitDB to identify live root cause (default: open a gitDB)')
    parser.add_argument('--date', metavar='date', required=False, type=str, 
                        help='a reference date (ex. 2022-01-01)')
    parser.add_argument('repo', metavar='gitRepo', type=str,
                        help='a path of a git repositiory to be identified')
    parser.add_argument('--db', metavar='db', default='.', type=str,
                        help='a path of gitDB (default: current path)')
    parser.add_argument('--after', default='1990-01-01 20:00:00', type=str,
                        help='starting from...')
    parser.add_argument('--out', default='gitDB.json', type=str)
    args = parser.parse_args()

    file = args.getDB(args, args.out, args.after)

    if file != None:
        analyze_live_rc(args, file)

        
if __name__ == '__main__':
    main()
