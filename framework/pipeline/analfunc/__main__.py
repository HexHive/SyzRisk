#!/usr/bin/python

import sys, os, re
import argparse
import shutil
import threading
import subprocess
from datetime import datetime
from alive_progress import alive_bar
from common import log
from common import canonicalize


parser = argparse.ArgumentParser()
parser.add_argument('ext_path', type=str, help='path to extracted functions.')
parser.add_argument('-o', '--output', type=str, default='./output', help="output dir path.")
parser.add_argument('-m', '--matcher', type=str, default='all', help="comma-separated list of matchers to use. (default=all)")
parser.add_argument('-d', '--debug-mode', action='store_true', help="single-file debug mode")
args = parser.parse_args()


class PatMatLauncher():
    def __init__(self, ext_path, out_path, matcher):
        self.COMMIT_BATCH_MAX = 50 if (not args.debug_mode) else 1
        self.NR_WORKERS = 6 #40 if (not args.debug_mode) else 1         # laptop=4
        self.WORKDIR = "./.analfunc"
        self.EXTDIR = ext_path
        self.OUTDIR = out_path
        self.MATCHER = matcher

        if (os.path.exists(self.OUTDIR)):
            shutil.rmtree(self.OUTDIR)
        os.makedirs(self.OUTDIR, exist_ok=True)
        if (os.path.exists(self.WORKDIR)):
            shutil.rmtree(self.WORKDIR)
        os.makedirs(self.WORKDIR, exist_ok=True)

        self.commit_dirs = next(os.walk(ext_path))[1]

        self.nr_threads = 0
        self.thread_avail = [True for i in range(self.NR_WORKERS)]
        self.thread_avail_lock = threading.Lock()
        self.thread_inst = [None for i in range(self.NR_WORKERS)]
        self.worker_sema = threading.BoundedSemaphore(self.NR_WORKERS)

    def Run(self):
        with alive_bar(len(self.commit_dirs), title='Commits') as pb:
            commit_off = 0
            nr_analyzed_funcs = 0
            while (commit_off < len(self.commit_dirs)):
                # Get an available thread.
                n_thread = self.GetAvailableThread()
                log.DEBUG("selected Thread {}.".format(n_thread))

                # Create an un-conflicting commit batch, maximum COMMIT_BATCH_SIZE.
                _nr_commits = self.GetNextCommitBatch(commit_off)
                commit_batch = self.commit_dirs[commit_off:commit_off + _nr_commits]
                commit_off += _nr_commits
                assert(len(commit_batch) > 0)
                log.DEBUG("chose {} commits for a batch.".format(
                    len(commit_batch)))

                basedir_path = self.WORKDIR + '/thread' + str(n_thread)
                os.makedirs(basedir_path + '/body')
                os.makedirs(basedir_path + '/line')

                nr_funcs = 0 
                for ver in ['new', 'old']:
                    # Prepare the batch.
                    _nr_funcs = self.PrepareCommitBatch(commit_batch, basedir_path, ver)
                    nr_funcs = max(nr_funcs, _nr_funcs)
                nr_analyzed_funcs += nr_funcs
                # TODO: print --> log.INFO
                print("created a batch: tid={}, size={}, path={}, #funcs={}"\
                        .format(n_thread, len(commit_batch), basedir_path, nr_analyzed_funcs))

                # Run Joern with the batch.
                log.DEBUG("launching Joern...")
                def _LaunchJoern(n_thread, basedir_path, commit_batch):
                    with open(basedir_path + '/joern_log', 'w') as lg:
                        _redir = { "stdout": lg, "stderr": lg }
                        if (args.debug_mode): _redir = {}
                        joern_proc = subprocess.Popen(["joern", "--script", 
                            os.path.dirname(os.path.realpath(__file__)) + "/entry.sc",
                            "--params", "outdir=" + os.path.realpath(self.OUTDIR) + \
                                    ",matcher=" + self.MATCHER], cwd=basedir_path,
                                    **_redir)
                        joern_proc.communicate()
                    if (joern_proc.returncode):
                        error_id = datetime.now().strftime("%Y%m%d_%H%M%S")
                        with open(basedir_path + '/joern_log', 'r') as lg:
                            lglines = lg.readlines()
                        with open(self.WORKDIR + '/joern_err', 'a') as er:
                            er.write("*** Joern error ({}) ***\n".format(error_id))
                            er.write("batch = [{}, ..., {}] ({} commits)\n"\
                                    .format(commit_batch[0], commit_batch[-1],
                                        len(commit_batch)))
                            er.write("joern_log=...\n")
                            er.writelines(lglines)
                            er.write("\n")
                            error_copy_dir = self.WORKDIR + "/error-" + error_id
                            for i in range(5):
                                if (i != 0): error_copy_dir += "_" + str(i)
                                if (not os.path.exists(error_copy_dir)):
                                    shutil.copytree(basedir_path, error_copy_dir)
                                    break
                        print("joern error: thread={}, error_id={}".format(
                            n_thread, error_id))
                    shutil.rmtree(basedir_path)
                    self.thread_avail[n_thread] = True
                    pb(len(commit_batch))
                    self.worker_sema.release()
                thread = threading.Thread(target=_LaunchJoern, 
                        args=(n_thread, basedir_path, commit_batch)) 
                self.thread_inst[n_thread] = thread
                thread.start()

            for i in range(self.NR_WORKERS):
                if (self.thread_inst[i]):
                    self.thread_inst[i].join()

        try:
            os.rmdir(self.WORKDIR)
            log.INFO("analysis complete.")
        except:
            log.WARN("some batches went wrong. See '{}/joern_err'."\
                    .format(self.WORKDIR))


    def GetAvailableThread(self):
        n_thread = -1
        self.worker_sema.acquire()
        self.thread_avail_lock.acquire()
        for i in range(self.NR_WORKERS):
            if (self.thread_avail[i]):
                self.thread_avail[i] = False
                n_thread = i
                break
        assert(n_thread != -1)
        self.thread_avail_lock.release()
        return n_thread

    def GetNextCommitBatch(self, commit_off):
        n_commits = 0
        func_names = set()
        while (n_commits < self.COMMIT_BATCH_MAX and
                commit_off + n_commits < len(self.commit_dirs)):
            commit_dir = self.EXTDIR + '/' + \
                    self.commit_dirs[commit_off + n_commits]
            func_dirs = next(os.walk(commit_dir))[1]
            this_func_names = set([x.split('.')[0] for x in func_dirs])
            
            if (func_names.intersection(this_func_names)):
                break

            func_names.update(this_func_names)
            n_commits += 1

        return n_commits

    def PrepareCommitBatch(self, commit_batch, basedir_path, ver):
        batch_file = open('{}/{}.c'.format(basedir_path, ver), 'w')

        n_file = 0
        nr_funcs = 0
        for commit in commit_batch:
            funcs = next(os.walk('{}/{}'.format(self.EXTDIR, commit)))[1]
            for func in funcs:
                nr_funcs += 1
                funcname = func.split('.')[0]
                if (not os.path.exists('{}/{}/{}/{}'.format(
                        self.EXTDIR, commit, func, ver))):
                    continue

                with open('{}/{}/{}/{}/body'.format(
                        self.EXTDIR, commit, func, ver), 'r') as f:
                    lines = f.readlines()

                (lines, attrs) = canonicalize.CanonFuncBody(lines, funcname=funcname)
                
                # Stage the function body.
                # NOTE: we deliberately don't discern multiple copies
                # of functions in a commit.
                def AddMetadata(f, func, name, *values):
                    f.write("struct {}__{} {{ ".format(func, name))
                    for value in values:
                        f.write("void _{}; ".format(value))
                    f.write("};\n")

                with open('{}/body/{}_{}.c'.format(
                        basedir_path, ver, str(n_file)), 'w') as f:
                    f.writelines(lines)
                    f.write('\n')

                    # Info 1) commit hexsha
                    AddMetadata(f, funcname, "HEXSHA", commit)

                    # Info 2) (space-separated) attributes
                    AddMetadata(f, funcname, "ATTRS", *attrs)

                shutil.copyfile('{}/{}/{}/{}/line'.format(
                            self.EXTDIR, commit, func, ver),
                        '{}/line/{}_{}.c'.format(
                            basedir_path, ver, str(n_file)))

                batch_file.write("#include \"body/{}_{}.c\"\n".format(ver, str(n_file)))
                n_file += 1

        batch_file.close()
        return nr_funcs

launcher = PatMatLauncher(args.ext_path, args.output, args.matcher)
launcher.Run()
