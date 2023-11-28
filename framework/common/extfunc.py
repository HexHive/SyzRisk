from enum import Enum
import argparse
import re
import json
import os
import shutil
import git
from common import log
from distutils.dir_util import copy_tree

def ExtractFunc(org_repo, output_path, reuse_temp_dirs=False,
        old_commit="", new_commit="", force_out=False):

    # NOTE: if provided with a diff log.
    # with open(args.diffpath, 'r', errors='ignore') as difffile:
    #     difflines = difffile.readlines()
    # difflines = [l.replace('\n', '') for l in difflines]
    difflines = org_repo.git.diff(old_commit.hexsha, new_commit.hexsha)
    difflines = difflines.split('\n')


    # Build linemaps, each of which projects a filename to its all modified lines.
    # TODO: build old's linemap.

    class ParseState(Enum):
        FILEPATH    = 1
        DIFF        = 2

    linemap = dict()
    fname_map = dict()
    total_add = 0
    total_del = 0

    state = ParseState.FILEPATH
    cur_filepath = None
    cur_lineno = None
    cur_diffnos = { 'old': [], 'new': [] }

    def TestFuncNameChangeAndRegister(difflines, lidx):
        line = difflines[lidx]
        if (line[0] != '-'): return False
        if (len(line) >= 2 and line[1] == ' '): return False

        def FindFuncName(prefix):
            # Extend string downward.
            proto_re = None
            proto_str = ""

            off = 0
            while True:
                if (len(difflines) <= lidx + off or off > 8):
                    # A function prototype spanning more than 8 lines 
                    # is unlikely and it can be a corner case of this
                    # script not handling very well. Just give up.
                    proto_str = ""
                    break
                if (difflines[lidx + off][0] != prefix and
                        difflines[lidx + off][0] != ' '): 
                    off += 1
                    continue
                proto_str += difflines[lidx + off][1:]
                if (proto_str.find('{') != -1):
                    break
                off += 1

            # If we managed to extract a possible function prototype,
            # match it to a function prototype format and see if it fits.
            if (proto_str):
                proto_re = re.search("^[a-zA-Z0-9_ ]+[\*\s]+" +
                        "([a-zA-Z_][a-zA-Z0-9_]*)\(.*\)[ ]*{", proto_str)

            if (not proto_re):
                # We've still not encountered the first matching prototype.
                # Keep scanning.
                return '' 

            return proto_re.group(1)

        old_fname = FindFuncName('-')
        if (old_fname):
            new_fname = FindFuncName('+')
            if (new_fname):
                fname_map[old_fname] = new_fname


    for lidx in range(len(difflines)):
        line = difflines[lidx]
        if (state == ParseState.FILEPATH):
            if (len(line) >= 3 and line[0:3] == "+++"):
                new_filepath_re = re.search("\+\+\+ b/(.*)", line)
                if (new_filepath_re):
                    if (line[-2:] != ".c" and line[-2:] != ".h"): 
                        log.INFO("skipping non-C file '" +
                                new_filepath_re.group(1) + "'...")
                        continue
                    new_filepath = new_filepath_re.group(1)
                    cur_filepath = new_filepath
                    cur_diffnos = { 'old': [], 'new': [] }
                    state = ParseState.DIFF
                    log.INFO("collecting diffs in '" + cur_filepath + "'...")

        elif (state == ParseState.DIFF and len(line) >= 1):
            if (line[0] == '@'):
                header_re = re.search("@@ -([0-9]+)(,[0-9]+)? \+([0-9]+)(,[0-9]+)? @@", line)
                if (header_re):
                    cur_lineno = { 'old': int(header_re.group(1)), 'new': int(header_re.group(3)) }
            elif (line[0] == 'd'):
                linemap[cur_filepath] = cur_diffnos
                state = ParseState.FILEPATH
            else:
                if (line[0] == '+'):
                    cur_diffnos['new'] += [cur_lineno['new']]
                    cur_lineno['new'] += 1
                    total_add += 1
                elif (line[0] == '-'):
                    cur_diffnos['old'] += [cur_lineno['old']]
                    cur_lineno['old'] += 1
                    total_del += 1
                    TestFuncNameChangeAndRegister(difflines, lidx)
                elif (line[0] == ' '):
                    cur_lineno['new'] += 1
                    cur_lineno['old'] += 1

    if (cur_diffnos['old'] or cur_diffnos['new']):
        linemap[cur_filepath] = cur_diffnos


    # Extract modified info.

    def CreateDir(d):
        os.makedirs(d, exist_ok=True)


    def ExtractClosedScope(srclines, baseno):
        lines = []
        scope_started = False
        curly_count = 0
        off = 0

        in_comment = False
        in_preproc = False
        in_define = False
        define_contd = False
        while True:
            if (len(srclines) <= baseno + off):
                break

            srcline = srclines[baseno + off]
            lines += [srcline]

            # Ignore multi-line comment
            _find_com_begin = srcline.find("/*")
            _find_com_end = srcline.find("*/")

            in_comment_pending = False
            if (_find_com_begin != -1 and _find_com_end == -1):
                in_comment_pending = True
                #srcline = re.sub(r'/\*.*', '', srcline)
                srcline = srcline[:_find_com_begin]
            elif (_find_com_begin == -1 and _find_com_end != -1):
                if (not in_comment):
                    for l in srclines[max(0, baseno - 5):
                            min(len(srclines), baseno + 5)]:
                        print(l)
                    print(lines)
                    print(srclines[baseno])
                in_comment = False
                #srcline = re.sub(r'.*\*/', '', srcline)
                srcline = srcline[_find_com_end+2:]
            elif (_find_com_begin != -1 and _find_com_end != -1):
                #srcline = re.sub(r'/\*.*\*/', '', srcline)
                srcline = srcline[:_find_com_begin] + srcline[_find_com_end+2:]

            # Ignore inline comment
            srcline = re.sub(r'//.*', '', srcline)

            # Ignore preprocessor blocks
            srcline_strip = srcline.strip()
            if (srcline_strip[0:5] == "#elif" or srcline_strip[0:5] == "#else"):
                in_preproc = True
            elif (srcline_strip[0:6] == "#endif"):
                in_preproc = False

            if (srcline_strip[0:7] == "#define" or define_contd):
                in_define = True
                if (srcline_strip[-1:] == '\\'):
                    define_contd = True
                else:
                    define_contd = False
            else:
                in_define = False

            # Ignore string/char
            if (not in_comment and not in_preproc and not in_define):
                srcline = re.sub(r'".*"', '', srcline)
                srcline = re.sub(r"'.*'", '', srcline)

                curly_count += srcline.count('{')
                if (not scope_started and curly_count > 0):
                    scope_started = True

                curly_count -= srcline.count('}')
                if (scope_started and curly_count <= 0):
                    break
            off += 1

            if (in_comment_pending):
                in_comment = True

        return lines

    def ExtractCommonScopeInfo(basedir, filepath, baseno, srclines, linemap):
        # file: original file path.
        with open(basedir + "/file", 'w') as f:
            f.write(filepath)

        # offset: where this scope starts in the original file.
        with open(basedir + "/offset", 'w') as f:
            f.write(str(baseno+1))

        # body: scope body, including the header.
        body_linenos = None 
        with open(basedir + "/body", 'w') as f:
            body_lines = ExtractClosedScope(srclines, baseno)
            body_linenos = range(baseno + 1, baseno + 1 + len(body_lines))
            for body_line in body_lines:
                body_line_nouni = ''.join([i if ord(i) < 128 else ' ' for i in body_line])
                f.write(body_line_nouni + '\n')

        # line: modified lines in the 'body' above.
        with open(basedir + "/line", 'w') as f:
            new_linenos = list(filter(lambda x: x in linemap, body_linenos))
            new_linenos = [lineno - baseno for lineno in new_linenos]
            for lineno in new_linenos:
                f.write(str(lineno) + '\n')

    def TryExtractFunc(ver, func_cache, srclines, i):
        # This loop finds the first matching function prototype, and assumes
        # it's the function containing this diff line.
        # To do this, check if it's the first line of a function prototype
        # If it is, try extracting the entire function prototype string.
        proto_re = None
        boff = 0

        if (re.search(r'^\w', srclines[i]) and 
                srclines[i].find(':') == -1):
            proto_str = ""
            
            # Extend string downward.
            off = 0
            while True:
                if (len(srclines) <= i + off or off > 8):
                    # A function prototype spanning more than 8 lines 
                    # is unlikely and it can be a corner case of this
                    # script not handling very well. Just give up.
                    proto_str = ""
                    break
                proto_str += srclines[i + off] + ' '
                if (proto_str.find(';') != -1):
                    break
                if (proto_str.find('{') != -1):
                    break
                off += 1

            # If we managed to extract a possible function prototype,
            # match it to a function prototype format and see if it fits.
            # FIX: some prototypes separate a type from a title. Try
            # to include the type if possible.
            if (proto_str):
                while True:
                    proto_re = re.search("^[a-zA-Z0-9_ ]+[\*\s]+" +
                            "([a-zA-Z_][a-zA-Z0-9_]*)\s*\(.*\)\s*{", proto_str)
                    if (proto_re): break
                    else:
                        boff += 1
                        if (i - boff < 0): break
                        if (i - boff in func_cache): break
                        proto_str = srclines[i - boff] + ' ' + proto_str
                    if (boff > 2): break

        if (not proto_re):
            # We've still not encountered the first matching prototype.
            # Keep scanning.
            return False

        # If we really found a function prototype, extract necessary info.

        # Extract the body of the modified function, and add this newly identified
        # function to the cache.
        funcname = proto_re.group(1)
        if (ver == 'old' and funcname in fname_map.keys()):
            funcname = fname_map[funcname]
        func_dir = output_path + '/' + funcname + '/' + ver
        if (os.path.exists(func_dir)):
            funcname += '.' + str(total_funcs)
            func_dir = output_path + '/' + funcname + '/' + ver
        func_cache[i] = funcname
        CreateDir(func_dir)

        # Extract common scope info: file, offset, body, line
        ExtractCommonScopeInfo(func_dir, filepath, i - boff, srclines, linemap[filepath][ver])

        return True


    total_funcs = 0

    for filepath, diffnos_pair in linemap.items():
        def Extract(ver):
            nonlocal total_funcs
            line_cache = { 'func': dict() }
            
            _c = new_commit if (ver == 'new') else old_commit
            try:
                srclines = org_repo.git.show('{}:{}'.format(_c.hexsha, filepath))
            except:
                log.INFO("cannot find '" + ver + "' version.")
                return

            srclines = srclines.split('\n')

            log.INFO("scanning '" + filepath + "'... (" + ver + ")")

            # For each modified line, scan the source code in reverse and find out if
            # it's a part of a function.
            for diffno in diffnos_pair[ver]:
                # NOTE: if there's no diff lines (e.g., no
                # added/removed lines), don't extract the version.
                if (not diffno): continue
                # NOTE: ad-hoc patch. skip lines that we don't care,
                # mostly within a flag-filled header.
                if (srclines[diffno-1][0:2] == '//' or
                        srclines[diffno-1][0:2] == '/*' or
                        srclines[diffno-1][0:2] == '*/' or
                        srclines[diffno-1][0:2] == ' *' or
                        srclines[diffno-1][0:1] == '#'):
                    continue
                for i in reversed(range(0, diffno)):
                    # If the function cache already specifies a function for this line,
                    # we've already extracted information.
                    if (i in line_cache['func']):
                        break

                    # If we encountered a closing bracket as a first character, we've
                    # been the outside of any scope until now. Not a relavant diff.
                    if (len(srclines[i]) >= 1 and srclines[i][0] == '}'):
                        line_cache['func'][i] = "(top-level)"
                        break

                    if (TryExtractFunc(ver, line_cache['func'], srclines, i)): 
                        total_funcs += 1
                        break

        Extract('old')
        Extract('new')


    log.INFO("{} added line(s), {} removed line(s), {} updated function(s)."
            .format(total_add, total_del, total_funcs))
