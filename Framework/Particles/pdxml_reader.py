#!/usr/bin/env python3

import sys, math, itertools, re, csv, pprint
import xml.etree.ElementTree as ET
from collections import OrderedDict
import pickle


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
        pdg_id = int(particle.attrib["pdgID"])
        map[pdg_id] = name
        
    return map 



##############################################################
# 
# Automatically produce a string qualifying as C++ class name
# 
# This function produces names of type "DELTA_PLUS_PLUS"
# 
def c_identifier(name):
    orig = name
    name = name.upper()
    for c in "() /":
        name = name.replace(c, "_")
    
    name = name.replace("BAR", "_BAR")
    name = name.replace("0", "_0")
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


##############################################################
# 
# Automatically produce a string qualifying as C++ class name
# 
# This function produces names of type "DeltaPlusPlus"
# 
def c_identifier_camel(name):
    orig = name
    name = name[0].upper() + name[1:].lower() # all lower case
    
    for c in "() /": # replace funny characters
        name = name.replace(c, "_")
    
    name = name.replace("bar", "Bar")
    name = name.replace("*", "Star")
    name = name.replace("'", "Prime")
    name = name.replace("+", "Plus")
    name = name.replace("-", "Minus")

    # move "Bar" to end of name
    ibar = name.find('Bar')
    if ibar > 0 and ibar < len(name)-3:
        name = name[:ibar] + name[ibar+3:] + 'Bar'
    
    # cleanup "_"s
    while True:
        tmp = name.replace("__", "_")
        if tmp == name:
            break
        else:
            name = tmp
    name.strip("_")

    # remove all "_", if this does not by accident concatenate two number
    istart = 0
    while True:
        i = name.find('_', istart)
        if i < 1 or i > len(name)-1:
            break
        istart = i
        if name[i-1].isdigit() and name[i+1].isdigit():
            # there is a number on both sides
            break
        name = name[:i] + name[i+1:]
        # and last, for example: make NuE out of Nue
        if name[i-1].islower() and name[i].islower():
            if i < len(name)-1:
                name = name[:i] + name[i].upper() + name[i+1:]
            else:
                name = name[:i] + name[i].upper()

    # check if name is valid C++ identifier
    pattern = re.compile(r'^[a-zA-Z_][a-zA-Z_0-9]*$')
    if pattern.match(name):
        return name
    else:
        raise Exception("could not generate C identifier for '{:s}': result '{:s}'".format(orig, name))


    
##########################################################
# 
# returns dict containing all data from pythia-xml input
# 
    
def build_pythia_db(filename, classnames):    
    particle_db = OrderedDict()
    
    counter = itertools.count(0)
    
    for (pdg, name, mass, electric_charge, antiName) in parse(filename):

        c_id = "Unknown"
        if pdg in classnames:
            c_id = classnames[pdg]
        else:
            c_id = c_identifier_camel(name) # the camel case names
        
        particle_db[c_id] = {
            "name" : name,
            "antiName" : antiName,
            "pdg" : pdg,
            "mass" : mass, # in GeV
            "electric_charge" : electric_charge, # in e/3
            "ngc_code" : next(counter)
        }
    
    return particle_db
    



###############################################################
# 
# return string with enum of all internal particle codes
# 

