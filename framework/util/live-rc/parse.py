#!/usr/bin/python

import os
import subprocess
import json
import re
import time
import datetime
import numpy as np
import requests 

import check
import models

import progressbar

from git import Repo

def get_log(args, filepath, start_date):
    print('[+] Collect git log...')

    create_json(args, start_date, filepath)
    file = open_log(args, filepath)

    return file

def open_log(args, filepath):
    if os.path.exists(args.db) == False:
        print('failed to open db path... (%s)' % os.path.abspath(args.db))
        return None

    file = open(filepath, 'r', encoding="utf-8")

    return file

def analyze_live_rc(args, file):

    if args.date == None:
        if file != None:
            file.close()
        return

    print('[+] Analyze live root causes...')
    logParser = GitLogParser()

    dbOuts = json.load(file)

    refDate = datetime.strptime(args.date, "%Y-%m-%d")
    for dbOut in dbOuts:
        commitDate = datetime.strptime(dbOut['current_date'], "%Y-%m-%d %H:%M:%S")
        fixedDate = datetime.strptime(dbOut['fixed_date'], "%Y-%m-%d %H:%M:%S")

        if refDate >= fixedDate and refDate < commitDate:
            logParser.parse_json(dbOut)

    logParser.save_log(args, 'liveRC[{}].json'.format(args.date))
        
    file.close()
    return

'''
def create_json1(repo, ref_date):
    logParser = GitLogParser()

    repo = Repo(repo)
    hcommit = repo.commit('b027789e5e50494c2325cc70c8642e7fd6059479')
    print(hcommit.message)
'''

def create_json(args, start_date, filepath):
    logParser = GitLogParser()
    repo = Repo(args.repo)

    commits = list(repo.iter_commits(None, "", after=start_date))
    for commit in progressbar.progressbar(commits):
        logParser.parse_message(repo, commit)

    print('[+] total {} commits found.'.format(len(commits)))
    logParser.save_log(args, filepath)

class GitLogParser(object):
    def __init__(self):
        self.commits = []
        self.count = 0
        self.ignore_count = 0
        self.merge_fix = 0

    def parse_message(self, repo, hcommit):
        msg = hcommit.message
        commit = models.CommitData() 
        (fixes_hash, vulnType) = check.CommitFilter(hcommit)

        # If it's not an identifiable memory bug fix, ignore it.
        if not vulnType: return 

        try: 
            fcommit = repo.commit(fixes_hash)
            # Find if the fixed commit is a merge commit.
            if (len(fcommit.parents) > 1):
                self.merge_fix += 1
        except Exception as e:
            self.ignore_count += 1
            return

        current = hcommit.committed_date
        fixed = fcommit.committed_date
        interval = str(hcommit.committed_datetime - fcommit.committed_datetime)
        commit.current_commit = str(hcommit)
        commit.fixed_commit = str(fcommit)
        commit.current_date = str(datetime.datetime.utcfromtimestamp(current))
        commit.fixed_date = str(datetime.datetime.utcfromtimestamp(fixed))
        commit.interval = interval
        commit.vuln_type = vulnType

        self.commits.append(commit)
        self.count += 1

    def parse_json(self, input):
        commit = models.CommitData()

        commit.current_commit = input['current_commit']
        commit.current_date   = input['current_date']
        commit.fixed_commit   = input['fixed_commit']
        commit.fixed_date     = input['fixed_date']
        commit.interval       = input['interval']

        self.commits.append(commit)
        self.count += 1

    def save_log(self, args, filepath):
        if os.path.exists(args.db) == False:
            print('failed to open db path... (%s)' % os.path.abspath(args.db))
            return False

        with open(filepath, 'w', encoding='utf-8') as f:
            json.dump(self, f, indent=4, cls=CommitEncoder, sort_keys=True)
        
        print('[*] {} commits fixed merge commit'.format(self.merge_fix))
        print('[+] ignored {} commits.'.format(self.ignore_count))
        print('[+] saved {} commits in {}.'.format(self.count, filepath))
        return True




class CommitEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, GitLogParser):
            encoded_logs = list()
            for commit in obj.commits:
                encoded_logs.append(commit.to_json())
            return encoded_logs
        return super(CommitEncoder, self).default(obj)
