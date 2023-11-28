#!/usr/bin/python

import os, sys, re

commits = next(os.walk('.'))[1]
for commit in commits:

    funcs = next(os.walk('./{}'.format(commit)))[1]
    for func in funcs:

        for ver in ['old', 'new']:
            if (not os.path.exists('./{}/{}/{}'.format(
                    commit, func, ver))):
                continue

            with open('./{}/{}/{}/body'.format(commit, func, ver), 'r') as f:
                lines = f.readlines()

            # Extract function attributes.
            attrs = []
            for i in range(len(lines)):
                attr_str = r'__\w+'
                attrs += re.findall(attr_str, lines[i])
                lines[i] = re.sub(attr_str, '', lines[i])

                # NOTE: we drop the arguments of "__printf" cause it
                # unnecessarily complicates attribute handling in Joern.
                printf_attr_str = r'__printf([\w,\s]*)'
                if (re.search(printf_attr_str, lines[i])):
                    attrs += ['__printf' ]
                lines[i] = re.sub(printf_attr_str, '', lines[i])

                if (lines[i].find('{') != -1):
                    break

            # Reserve the original function body as ".body.org".
            os.rename('./{}/{}/{}/body'.format(commit, func, ver),
                    './{}/{}/{}/.body.org'.format(commit, func, ver))

            
            # Rewrite the body.
            with open('./{}/{}/{}/body'.format(commit, func, ver), 'w') as f:
                # NOTE: we deliberately don't discern multiple copies
                # of functions in a commit.
                info_def = "const char *{}__".format(
                        func.split('.')[0]) + "{} = {};"
                
                # Info 1) commit hexsha
                f.write(info_def.format('HEXSHA', 
                    '"' + commit + '"') + '\n')

                # Info 2) (space-separated) attributes
                f.write(info_def.format('ATTRS',
                    '"' + ' '.join(attrs) + '"') + '\n')

                f.write('\n')
                f.writelines(lines)
