#!/usr/bin/python3
import struct
import sys

def pepatch(filename):
    with open(filename, 'r+b') as f:
        if f.read(2) != b'MZ':
            raise Exception("Not an EXE file: '%s'" % filename)

        f.seek(0x3C)
        pe_offset, = struct.unpack('<L', f.read(4))

        f.seek(pe_offset)
        sig = f.read(2)
        if sig == b'PX':
            raise Exception("Already patched: '%s'" % filename)
        if sig != b'PE':
            raise Exception("Not a PE EXE file: '%s'" % filename)

        f.seek(pe_offset)
        f.write(b'PX')


if __name__ == '__main__':
    if len(sys.argv) != 2:
        print('Usage: pepatch FILE', file=sys.stderr)
        sys.exit(1)
    try:
        pepatch(sys.argv[1])
    except Exception as e:
        print('pepatch: %s' % e, file=sys.stderr)
        sys.exit(1)
