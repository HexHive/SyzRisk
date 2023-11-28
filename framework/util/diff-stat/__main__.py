#!/bin/usr/python

import sys, os
import argparse
import commit
import json
from common import log


parser = argparse.ArgumentParser()
parser.add_argument('-r', '--repo', type=str, default='', help='path to git repo.')
parser.add_argument('--after', type=str, default='1990-01-01', help='commits after... (yyyy-mm-dd)')
parser.add_argument('--before', type=str, default='2038-01-01', help='commits before... (yyyy-mm-dd)')
parser.add_argument('-i', '--ignore-merge', help="ignore merge commits.", action='store_true')
parser.add_argument('-o', '--output', type=str, default='', help="output path to print each commit's stat.")
parser.add_argument('-f', '--filter', type=str, default='', help="path to custom commit filter.")
parser.add_argument('-s', '--superset', type=str, default='', help="consider only the commits in the provided list.")
parser.add_argument('-l', '--hexsha-list', type=str, default='', help="list of hexshas to investigate")
parser.add_argument('-c', '--cssp', type=str, default='', help="path to cssp.")


args = parser.parse_args()


def main():
    #test_hexshas = ['4cb682964706deffb4861f0a91329ab3a705039f',
    #                '2d2f6f1b4799428d160c021dd652bc3e3593945e',
    #                '11c514a99bb960941535134f0587102855e8ddee',
    #                '0fedc63fadf0404a729e73a35349481c8009c02f',
    #                '4731210c09f5977300f439b6c56ba220c65b2348',
    #                '3a08c2fd763450a927d1130de078d6f9e74944fb',
    #                '13fcc6837049f1bd76d57e9abc217a91fdbad764',
    #                '45a86681844e375bef6f6add272ccc309bb6a08d']

    test_hexshas = []
    if (args.hexsha_list):
        print(args.hexsha_list)
        try:
            with open(args.hexsha_list, 'r') as f:
                test_hexshas = json.load(f)
        except Exception as err:
            log.FATAL("failed to load hexsha list: {}".format(err))

    if (not args.cssp and not args.repo):
        parser.print_help()
        log.FATAL("either a repo path or a cssp path should be provided.")

    # I knew it too late that Git assumes the 'current' time if the exact time
    # (to the second resolution) was not given. Since the first data is drawn
    # at around 8:00 PM, we just rather set it our default time and leave the
    # first data intact.
    args.after += ' 20:00:00'
    args.before += ' 20:00:00'

    if (not args.cssp):
        commit.AnalyzeHistoryStat(args.repo, 
                start_date = args.after, 
                end_date = args.before,
                hexshas = test_hexshas,
                ignore_merge = args.ignore_merge,
                output_path = args.output,
                filter_path = args.filter,
                superset_path = args.superset,
                )
    else:
        commit.ShowCommitStatPack(args.cssp)

if __name__ == '__main__':
    main()
