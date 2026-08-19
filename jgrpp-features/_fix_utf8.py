#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# Fix files written by the Write/Edit tools on this Windows machine.
# Those tools write Chinese text as GBK bytes; this script re-encodes to UTF-8
# and normalises line endings to LF. If the file is already valid UTF-8 it is
# left untouched (unless --force is given).
import sys, os

def looks_like_mojibake(text):
    # GBK-decoded Chinese re-encoded to UTF-8 produces sequences that fail
    # strict UTF-8 validation in most cases; the try/except below handles it.
    return True  # placeholder, actual check is the decode attempt

def fix(path, force=False):
    raw = open(path, 'rb').read()
    # Try strict UTF-8 first
    try:
        text = raw.decode('utf-8')
        if not force:
            print(f'OK  (already utf-8): {path}')
            return False
    except UnicodeDecodeError:
        # Try GBK
        try:
            text = raw.decode('gbk')
        except UnicodeDecodeError as e:
            print(f'ERR (neither utf-8 nor gbk): {path}: {e}')
            return False
        text = text.replace('\r\n', '\n')
        with open(path, 'w', encoding='utf-8', newline='\n') as f:
            f.write(text)
        print(f'FIX (gbk->utf8, lf): {path}')
        return True
    return False

if __name__ == '__main__':
    changed = 0
    for arg in sys.argv[1:]:
        if arg == '--force':
            continue
        if fix(arg, '--force' in sys.argv):
            changed += 1
    print(f'{changed} file(s) fixed')
