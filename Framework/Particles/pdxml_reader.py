#!/usr/bin/env python3

import sys, math, itertools, re, csv, pprint
import xml.etree.ElementTree as ET
from collections import OrderedDict



##############################################################
# 
# reading xml input data, return line by line particle data
# 

def parse(filename):
    tree = ET.parse(filename)
    root = tree.getroot()
        
    for particle in root.iter("particle"):
        name = particle.attrib["name"]
        antiName = "Unknown"
        # print (str(particle.attrib))
        if ("antiName" in particle.attrib):
            antiName = particle.attrib["antiName"]            
#            print ("found anti: " + name + " " + antiName + "\n" )
        pdg_id = int(particle.attrib["id"])
        mass = float(particle.attrib["m0"]) # GeV
        electric_charge = int(particle.attrib["chargeType"]) # in units of e/3
        
        decay_width = float(particle.attrib.get("mWidth", 0)) # GeV
        lifetime = float(particle.attrib.get("tau0", math.inf)) # mm / c
        
        yield (pdg_id, name, mass, electric_charge, antiName)
                
        # TODO: read decay channels from child elements
        
        if "antiName" in particle.attrib:
            yield (-pdg_id, antiName, mass, -electric_charge, name)


            
##############################################################
# 
# returns dict with particle codes and class names
# 

def class_names(filename):
    tree = ET.parse(filename)
    root = tree.getroot()

    map = {}
    
    for particle in root.iter("particle"):
        name = particle.attrib["classname"]
        pdg_id = int(particle.attrib["id"])
        map[pdg_id] = name
        
    return map 



##############################################################
# 
# Automatically produce a string qualifying as C++ class name
# 
            
def c_identifier(name):
    orig = name
    name = name.upper()
    for c in "() ":
        name = name.replace(c, "_")
    
    name = name.replace("BAR", "_BAR")
    name = name.replace("0", "_0")
    name = name.replace("/", "_")
    name = name.replace("*", "_STAR")
    name = name.replace("'", "_PRIME")
    name = name.replace("+", "_PLUS")
    name = name.replace("-", "_MINUS")
    
    while True:
        tmp = name.replace("__", "_")
        if tmp == name:
            break
        else:
            name = tmp    

    pattern = re.compile(r'^[A-Z_][A-Z_0-9]*$')
    if pattern.match(name):
        return name.strip("_")
    else:
        raise Exception("could not generate C identifier for '{:s}'".format(orig))


    
# ########################################################
# 
# returns dict containing all data from pythia-xml input
# 
    
def build_pythia_db(filename, classnames):    
    particle_db = OrderedDict()    
    for (pdg, name, mass, electric_charge, antiName) in parse(filename):

        c_id = "unknown"
        if (pdg in classnames):
            c_id = classnames[pdg]
        else:
            c_id = c_identifier(name)
        
        #~ print(name, c_id, sep='\t', file=sys.stderr)
        #~ enums += "{:s} = {:d}, ".format(c_id, corsika_id)
        particle_db[c_id] = {
            "name" : name,
            "antiName" : antiName,
            "pdg" : pdg,
            "mass" : mass, # in GeV
            "electric_charge" : electric_charge # in e/3
        }
    
    return particle_db
    



###############################################################
# 
# return string with enum of all internal particle codes
# 

def gen_internal_enum(pythia_db):
    string = "enum class InternalParticleCode : uint8_t {\n"
    
    for k in filter(lambda k: "ngc_code" in pythia_db[k], pythia_db):
        string += "  {key:s} = {code:d},\n".format(key = k, code = pythia_db[k]['ngc_code'])
    
    string += "};"
    return string




###############################################################
# 
# return string with all data arrays 
# 

