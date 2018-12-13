#!/usr/bin/env python3

# (c) Copyright 2018 CORSIKA Project, corsika-project@lists.kit.edu
#
# See file AUTHORS for a list of contributors.
#
# This software is distributed under the terms of the GNU General Public
# Licence version 3 (GPL Version 3). See file LICENSE for a full version of
# the license.


import pickle, sys, itertools



# loads the pickled pythia_db (which is an OrderedDict)
def load_pythiadb(filename):
    with open(filename, "rb") as f:
        pythia_db = pickle.load(f)
    return pythia_db



# 
def read_sibyll_codes(filename, pythia_db):
    with open(filename) as f:
        for line in f:
            line = line.strip()
            if line[0] == '#':
                continue            
            identifier, sib_code, canInteractFlag, xsType = line.split()
            try:
                pythia_db[identifier]["sibyll_code"] = int(sib_code)
                pythia_db[identifier]["sibyll_canInteract"] = int(canInteractFlag)
                pythia_db[identifier]["sibyll_xsType"] = int(xsType)
            except KeyError as e:
                raise Exception("Identifier '{:s}' not found in pythia_db".format(identifier))


            

# generates the enum to access sibyll particles by readable names
def generate_sibyll_enum(pythia_db):
    output = "enum class SibyllCode : int8_t {\n"
    for identifier, pData in pythia_db.items():
        if pData.get('sibyll_code') != None:
            output += "  {:s} = {:d},\n".format(identifier, pData['sibyll_code'])
    output += "};\n"
    return output



# generates the look-up table to convert corsika codes to sibyll codes
def generate_corsika2sibyll(pythia_db):    
    string = "std::array<SibyllCodeIntType, {:d}> constexpr corsika2sibyll = {{\n".format(len(pythia_db))
    for identifier, pData in pythia_db.items():
        sibCode = pData.get("sibyll_code", 0)
        string += "  {:d}, // {:s}\n".format(sibCode, identifier if sibCode else identifier + " (not implemented in SIBYLL)")
    string += "};\n"
    return string
    


# generates the look-up table to convert corsika codes to sibyll codes
def generate_corsika2sibyll_xsType(pythia_db):    
    string = "std::array<int, {:d}> constexpr corsika2sibyllXStype = {{\n".format(len(pythia_db))
    for identifier, pData in pythia_db.items():
        sibCodeXS = pData.get("sibyll_xsType", -1)
        string += "  {:d}, // {:s}\n".format(sibCodeXS, identifier if sibCodeXS else identifier + " (not implemented in SIBYLL)")
    string += "};\n"
    return string


# generates the look-up table to convert sibyll codes to corsika codes    
def generate_sibyll2corsika(pythia_db) :
    string = ""
    
    minID = 0
    for identifier, pData in pythia_db.items() :
        if 'sibyll_code' in pData:
            minID = min(minID, pData['sibyll_code'])

    string += "SibyllCodeIntType constexpr minSibyll = {:d};\n\n".format(minID)

    pDict = {}
    for identifier, pData in pythia_db.items() :
        if 'sibyll_code' in pData:
            sib_code = pData['sibyll_code'] - minID
            pDict[sib_code] = identifier
    
    nPart = max(pDict.keys()) - min(pDict.keys()) + 1
    string += "std::array<corsika::particles::Code, {:d}> constexpr sibyll2corsika = {{\n".format(nPart)
    
    for iPart in range(nPart) :
        if iPart in pDict:
            identifier = pDict[iPart]
        else:
            identifier = "Unknown"
        string += "  corsika::particles::Code::{:s}, \n".format(identifier)
    
    string += "};\n"
    return string



# generates the bitset for the flag whether Sibyll knows the particle
def generate_known_particle(pythia_db):
    num_particles = len(pythia_db)
    num_bytes = num_particles // 32 + 1
    string = "Bitset2::bitset2<{:d}> constexpr isKnown{{ std::array<uint32_t, {:d}>{{{{\n".format(num_particles, num_bytes)
    
    numeric = 0
    for identifier, pData in reversed(pythia_db.items()):
        handledBySibyll = int("sibyll_code" in pData) & 0x1
        numeric = (numeric << 1) | handledBySibyll
    
    while numeric != 0:
        low = numeric & 0xFFFFFFFF
        numeric = numeric >> 32
        string += "  0x{:0x},\n".format(low)
        
    string += "}}};\n"
    return string



# generates the bitset for the flag whether Sibyll can use particle as projectile
def generate_interacting_particle(pythia_db):
    num_particles = len(pythia_db)
    num_bytes = num_particles // 32 + 1
    string = "Bitset2::bitset2<{:d}> constexpr canInteract{{ std::array<uint32_t, {:d}>{{{{\n".format(num_particles, num_bytes)
    #string = "std::array<bool, {:d}> constexpr corsika2sibyll = {{\n".format(num_particles)
    
    numeric = 0    
    for identifier, pData in reversed(pythia_db.items()):
        can = 0
        if 'sibyll_canInteract' in pData:
            if pData['sibyll_canInteract'] > 0:
                can = 0x1
        numeric = (numeric << 1) | can
    
    while numeric != 0:
        low = numeric & 0xFFFFFFFF
        numeric = numeric >> 32
        string += "  0x{:0x},\n".format(low)
    
    string += "}}};\n"
    return string
    


if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("usage: {:s} <pythia_db.pkl> <sibyll_codes.dat>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)
        
    print("code_generator.py for SIBYLL")
    
    pythia_db = load_pythiadb(sys.argv[1])
    read_sibyll_codes(sys.argv[2], pythia_db)
    
    with open("Generated.inc", "w") as f:
        print("// this file is automatically generated\n// edit at your own risk!\n", file=f)
        print(generate_sibyll_enum(pythia_db), file=f)
        print(generate_corsika2sibyll(pythia_db), file=f)
        print(generate_known_particle(pythia_db), file=f)
        print(generate_sibyll2corsika(pythia_db), file=f)
        print(generate_interacting_particle(pythia_db), file=f)
        print(generate_corsika2sibyll_xsType(pythia_db), file=f)
