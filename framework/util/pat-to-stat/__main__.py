#/usr/bin/python

import os, sys, re
import json
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('ext_path', type=str, help="extracted funcs dir path.")
parser.add_argument('rc_info', type=str, help="root-cause info json path.")
parser.add_argument('-i', '--input', type=str, default='.', help="input dir path.")
#parser.add_argument('-o', '--output', type=str, default='.', help="output dir path.")
args = parser.parse_args()


rc_info = None 


def IsIncludedCommit(commit):
    if (not rc_info): return True
    elif (not commit in rc_info.keys()):
        return True 
    else:
        return (rc_info[commit]["selected"])

def IsRCCommit(commit):
    if (not rc_info): return True
    return (commit in rc_info.keys() and rc_info[commit]["selected"])

def IsRCFunc(commit, func):
    if (not rc_info): return True
    return (commit in rc_info.keys() and rc_info[commit]["selected"] and
            func in rc_info[commit]["funcs"])


def main():
    global rc_info
    if (args.rc_info):
        with open(args.rc_info, 'r') as f:
            rc_info = json.load(f)
            rc_info_new = dict()
            for k, v in rc_info.items():
                rc_info_new[k[0:12]] = rc_info[k]
            rc_info = rc_info_new

    func_stat = dict() 
    rc_stat = dict()
    mat_count = dict() 
    mat_rc_count = dict()

    commit_dirs = next(os.walk(args.input))[1]
    for commit_dir in commit_dirs:
        if (not IsIncludedCommit(commit_dir)): continue
        func_stat[commit_dir] = dict()  # func_name -> list of mats
        if (IsRCCommit(commit_dir)):
            rc_stat[commit_dir] = dict()
            
        func_dirs = next(os.walk("{}/{}".format(args.input, commit_dir)))[1]
        for func_dir in func_dirs:
            func_stat[commit_dir][func_dir] = []
            if (IsRCFunc(commit_dir, func_dir)):
                rc_stat[commit_dir][func_dir] = []

            mat_dirs = next(os.walk("{}/{}/{}".format(args.input, commit_dir, func_dir)))[1]
            for mat_dir in mat_dirs:
                func_stat[commit_dir][func_dir] += [mat_dir]
                if (mat_dir not in mat_count.keys()):
                    mat_count[mat_dir] = 0
                mat_count[mat_dir] += 1
                if (IsRCFunc(commit_dir, func_dir)):
                    if (mat_dir not in mat_rc_count.keys()):
                        mat_rc_count[mat_dir] = 0
                    mat_rc_count[mat_dir] += 1

    pool_commits = next(os.walk(args.ext_path))[1]
    pool_commits = list(filter(lambda x : IsIncludedCommit(x), pool_commits))
    print("info: selected {}/{} ({:.2f}%) commits in the pool.".format(
        len(func_stat.keys()), len(pool_commits),
        len(func_stat.keys()) / len(pool_commits) * 100))

    n_pool_funcs = 0
    for com in next(os.walk(args.ext_path))[1]:
        funcs = next(os.walk(args.ext_path + "/" + com))[1]
        n_pool_funcs += len(funcs)

    n_selected_funcs = 0
    for k, v in func_stat.items():
        n_selected_funcs += len(v.keys())

    print("info: selected {}/{} ({:.2f}%) functions in the pool.".format(
        n_selected_funcs, n_pool_funcs,
        n_selected_funcs / n_pool_funcs * 100))
    
    if (rc_info):
        rc_commits = rc_info.keys()
        print("info: selected {}/{} ({:.2f}%) ROOT-CAUSE commits.".format(
            len(rc_stat.keys()), len(rc_commits),
            len(rc_stat.keys()) / len(rc_commits) * 100))

        n_rcfuncs = 0
        for k, v in rc_info.items():
            if ("selected" not in v or not v["selected"]): continue
            n_rcfuncs += len(v["funcs"])

        n_selected_rcfuncs = 0
        for _, v in rc_stat.items():
            n_selected_rcfuncs += len(v.keys())

        print("info: selected {}/{} ({:.2f}%) ROOT-CAUSE functions.".format(
            n_selected_rcfuncs, n_rcfuncs, n_selected_rcfuncs / n_rcfuncs * 100))
        
        print(" <pat_name>: <recall>, <precision>, <reduction>")
        for mat, count in mat_count.items():
            rc_count = mat_rc_count[mat] if mat in mat_rc_count.keys() else 0
            print(" {}: {:.2f}% ({}/{}), {:.2f}% ({}/{}), {:.2f}% ({}/{})".format(
                mat, 
                rc_count / n_rcfuncs * 100, rc_count, n_rcfuncs,
                rc_count / count * 100, rc_count, count,
                count / n_pool_funcs * 100, count, n_pool_funcs))

if __name__ == "__main__":
    main()
