#!/usr/bin/python

import os.path

text = """
/*
 * (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
 *
 * See file AUTHORS for a list of contributors.
 *
 * This software is distributed under the terms of the GNU General Public
 * Licence version 3 (GPL Version 3). See file LICENSE for a full version of
 * the license.
 */\n
"""

excludeDirs = ["ThirdParty", "git"]
excludeFiles = ['PhysicalConstants.h']

extensions = [".cc", ".h", ".test"]

def checkNote(filename):

    startNote = -1
    endNote = -1
    isCopyright = False
    lines = []

    with open(filename, "r") as file:
        for line in file.readlines():
            lines.append(line)            
        file.close()

            
    for iLine in range(len(lines)):
        line = lines[iLine]
        if "/**" in line and startNote == -1:
            startNote = iLine
        if "copyright" in line.lower() and startNote>=0 and endNote==-1:
            isCopyright = True
        if "*/" in line and startNote>=0 and endNote==-1:
            endNote = iLine
        iLine += 1

    # now check if copyright notice is already there and identical...
    isSame = False
    if startNote>=0 and endNote>=0 and isCopyright:
        isSame = True
        noteLines = text.split('\n')        
        for iLine in range(len(noteLines)-2):
            if startNote+iLine >= len(lines):
                isSame = False
                break            
            if noteLines[iLine+1].strip(" \n") != lines[startNote+iLine].strip(" \n"):
                isSame = False
                print "not same: " + filename + " new=\'" + noteLines[iLine+1] + "\' vs old=\'" + lines[startNote+iLine].rstrip('\n') + "\'"
                break

    # check if notice is the same 
    if isSame:
        return                
    
    # add (new) copyright notice here:
        
    os.rename(filename, filename+".bak")

    with open(filename, "w") as file:

        file.write(text)

        firstLine = 0
        if startNote>=0 and endNote>=0 and isCopyright:
            firstLine = endNote + 2

        for iLine in range(firstLine, len(lines)):
            file.write(lines[iLine])
        
        file.close()


def next_file(x, dir_name, files):
    for check in excludeDirs :
        if check in dir_name:
            return
    for check in files :
        filename, file_extension = os.path.splitext(check)
        for check2 in excludeFiles :
            if check2 in check:
                return
        if file_extension in extensions:
            checkNote(dir_name + "/" + check)


os.path.walk("./", next_file, 0)
