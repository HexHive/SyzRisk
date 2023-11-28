#!/usr/bin/python

import re
import requests
import git

SYZBOT_TAG_ONLY = False     # Only take Syzbot-reported bugs 
SYZBOT_TAG_NULL = False     # Ignore if it's from Syzbot

def CommitFilter(commit):
    msg = commit.message
    vulnType = []

    # Find a "Fixes" tag.
    fixes_re = re.search('Fixes:\s*(\w+)', msg, re.IGNORECASE)
    if (not fixes_re): return (None, None)
    fixes_hash = fixes_re.group(1)

    # Check if it's for fixing a MEMORY bug.
    def check_msg_keywords():
        retlist = []

        # Cut out flags from the message body.
        _msg = msg
        flag_re = re.search('\n[A-Z-]\w+: ', msg)
        if (flag_re):
            _msg = msg[:flag_re.span()[0]]
        _msg = re.sub(r'"[^"]*"', '', _msg)

        warning_re = re.search('(warning|potential)', _msg, re.IGNORECASE)

        if (not SYZBOT_TAG_ONLY):
            if (not warning_re and
                (re.search('use.+after.+free', _msg, re.IGNORECASE) \
                or re.search('double.+free', _msg, re.IGNORECASE) \
                or re.search('uaf', _msg, re.IGNORECASE))):
                retlist += ["use-after-free"]

            if (not warning_re and
                (re.search('stack.+overflow', _msg, re.IGNORECASE) \
                or re.search('buffer.+overflow', _msg, re.IGNORECASE) \
                or re.search('heap.+overflow', _msg, re.IGNORECASE) \
                or re.search('global.+overflow', _msg, re.IGNORECASE) \
                or re.search('vmalloc.+overflow', _msg, re.IGNORECASE) \
                or re.search('slab.+overflow', _msg, re.IGNORECASE) \
                or re.search('stack.+overrun', _msg, re.IGNORECASE) \
                or re.search('buffer.+overrun', _msg, re.IGNORECASE) \
                or re.search('heap.+overrun', _msg, re.IGNORECASE) \
                or re.search('global.+overrun', _msg, re.IGNORECASE) \
                or re.search('vmalloc.+overrun', _msg, re.IGNORECASE) \
                or re.search('slab.+overrun', _msg, re.IGNORECASE) \
                or re.search('off.+by.+one', _msg, re.IGNORECASE) \
                or re.search('out.+of.+bound', _msg, re.IGNORECASE))):
                retlist += ["buffer-overflow"]
            
            if (re.search('deadlock', _msg, re.IGNORECASE) \
                or re.search('dead lock', _msg, re.IGNORECASE)):
                retlist += ["deadlock"]
        
        if (re.search('(thread|core|cpu)1', _msg, re.IGNORECASE) \
            and re.search('(thread|core|cpu)2', _msg, re.IGNORECASE)):
            if (re.search('dead[-\s]?lock', _msg, re.IGNORECASE)):
                retlist += ["deadlock"]
            elif (re.search(' race', _msg, re.IGNORECASE)):
                retlist += ["race-condition"]

        # TODO: may have false positives when we use "race", 
        #       beacuse regex also detects 'trace' as 'race'.
        #or re.search(' race', _msg, re.IGNORECASE)):
        if (re.search('(fix|prevent|mitigate)', _msg, re.IGNORECASE)
            and re.search('[^\w]race[^\w]+condition', _msg, re.IGNORECASE)):
            retlist += ["race-condition"]

        if (not SYZBOT_TAG_ONLY):
            if (not warning_re and 
                (re.search('memory.+leak', _msg, re.IGNORECASE) \
                or re.search('leak.+memory', _msg, re.IGNORECASE))):
                retlist += ["memory-leak"]

            if (not warning_re and
                (re.search('null.+deref', _msg, re.IGNORECASE) \
                or re.search('deref.+null', _msg, re.IGNORECASE))):
                retlist += ["NULL-dereference"]

            if (not retlist):
                if (not warning_re and
                    (re.search('crash', _msg, re.IGNORECASE) \
                    or re.search('panic', _msg, re.IGNORECASE))):
                    retlist += ["kernel-panic"]

        return retlist

    vulnType += check_msg_keywords()

    # Attempt 2: inspect the syzbot report title.
    def check_syzbot_report():
        syzrep_re = re.search('Reported-(and-tested-)?by:\s*syzbot\+([0-9a-f]+)', msg, re.IGNORECASE)
        if (not syzrep_re): return ""

        syz_hash = syzrep_re.group(2)
        syz_url = "https://syzkaller.appspot.com/bug?extid=" + syz_hash

        try:
            syzrep_txt = requests.get(syz_url).text
        except:
            return []

        syz_title_re = re.search("<title>(.*?)</title>", syzrep_txt, re.IGNORECASE|re.DOTALL)
        if (not syz_title_re): return []
        syz_title = syz_title_re.group(1)
        
        bugtype_re = re.search('^KASAN: ([\w-]+)', syz_title)
        if (bugtype_re):
            if (bugtype_re.group(1) == 'use-after-free' \
                or bugtype_re.group(1) == 'use-after-scope' \
                or bugtype_re.group(1) == 'double-free'):
                return ["use-after-free"]
            elif (re.search('out-of-bounds', bugtype_re.group(1))):
                return ["buffer-overflow"]
            elif (bugtype_re.group(1) == 'null-ptr-deref'):
                return ["NULL-dereference"]

        bugtype_re = re.search('^KCSAN:', syz_title)
        if (bugtype_re):
            return ["race-condition"]

        bugtype_re = re.search('^general protection fault', syz_title)
        if (bugtype_re):
            return ["NULL-dereference"]

        bugtype_re = re.search('^BUG: kernel NULL', syz_title)
        if (bugtype_re):
            return ["NULL-dereference"]

        bugtype_re = re.search('^memory leak', syz_title)
        if (bugtype_re):
            return ["memory-leak"]

        bugtype_re = re.search('^WARNING: .*lock', syz_title)
        if (bugtype_re):
            return ["deadlock"]

        bugtype_re = re.search('^BUG: .*(lock|sleep)?', syz_title)
        if (bugtype_re):
            if (bugtype_re.group(1)):
                return ["deadlock"]
            else:
                return ["kernel-panic"]

        bugtype_re = re.search('^kernel BUG', syz_title)
        if (bugtype_re):
            return ["kernel-panic"]

        return []

    vulnTypeSyz = check_syzbot_report()
    if (SYZBOT_TAG_NULL and vulnTypeSyz):
        vulnType = []
    else:
        vulnType += vulnTypeSyz
    vulnType = list(set(vulnType))

    return (fixes_hash, vulnType)
