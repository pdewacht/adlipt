#!/usr/bin/python3
import glob
import os
import subprocess
import tempfile
import yaml
import sys

def run_nasm(source, defines=None):
    with tempfile.TemporaryDirectory() as d:
        asm_name = os.path.join(d, 'code.asm')
        bin_name = os.path.join(d, 'code.bin')
        with open(asm_name, 'wt') as asm_file:
            asm_file.write(source)
        d_opts = ['-D%s=%s' % d for d in (defines or {}).items()]
        subprocess.check_call(['nasm', '-Iasm/', *d_opts, '-o', bin_name, asm_name])
        with open(bin_name, 'rb') as bin_file:
            return bin_file.read()

def compile_pattern(source):
    defines = {
        'BEGIN': 'db 0x01',
        'REPEAT': 'db 0x02',
        'REPEAT0': 'db 0x03',
        'OPTION': 'db 0x04',
        'END': 'db 0x05',
        'ALT': 'db 0x06',
        'ANYBYTE': 0xF0,
        'ANYWORD': 0xF0F0,
        'ANYDWORD': 0xF0F0F0F0,
    }
    anti_defines = {
        'BEGIN': 'db 0x00',
        'REPEAT': 'db 0x00',
        'REPEAT0': 'db 0x00',
        'OPTION': 'db 0x00',
        'END': 'db 0x00',
        'ALT': 'db 0x00',
        'ANYBYTE': 0xFF,
        'ANYWORD': 0xFFFF,
        'ANYDWORD': 0xFFFFFFFF,
    }
    code1 = run_nasm(source, defines)
    code2 = run_nasm(source, anti_defines)
    return '.'.join(
        '0x%02X' % a if a == b else
        '(zlen'  if a == 1 else
        'zlen)+' if a == 2 else
        'zlen)*' if a == 3 else
        'zlen)?' if a == 4 else
        'zlen)'  if a == 5 else
        'zlen|zlen' if a == 6 else
        'any'    if a == 0xF0 else
        'error'
        for a, b in zip(code1, code2))

def compile_replacement(source, port=0x123):
    defines = {'PORT': 0x0000}
    anti_defines = {'PORT': 0xFFFF}
    code1 = run_nasm(source, defines)
    code2 = run_nasm(source, anti_defines)
    port_idx = next((idx for idx, val
                     in enumerate(zip(code1, code2))
                     if val == (0x00, 0xFF)),
                    -1)
    return code1, port_idx

def c_string(s):
    return '"' + s.replace('\\', '\\\\').replace('"', '\\"') + '"'

def c_bytes(s):
    return '"' + ''.join('\\%03o' % ch for ch in s) + '"'

print("""
%%{
machine AdlibScanner;
include x86 "x86.rl";
main := |*
""")

for f in sorted(glob.glob('patterns/*.yaml')):
    for doc in yaml.safe_load_all(open(f)):
        if 'name' in doc:
            name = doc['name']
        else:
            print('%s: missing name' % f, file=sys.stderr)
            continue

        if 'ragel' in doc:
            pattern = doc['ragel']
        elif 'find' in doc:
            pattern = compile_pattern(doc['find'])
        else:
            print('%s: %s: missing pattern' % (f, name), file=sys.stderr)
            continue

        action = doc.get('hack', '')
        if 'warn' in doc:
            action += '\n  printf("Warning: %%s\\n", %s);' % c_string(doc['warn'])
        if 'replace' in doc:
            code, port_idx = compile_replacement(doc['replace'])
            action += '\n  if (!patch(%s, ts, te, %s, %s, %s)) { return false; }' % (
                c_string(name),
                c_bytes(code),
                len(code),
                port_idx)
        if not action:
            printf('%s: %s: missing action' % (f, name), file=sys.stderr)
            continue

        print("%s => { %s };\n" % (pattern, action))

print("""any;
*|;
}%%
""")
