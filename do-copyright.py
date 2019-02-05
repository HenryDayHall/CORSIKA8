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
excludeFiles = ['PhysicalConstants.h','CorsikaFenvOSX.cc']

extensions = [".cc", ".h", ".test"]

def checkNote(filename):

    startNote = []
    endNote = []

    # read input file into lines
    lines = []
    with open(filename, "r") as file:
        for line in file.readlines():
            lines.append(line)            
        file.close()

    searchStatus = 0 # 0:before comment block, #1 in comment block, #2 found copyright
    blockStart = 0
    for iLine in range(len(lines)):
        line = lines[iLine]
        if "/*" in line and searchStatus==0:
            searchStatus = 1
            blockStart = iLine
        if "copyright" in line.lower() and searchStatus>0:
            searchStatus = 2
        if "*/" in line:
            if searchStatus>=2:
                startNote.append(blockStart)
                endNote.append(iLine)
            searchStatus = 0
        iLine += 1
        
    # now check if first copyright notices is already identical...
    isSame = False
    if len(startNote)>0: 
        isSame = True
        noteLines = text.split('\n')        
        for iLine in range(len(noteLines)-2):
            if startNote[0]+iLine >= len(lines):
                isSame = False
                break
            if noteLines[iLine+1].strip(" \n") != lines[startNote[0]+iLine].strip(" \n"):
                isSame = False
                print "need update: " + filename + " new=\'" + noteLines[iLine+1] + "\' vs old=\'" + lines[startNote+iLine].rstrip('\n') + "\'"
                break
    
    # check if notice is the same, or we need to remove multiple notices...
    if isSame and len(startNote)<=1:
        return                
    
    # add (new) copyright notice here:
    print ("File: " + filename + ", make copy to " + filename+".bak")
    os.rename(filename, filename+".bak")

    with open(filename, "w") as file:

        file.write(text)

        skip = False
        for iLine in range(len(lines)):

            inBlock = False
            for iBlock in range(len(startNote)):
                if iLine>=startNote[iBlock] and iLine<=endNote[iBlock]:
                    print "   [remove " + str(iBlock) + "] " + (lines[iLine]).strip()
                    inBlock = True
                    skip = True

            if inBlock:
                continue
            
            if lines[iLine].strip() != "": # if line after comment is empty, also remove it
                skip = False
                    
            if not skip:
                file.write(lines[iLine])
        
        file.close()


def next_file(x, dir_name, files):
    for check in excludeDirs :
        if check in dir_name:
            return
    for check in files :
        filename, file_extension = os.path.splitext(check)
        if '#' in check or '~' in check:
            return
        for check2 in excludeFiles :
            if check2 in check:
                return
        if file_extension in extensions:
            checkNote(dir_name + "/" + check)


os.path.walk("./", next_file, 0)
