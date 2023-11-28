import os, sys, re

def CanonFuncBody(lines, funcname=""):
    # Extract function attributes.
    attrs = []
    paren_end = False
    for i in range(len(lines)):
        # FIXME: currently not distinguishing attributes for the
        # function itself and parameters.
        attr_str = r'(__init|__exit|__user|__unused|' +\
                r'__maybe_unused|__always_unused|__force|' +\
                r'__rcu|__read_mostly|__write_motely|__net_exit|' +\
                r'__net_init|__per_cpu|__must_check|' +\
                r'__kprobes|__ref|__always_inline|__iomem|' +\
                r'__cold|__pure|__visible|__latent_entropy|' +\
                r'__no_sanitize_address|__weak|' +\
                r'__no_k.san|__noreturn|__irq_entry|__meminit|' +\
                r'__cpuidle|__sched|__hyp_text|__head|' +\
                r'__init_or_module|__percpu|__malloc|' +\
                r'__nostackprotector|__init_memblock|' +\
                r'__noinline|ICE_noinline|__u64|__xipram|' +\
                r'__sum16|__used|__hot|__softirq_entry|__naked|' +\
                r'__attribute_const__|__exception_irq_entry|' +\
                r'__no_fgces|__no_k.san_or_inline|' +\
                r'__deprecated|' +\
                r'__attribute__\s*\(\(.*\)\))(?=[\s,)])'
        attrs += re.findall(attr_str, lines[i])
        lines[i] = re.sub(attr_str, '', lines[i])

        if (funcname):
            # Find the function name. We need to adjust it to the newer
            # (and unified) one if the newer version changed the name.
            name_re = re.search("([a-zA-Z_][a-zA-Z0-9_]*)\(", lines[i])
            if (name_re and name_re.group(1) != funcname):
                lines[i] = re.sub(name_re.group(1), funcname, lines[i])
        
        # Find any call-like attributes. 
        paren_end_off = -1
        if (not paren_end): 
            paren_end_off = lines[i].find(')')
        if (lines[i].find(')') != -1):
            paren_end = True

        if (paren_end):
            call_str = "(\w+)\([^)]*\)"
            _line = lines[i]
            if (paren_end_off != -1):
                _line = _line[paren_end_off:]
            attrs += re.findall(call_str, _line)
            if (paren_end_off != -1):
                lines[i] = lines[i][0:paren_end_off] + \
                        re.sub(call_str, '', _line)
            else:
                lines[i] = re.sub(call_str, '', _line)

        # Find any known no-underscore attrs.
        attr_str = r'(noinstr|noinline|asmlinkage|' +\
                r'nokprobe_inline|notrace|INIT|' +\
                r'PRINTF|INLINE|INLINING|' +\
                r'noinline_for_stack)(?=[\s,)])'
        attrs += re.findall(attr_str, lines[i])
        lines[i] = re.sub(attr_str, '', lines[i])

        # Some weird defined keywords. Just throw them away.
        weird_str = r'(STORAGE_CLASS_\w+_C|' +\
                r'HOST_WIDE_INT)(?=[\s,)])'
        lines[i] = re.sub(weird_str, '', lines[i])

        lines[i] = re.sub('STATIC', 'static', lines[i])

        if (lines[i].find('{') != -1):
            break

    # Remove the conditional preprocessor.
    # NOTE: currently just take the preceding code.
    _ignore_line = False
    lines_new = []
    for line in lines:
        line_strip = line.strip()
        if (line_strip[0:3] == '#if'):
            lines_new += ['\n']
            continue
        elif (line_strip[0:5] == '#elif' or line_strip[0:5] == '#else'):
            _ignore_line = True
            lines_new += ['\n']
            continue
        elif (line_strip[0:6] == '#endif'):
            _ignore_line = False
            lines_new += ['\n']
            continue
        
        if (not _ignore_line):
            lines_new += [line]
        else:
            lines_new += ['\n']

    return (lines_new, attrs)