def gen_properties(pythia_db):

    # number of particles, size of tables
    string = "static constexpr std::size_t size = {size:d};\n".format(size = len(pythia_db))
    string += "\n"
    
    # particle masses table
    string += "static constexpr std::array<quantity<energy_d> const, size> masses = {\n"    
    for p in pythia_db.values():
        string += "  {mass:f}_GeV, // {name:s}\n".format(mass = p['mass'], name = p['name'])              
    string += "};\n\n"
                   
    # PDG code table
    string += "static constexpr std::array<PDGCode const, size> pdg_codes = {\n"
    for p in pythia_db.values():
        string += "  {pdg:d}, // {name:s}\n".format(pdg = p['pdg'], name = p['name'])    
    string += "};\n"
    
    # name string table
    string += "static const std::array<std::string const, size> names = {\n"
    for p in pythia_db.values():
        string += "  \"{name:s}\",\n".format(name = p['name'])            
    string += "};\n"
    
    # electric charges table
    string += "static constexpr std::array<int16_t, size> electric_charge = {\n"
    for p in pythia_db.values():
        string += "  {charge:d},\n".format(charge = p['electric_charge'])
    string += "};\n"

    # anti-particle table
#    string += "static constexpr std::array<size, size> anti_particle = {\n"
#    for p in pythia_db.values():
#        string += "  {anti:d},\n".format(charge = p['anti_particle'])
#    string += "};\n"

    return string




###############################################################
# 
# return string with a list of classes for all particles
# 

def gen_classes(pythia_db):

    string = "// list of C++ classes to access particle properties"
    
    for cname in pythia_db:

        antiP = 'unknown'
        for cname_anti in pythia_db:
            if (pythia_db[cname_anti]['name'] == pythia_db[cname]['antiName']):
                antiP = cname_anti
                break
        
        string += "\n";
        string += "/** @class " + cname + "\n"
        string += "*/\n\n"
        string += "struct " + cname + "{\n"
        string += "   static InternalParticleCode GetType() { return Type; }\n"
        string += "   static quantity<energy_d> GetMass() { return masses[TypeIndex]; }\n"
        string += "   static quantity<electric_charge_d> GetCharge() { return phys::units::e*electric_charge[TypeIndex]/3; }\n"
        string += "   static std::string GetName() { return names[TypeIndex]; }\n"
        string += "   static InternalParticleCode GetAntiParticle() { return AntiType; }\n"
        string += "   static const InternalParticleCode Type = InternalParticleCode::" + cname + ";\n"
        string += "   static const InternalParticleCode AntiType = InternalParticleCode::" + antiP + ";\n"
        string += " private:\n"
        string += "   static const uint8_t TypeIndex = static_cast<uint8_t const>(Type);\n"
        string += "};\n"

    return string


###############################################################
# 
# 

def inc_start():
    string = ""
    string += "#ifndef _include_GeneratedParticleDataTable_h_\n"
    string += "#define _include_GeneratedParticleDataTable_h_\n\n"
    string += "#include <array>\n"
    string += "#include <cstdint>\n"
    string += "#include <iostream>\n\n"
    string += "using namespace phys::units;\n"
    string += "using namespace phys::units::literals;\n\n"
    string += "namespace ParticleProperties {\n\n"
    string += "typedef int16_t PDGCode;\n\n"
    return string


###############################################################
# 
# 

def inc_end():
    string = ""
    string += "\n}\n\n"
    string += "#endif\n"
    return string


###################################################################
# 
# Main function
# 

if __name__ == "__main__":

    if (len(sys.argv)!=3) :
        print ("pdxml_reader.py Pythia8.xml ClassNames.xml")
        sys.exit(0)
        
    names = class_names(sys.argv[2])
    pythia_db = build_pythia_db(sys.argv[1], names)

    print ("\n       pdxml_reader.py: Automatically produce particle-properties from PYTHIA8 xml file\n")
    
    counter = itertools.count(0)
    
    not_modeled = []
    for p in pythia_db:
        pythia_db[p]['ngc_code'] = next(counter)               
    
    with open("GeneratedParticleProperties.inc", "w") as f:
        print(inc_start(), file=f) 
        print(gen_internal_enum(pythia_db), file=f)
        print(gen_properties(pythia_db), file=f)
        print(gen_classes(pythia_db), file=f)
        print(inc_end(), file=f) 
    
    #~ print(pdg_id_table, mass_table, name_table, enums, sep='\n\n')


    
