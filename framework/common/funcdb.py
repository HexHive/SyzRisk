#!/usr/bin/python

import os, sys, re

def TryGetFuncFileMap(hexsha):
    THIS_DIR_PATH = os.path.dirname(os.path.realpath(__file__))
    FUNC_DB_PATH = THIS_DIR_PATH + "/../../data/funcs/db"
    commit_path = FUNC_DB_PATH + '/' + hexsha[:12]
    if (os.path.exists(commit_path)):
        funcfile = {}
        for func in next(os.walk(commit_path))[1]:
            new_file_path = "{}/{}/new/file".format(commit_path, func)
            old_file_path = "{}/{}/old/file".format(commit_path, func)
            if (os.path.exists(new_file_path)):
                file_path = new_file_path
            elif (os.path.exists(old_file_path)):
                file_path = old_file_path
            else:
                file_path = None

            if (file_path):
                with open(file_path, "r") as f:
                    funcfile[func] = f.readline().strip()
        return funcfile
    else:
        return None
