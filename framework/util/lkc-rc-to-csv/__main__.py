#/usr/bin/python

import argparse
import os, sys
import json


parser = argparse.ArgumentParser()
parser.add_argument('-j', '--json-path', type=str, help='json file path.', default='kernel_cves.json')
parser.add_argument('-o', '--out-path', type=str, help='out file path.', default='out.csv')
parser.add_argument('-y', '--years', type=str, help='comma-separated years to include', default='2020,2021')
args = parser.parse_args()


def main():
    with open(args.json_path, "r") as f:
        json_data = json.load(f)

    if (args.years):
        years_to_include = args.years.split(',')
        def _FromTargetYears(cve_name):
            nonlocal years_to_include
            for y in years_to_include:
                if (cve_name.find('-' + str(y) + '-') != -1):
                    return True
            return False
        json_data = dict(filter(lambda e: _FromTargetYears(e[0]), json_data.items()))

    with open(args.out_path, "w") as f:
        f.write('Type,Bug Kind,CVE Name,Root-cause Hexsha\n')
        for cve_name, cve_dets in json_data.items():
            if (not 'breaks' in cve_dets.keys() or not cve_dets['breaks']):
                continue
            breaks_str = cve_dets['breaks']

            regression_str = 'R'
            if (breaks_str == '1da177e4c3f41524e886b7f1b8a0c1fc7321cac2'):
                regression_str = 'N'
            
            kws_to_preserve = {
                'Use After Free': ['Use After Free', 'Use-after-free', 'UAF'],
                'Buffer Overflow': ['Buffer Overflow', 
                    'Out-of-bounds Read', 'Out-of-bounds Write',
                    'Off-by-one'],
                'NULL Pointer Dereference': ['NULL Pointer Dereference'],
                'Double Free': ['Double Free'],
                'Deadlock': ['Deadlock'],
                'Infinite Loop': ['Infinite Loop']
                }

            def _FindFirstMatchingBugKind(nvdt):
                nonlocal kws_to_preserve
                for bk, kws in kws_to_preserve.items():
                    for kw in kws:
                        if (kw.casefold() in nvdt.casefold()):
                            return bk
                return ''

            def _ToUnifiedCWEName(cwe):
                nonlocal kws_to_preserve
                for bk, kws in kws_to_preserve.items():
                    for kw in kws:
                        if (kw == cwe):
                            return bk
                return ''

            bugkind_str = 'Semantic (Unknown)'
            if ('cwe' in cve_dets.keys() and _ToUnifiedCWEName(cve_dets['cwe'])):
                bugkind_str = _ToUnifiedCWEName(cve_dets['cwe']) 
            else:
                # Check 1: race condition?
                if (('cwe' in cve_dets.keys() and
                    'race condition' in cve_dets['cwe'].casefold()) or
                    ('nvd_text' in cve_dets.keys() and
                     'race condition' in cve_dets['nvd_text'].casefold())):
                    bugkind_str = 'Race Condition'

                # Check 2: memory leak?
                elif (('nvd_text' in cve_dets.keys() and
                     'memory leak' in cve_dets['nvd_text'].casefold()) or
                    ('cmt_msg' in cve_dets.keys() and
                     'memory leak' in cve_dets['cmt_msg'].casefold())):
                    bugkind_str = 'Memory Leak'

                # Check 3: general crash and panic.
                elif ('nvd_text' in cve_dets.keys() and
                    ('crash' in cve_dets['nvd_text'].casefold() or
                    'panic' in cve_dets['nvd_text'].casefold())):
                    bugkind_str = 'Kernel Panic'

                # Check 4: bug kind hidden in nvd text?
                elif ('nvd_text' in cve_dets.keys() and
                    _FindFirstMatchingBugKind(cve_dets['nvd_text'])):
                    bugkind_str = _FindFirstMatchingBugKind(cve_dets['nvd_text'])

                # Check 5: bug kind hidden in cwe?
                elif ('cwe' in cve_dets.keys() and
                    _FindFirstMatchingBugKind(cve_dets['cwe'])):
                    bugkind_str = _FindFirstMatchingBugKind(cve_dets['cwe'])

                elif ('cwe' in cve_dets.keys()):
                    bugkind_str = 'Semantic (' + cve_dets['cwe'] + ')'

            f.write(regression_str + ',' + bugkind_str + ',' + cve_name + ',' + breaks_str + '\n')

if __name__ == "__main__":
    main()