def gen_internal_enum(pythia_db):
    string = ("enum class Code : int16_t {\n"
              "  FirstParticle = 1, // if you want to loop over particles, you want to start with \"1\"  \n") # identifier for eventual loops...
    
    
    for k in filter(lambda k: "ngc_code" in pythia_db[k], pythia_db):
        last_ngc_id = pythia_db[k]['ngc_code']
        string += "  {key:s} = {code:d},\n".format(key = k, code = last_ngc_id)

    string += ("  LastParticle = {:d},\n" # identifier for eventual loops...
               "}};").format(last_ngc_id + 1)
               
    if last_ngc_id > 0x7fff: # does not fit into int16_t
        raise Exception("Integer overflow in internal particle code definition prevented!")
    
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
    string += "static constexpr std::array<corsika::units::si::MassType const, size> masses = {\n"    
    for p in pythia_db.values():
        string += "  {mass:f} * (1e9 * corsika::units::si::constants::eV / corsika::units::si::constants::cSquared), // {name:s}\n".format(mass = p['mass'], name = p['name'])              
    string += "};\n\n"
                   
    # PDG code table
    string += "static constexpr std::array<PDGCodeType const, size> pdg_codes = {\n"
    for p in pythia_db.values():
        string += "  {pdg:d}, // {name:s}\n".format(pdg = p['pdg'], name = p['name'])    
    string += "};\n"
    
    # name string table
    string += "static const std::array<std::string const, size> names = {\n"
    for p in pythia_db.values():
        string += "  \"{name:s}\",\n".format(name = p['name'])            
    string += "};\n"
    
    # electric charges table
    string += "static constexpr std::array<int16_t, size> electric_charges = {\n"
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

        antiP = 'Unknown'
        for cname_anti in pythia_db:
            if (pythia_db[cname_anti]['name'] == pythia_db[cname]['antiName']):
                antiP = cname_anti
                break
        
        string += "\n";
        string += "/** @class " + cname + "\n\n"
        string += " * Particle properties are taken from the PYTHIA8 ParticleData.xml file:<br>\n"
        string += " *  - pdg=" + str(pythia_db[cname]['pdg']) +"\n"
        string += " *  - mass=" + str(pythia_db[cname]['mass']) + " GeV/c2 \n"
        string += " *  - charge= " + str(pythia_db[cname]['electric_charge']/3) + " \n"
        string += " *  - name=" + str(cname) + "\n"
        string += " *  - anti=" + str(antiP) + "\n"
        string += "*/\n\n"
        string += "class " + cname + " {\n"
        string += "  public:\n"
        string += "   static constexpr Code GetCode() { return Type; }\n"
        string += "   static constexpr corsika::units::si::MassType GetMass() { return corsika::particles::GetMass(Type); }\n"
        string += "   static constexpr corsika::units::si::ElectricChargeType GetCharge() { return corsika::particles::GetElectricCharge(Type); }\n"
        string += "   static constexpr int16_t GetChargeNumber() { return corsika::particles::GetElectricChargeNumber(Type); }\n"
        string += "   static std::string const& GetName() { return corsika::particles::GetName(Type); }\n"
        string += "   static constexpr Code GetAntiParticle() { return AntiType; }\n"
        string += "   static constexpr Code Type = Code::" + cname + ";\n"
        string += "   static constexpr Code AntiType = Code::" + antiP + ";\n"
        string += " private:\n"
        string += "   static constexpr CodeIntType TypeIndex = static_cast<CodeIntType const>(Type);\n"
        string += "};\n"

    return string


###############################################################
# 
# 

def inc_start():
    string = ('// generated by pdxml_reader.py\n'
              '// MANUAL EDITS ON OWN RISK\n')
    return string


###############################################################
# 
# 

def inc_end():
    string = ""
    return string


###################################################################
# 
# Serialize pythia_db into file 
# 

def serialize_pythia_db(pythia_db, file):
    pickle.dump(pythia_db, file)

###################################################################
# 
# Main function
# 

if __name__ == "__main__":

    if len(sys.argv) != 3:
        print("usage: {:s} <Pythia8.xml> <ClassNames.xml>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)
        
    names = class_names(sys.argv[2])
    pythia_db = build_pythia_db(sys.argv[1], names)

    print("\n       pdxml_reader.py: Automatically produce particle-properties from PYTHIA8 xml file\n")
    
    with open("GeneratedParticleProperties.inc", "w") as f:
        print(inc_start(), file=f) 
        print(gen_internal_enum(pythia_db), file=f)
        print(gen_properties(pythia_db), file=f)
        print(gen_classes(pythia_db), file=f)
        print(inc_end(), file=f) 
    
    with open("pythia_db.pkl", "wb") as f:
        serialize_pythia_db(pythia_db, f)
