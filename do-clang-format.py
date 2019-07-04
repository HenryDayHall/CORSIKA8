#!/usr/bin/env python3
"""
Run clang-format with the style file in the CORSIKA repository.

By default it finds new files and files with modifications with respect to the current master and prints the filenames which need clang-formatting. Returns 1 if there are files which need modifications and 0 otherwise, so it can be used as a test.
"""
import argparse
import subprocess as subp
import os
import sys

parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--apply', action="store_true",
    help="Apply clang-format to files which need changes.")
parser.add_argument("--all", action="store_true",
    help="Check all files below current path instead of new/modified.")

args = parser.parse_args()

filelist = []
if args.all:
    for dirpath, dirnames, filenames in os.walk("."):
        if "ThirdParty" in dirpath:
            continue
        for f in filenames:
            if f.endswith(".h") or f.endswith(".cc"):
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
        filelist.append(line[1:].lstrip())

    cmd = "git ls-files --exclude-standard --others"
    filelist += subp.check_output(cmd, shell=True).decode("utf8").strip().split("\n")
    filelist = [x for x in filelist
                if "ThirdParty" not in x and (x.endswith(".h") or x.endswith(".cc"))]

cmd = "clang-format -style=file"
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
