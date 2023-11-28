#/usr/bin/python

import os, sys, re
import subprocess
import argparse


parser = argparse.ArgumentParser()
parser.add_argument('body_path', type=str, help='"body" path.') 
parser.add_argument('line_path', type=str, default='', nargs='?', help='"line" path.')
parser.add_argument('--matcher', '-m', type=str)
args = parser.parse_args()


def main():
    this_dir = os.path.dirname(os.path.abspath(__file__))
    with open(this_dir + '/template.sc', 'r') as f:
        lines = f.readlines()

    if (not args.line_path):
        args.line_path = "/tmp/noline"
        open(args.line_path, 'w').close()

    for i in range(len(lines)):
        lines[i] = re.sub('@@BODY_PATH@@', '"' + args.body_path + '"', lines[i])
        lines[i] = re.sub('@@LINE_PATH@@', '"' + args.line_path + '"', lines[i])

    with open('/tmp/filled.sc', 'w') as f:
        f.writelines(lines)

    joern_proc = subprocess.Popen(["joern", "--import", "/tmp/filled.sc"])
    joern_proc.communicate()

if __name__ == "__main__":
    main()
