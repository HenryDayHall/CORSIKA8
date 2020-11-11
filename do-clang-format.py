#!/usr/bin/env python3
"""
Run clang-format with the style file in the CORSIKA repository.

By default it finds new files and files with modifications with respect to the current master and prints the filenames which need clang-formatting. Returns 1 if there are files which need modifications and 0 otherwise, so it can be used as a test.
"""
import argparse
import subprocess as subp
import os
import sys
import re

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--apply', action="store_true",
    help="Apply clang-format to files which need changes.")
parser.add_argument("--all", action="store_true",
    help="Check all files below current path instead of new/modified.")

args = parser.parse_args()

excludeDirs = [r"^(\./)?modules/", r"^(\./)?externals/", r"^(\./)?build", r"^(\./)?install", r"(\./)?\.git"]

filelist = []
if args.all:
    for dirpath, dirnames, filenames in os.walk("."):
        excl = False
        for excl_dir in excludeDirs:
            if re.findall(excl_dir, dirpath):
                excl = True
                break
        if excl:
            continue
        for f in filenames:
            if (f.endswith(".hpp") or f.endswith(".cpp") or f.endswith(".inl")):
                filename = os.path.join(dirpath, f)
                if not os.path.islink(filename):
                    filelist.append(filename)
    if not filelist:
        raise SystemExit("Error: You specified --all, but file list is empty. "
                         "Did you run from the build directory?")
else:
    cmd = "git diff master --name-status"
    for line in subp.check_output(cmd, shell=True).decode("utf8").strip().split("\n"):
        if line.startswith("D"): continue
        if line.startswith("R"):
            filelist.append(line.split()[-1])
        else:
            filelist.append(line[1:].lstrip())

    cmd = "git ls-files --exclude-standard --others"
    filelist2 = subp.check_output(cmd, shell=True).decode("utf8").strip().split("\n")
    filelist += filelist2
    # some cleanup
    filelist_clean = []
    for f in filelist:
        if not (f.endswith(".hpp") or f.endswith(".cpp") or f.endswith(".inl")):
            continue
        if os.path.islink(f):
            continue
        excl = False
        for excl_dir in excludeDirs:
            if re.findall(excl_dir, f):
                excl = True
                break
        if excl:
            continue
        filelist_clean.append(f)
    filelist = filelist_clean

cmd = "clang-format"
if "CLANG_FORMAT" in os.environ:
  cmd = os.environ["CLANG_FORMAT"]
cmd +=  " -style=file"
if args.apply:
    for filename in filelist:        
        subp.check_call(cmd.split() + ["-i", filename])

else:
    # only print files which need formatting
    files_need_formatting = 0
    for filename in filelist:
        a = open(filename, "rb").read()
        b = subp.check_output(cmd.split() + [filename])
        if a != b:
            files_need_formatting += 1
            print(filename)
    sys.exit(1 if files_need_formatting > 0 else 0)
