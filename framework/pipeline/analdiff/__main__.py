#!/bin/usr/python

import sys, os
import argparse
import commit
import shutil
import json
from common import log


parser = argparse.ArgumentParser()
parser.add_argument('repo', type=str, help='path to git repo.')
parser.add_argument('--after', type=str, default='1990-01-01 20:00:00', help='commits after... (yyyy-mm-dd)')
parser.add_argument('--before', type=str, default='2038-01-01 20:00:00', help='commits before... (yyyy-mm-dd)')
parser.add_argument('-o', '--output', type=str, default='./output', help="output dir path.")

parser.add_argument('-m', '--matcher', type=str, default='none', help="comma-separated list of matchers to use. (default=none)")
parser.add_argument('-i', '--ignore-merge', default=True, help="ignore merge commits.", action='store_true')
parser.add_argument('-e', '--extract-funcs', default=False, help="extract all modified functions.", action="store_true")
parser.add_argument('-s', '--superset', type=str, default='', help="consider only the commits in the provided list.")
parser.add_argument('-l', '--hexsha-list', type=str, default='', help="list of hexshas to investigate")


args = parser.parse_args()


def main():
#    test_hexshas = ['4cb682964706deffb4861f0a91329ab3a705039f',
#                    '2d2f6f1b4799428d160c021dd652bc3e3593945e',
#                    '11c514a99bb960941535134f0587102855e8ddee',
#                    '0fedc63fadf0404a729e73a35349481c8009c02f',
#                    '4731210c09f5977300f439b6c56ba220c65b2348',
#                    '3a08c2fd763450a927d1130de078d6f9e74944fb',
#                    '13fcc6837049f1bd76d57e9abc217a91fdbad764',
#                    '45a86681844e375bef6f6add272ccc309bb6a08d']
#    test_hexshas = ['3a08c2fd763450a927d1130de078d6f9e74944fb']
#    test_hexshas = ['13fcc6837049f1bd76d57e9abc217a91fdbad764']
#    test_hexshas = ['45a86681844e375bef6f6add272ccc309bb6a08d']
#    test_hexshas = ['61177e911dad660df86a4553eb01c95ece2f6a82']

    test_hexshas = []

    if (args.hexsha_list):
        print(args.hexsha_list)
        try:
            with open(args.hexsha_list, 'r') as f:
                test_hexshas = json.load(f)
        except Exception as err:
            log.FATAL("failed to load hexsha list: {}".format(err))

    # I figured it out too late that Git assumes the 'current' time if
    # the exact time (to the second resolution) was not given.
    # (https://stackoverflow.com/questions/36027574/git-rev-list-command-gives-different-result-on-different-time)
    # Since the first data is drawn at around 8:00 PM, we just rather
    # set it our default time and leave the first data intact.
    args.after += ' 20:00:00'
    args.before += ' 20:00:00'

    if (args.matcher == 'all'):
        # NOTE: the directory name is a short name of the matcher. 
        DIR_PREFIX = os.path.dirname(os.path.abspath(__file__))
        matcher_list = list(filter(lambda n: n != '__pycache__',
            next(os.walk(DIR_PREFIX + '/matcher'))[1]))
    elif (args.matcher == 'none'):
        matcher_list = []
    else:
        matcher_list = args.matcher.split(',')

    # NOTE: in case this script has crashed while doing some
    # time-consuming jobs, comment them out.
    if (os.path.exists(args.output)):
        log.INFO("output directory already exists. removing...")
        shutil.rmtree(args.output)

    os.makedirs(args.output, exist_ok=True)
    os.makedirs(args.output + '/matcher', exist_ok=True)
    log.OUT_FILE(args.output + '/log')

    log.DEBUG("matching with the patterns...: {}", matcher_list)

    extfunc_path = ''
    if (args.extract_funcs):
        extfunc_path = args.output + '/extfunc'

    commit.AnalyzeHistoryStat(args.repo, 
            start_date = args.after, 
            end_date = args.before,
            hexshas = test_hexshas,
            ignore_merge = args.ignore_merge,
            output_dir = args.output,
            matcher_list = matcher_list,
            superset_path = args.superset,
            ext_path = extfunc_path)

if __name__ == '__main__':
    main()
