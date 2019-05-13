#!/usr/bin/env python3

import sys, math, itertools, re, csv, pprint
import xml.etree.ElementTree as ET
from collections import OrderedDict
import pickle
import io

GeVfm = 0.19732696312541853
c_speed_of_light = 29.9792458e10  # mm / s
# for nuclear masses
mneutron = 0.9395654133 # GeV
mproton = 0.9382720813 # GeV


##############################################################
# 
# reading xml input data, return line by line particle data
# 
def parsePythia(filename):
    tree = ET.parse(filename)
    root = tree.getroot()
    
    for particle in root.iter("particle"):
        name = particle.attrib["name"]        
        antiName = "Unknown"
        if ("antiName" in particle.attrib):
            antiName = particle.attrib["antiName"]
        pdg_id = int(particle.attrib["id"])
        mass = float(particle.attrib["m0"])  # GeV
        electric_charge = int(particle.attrib["chargeType"])  # in units of e/3        
        ctau = 0.
        if pdg_id in (11, 12, 14, 16, 22, 2212):  # these are the stable particles !
            ctau = float('Inf')
        elif 'tau0' in particle.attrib:
            ctau = float(particle.attrib['tau0'])  # mm / c
        elif 'mWidth' in particle.attrib:
            ctau = GeVfm / float(particle.attrib['mWidth']) * 1e-15 * 1000.0  # mm / s
        elif pdg_id in (0, 423, 433, 4312, 4322, 5112, 5222):  # those are certainly not stable....
            ctau = 0.
        else:
            print ("missing lifetime: " + str(pdg_id) + " " + str(name))
            sys.exit(1)
        
        yield (pdg_id, name, mass, electric_charge, antiName, ctau/c_speed_of_light)
                
        # TODO: read decay channels from child elements
        
        if "antiName" in particle.attrib:
            yield (-pdg_id, antiName, mass, -electric_charge, name, ctau/c_speed_of_light)


##############################################################
# 
# reading xml input data, return line by line particle data
# 
def parseNuclei(filename):
    tree = ET.parse(filename)
    root = tree.getroot()
        
    for particle in root.iter("particle"):
        name = particle.attrib["name"]        
        antiName = "Unknown"
        if ("antiName" in particle.attrib):
            antiName = particle.attrib["antiName"]
        pdg_id = int(particle.attrib["id"])
        A = int(particle.attrib["A"])
        Z = int(particle.attrib["Z"])
        # mass in GeV
        if ("mass" in particle.attrib):
            mass = particle.attrib["mass"] 
        else:
            mass = (A-Z)*mneutron + Z*mproton

        electric_charge = Z*3  # in units of e/3        
        ctau = float('Inf')
        
        yield (pdg_id, name, mass, electric_charge, antiName, ctau/c_speed_of_light, A, Z)
                

        
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

    # remove all "_", if this does not by accident concatenate two numbers
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
def read_pythia_db(filename, particle_db, classnames):    
    
    counter = itertools.count(len(particle_db))
    
    for (pdg, name, mass, electric_charge, antiName, lifetime) in parsePythia(filename):

        c_id = "Unknown"
        if pdg in classnames:
            c_id = classnames[pdg]
        else:
            c_id = c_identifier_camel(name) # the camel case names

        hadron = False
        if abs(pdg) > 100:
            hadron = True
            
        particle_db[c_id] = {
            "name" : name,
            "antiName" : antiName,
            "pdg" : pdg,
            "mass" : mass, # in GeV
            "electric_charge" : electric_charge, # in e/3
            "lifetime" : lifetime,
            "ngc_code" : next(counter),
            "isNucleus" : False,
            "isHadron" : hadron,
        }
    
    return particle_db
    


##########################################################
# 
# returns dict containing all data from pythia-xml input
#     
def read_nuclei_db(filename, particle_db, classnames):    
    
    counter = itertools.count(len(particle_db))

    for (pdg, name, mass, electric_charge, antiName, lifetime, A, Z) in parseNuclei(filename):
        
        c_id = "Unknown"
        if pdg in classnames:
            c_id = classnames[pdg]
        else:
            c_id = c_identifier_camel(name)

        particle_db[c_id] = {
            "name" : name,
            "antiName" : antiName,
            "pdg" : pdg,
            "mass" : mass, # in GeV
            "electric_charge" : electric_charge, # in e/3
            "lifetime" : lifetime,
            "ngc_code" : next(counter),
            "A" : A,
            "Z" : Z,
            "isNucleus" : True,
            "isHadron" : True,
        }
    
    return particle_db


###############################################################
# 
# build conversion table PDG -> ngc
# 
def gen_conversion_PDG_ngc(particle_db):
    # todo: find a optimum value, think about cache miss with array vs lookup time with map
    P_MAX = 500 # the maximum PDG code that is still filled into the table
        
    conversionDict = dict()
    conversionTable = [None] * (2*P_MAX + 1)
    for cId, p in particle_db.items():
        pdg = p['pdg']
        
        if abs(pdg) < P_MAX:
            if conversionTable[pdg + P_MAX]:
                raise Exception("table entry already occupied")
            else:
                conversionTable[pdg + P_MAX] = cId
        else:
            if pdg in conversionDict.keys():
                raise Exception(f"map entry {pdg} already occupied")
            else:
                conversionDict[pdg] = cId
    
    output = io.StringIO()
    def oprint(*args, **kwargs):
        print(*args, **kwargs, file=output)
        
    oprint(f"static std::array<Code, {len(conversionTable)}> constexpr conversionArray {{")
    for ngc in conversionTable:
        oprint("    Code::{0},".format(ngc if ngc else "Unknown"))
    oprint("};")
    oprint()
    
    oprint("static std::map<PDGCode, Code> const conversionMap {")
    for ngc in conversionDict.values():
        oprint(f"    {{PDGCode::{ngc}, Code::{ngc}}},")
    oprint("};")
    oprint()
    
    return output.getvalue()


###############################################################
# 
# return string with enum of all internal particle codes
# 
def gen_internal_enum(particle_db):
    string = ("enum class Code : CodeIntType {\n"
              "  FirstParticle = 1, // if you want to loop over particles, you want to start with \"1\"  \n") # identifier for eventual loops...
    
    
    for k in filter(lambda k: "ngc_code" in particle_db[k], particle_db):
        last_ngc_id = particle_db[k]['ngc_code']
        string += "  {key:s} = {code:d},\n".format(key = k, code = last_ngc_id)

    string += ("  LastParticle = {:d},\n" # identifier for eventual loops...
               "}};").format(last_ngc_id + 1)
               
    if last_ngc_id > 0x7fff: # does not fit into int16_t
        raise Exception("Integer overflow in internal particle code definition prevented!")
    
    return string


###############################################################
# 
# return string with enum of all PDG particle codes
# 
def gen_pdg_enum(particle_db):
    string = "enum class PDGCode : PDGCodeType {\n"
    
    for cId in particle_db:
        pdgCode = particle_db[cId]['pdg']
        string += "  {key:s} = {code:d},\n".format(key = cId, code = pdgCode)

    string += " };\n"
    
    return string


###############################################################
# 
# return string with all data arrays 
# 
def gen_properties(particle_db):

    # number of particles, size of tables
    string = "static constexpr std::size_t size = {size:d};\n".format(size = len(particle_db))
    string += "\n"
    
    # particle masses table
    string += "static constexpr std::array<corsika::units::si::HEPMassType const, size> masses = {\n"    
    for p in particle_db.values():
        string += "  {mass:e} * 1e9 * corsika::units::si::electronvolt, // {name:s}\n".format(mass = p['mass'], name = p['name'])
    string += "};\n\n"
                   
    # PDG code table
    string += "static constexpr std::array<PDGCode, size> pdg_codes = {\n"
    for p in particle_db.keys():
        string += f"  PDGCode::{p},\n"
    string += "};\n"
    
    # name string table
    string += "static const std::array<std::string const, size> names = {\n"
    for p in particle_db.values():
        string += "  \"{name:s}\",\n".format(name = p['name'])            
    string += "};\n"
    
    # electric charges table
    string += "static constexpr std::array<int16_t, size> electric_charges = {\n"
    for p in particle_db.values():
        string += "  {charge:d},\n".format(charge = p['electric_charge'])
    string += "};\n"

    # anti-particle table
    #    string += "static constexpr std::array<size, size> anti_particle = {\n"
    #    for p in particle_db.values():
    #        string += "  {anti:d},\n".format(charge = p['anti_particle'])
    #    string += "};\n"

    # lifetime
    #string += "static constexpr std::array<corsika::units::si::TimeType const, size> lifetime = {\n"
    string += "static constexpr std::array<double const, size> lifetime = {\n"
    for p in particle_db.values():
        if p['lifetime'] == float("Inf") :
            string += "  std::numeric_limits<double>::infinity(), \n" # * corsika::units::si::second, \n"
        else :
            string += "  {tau:e}, \n".format(tau = p['lifetime'])
            #string += "  {tau:e} * corsika::units::si::second, \n".format(tau = p['lifetime'])
    string += "};\n"
    
    # is Hadron flag
    string += "static constexpr std::array<bool, size> isHadron = {\n"
    for p in particle_db.values():
        value = 'false'
        if p['isHadron']:
            value = 'true'
        string += "  {val},\n".format(val = value)
    string += "};\n"

    
    ### nuclear data ###
    
    # nucleus mass number A
    string += "static constexpr std::array<int16_t, size> nucleusA = {\n"
    for p in particle_db.values():
        A = p.get('A', 0)
        string += "  {val},\n".format(val = A)
    string += "};\n"
    
    # nucleus charge number Z
    string += "static constexpr std::array<int16_t, size> nucleusZ = {\n"
    for p in particle_db.values():
        Z = p.get('Z', 0)
        string += "  {val},\n".format(val = Z)
    string += "};\n"

    return string


###############################################################
# 
# return string with a list of classes for all particles
# 
def gen_classes(particle_db):

    string = "// list of C++ classes to access particle properties"
    
    for cname in particle_db:

        antiP = 'Unknown'
        for cname_anti in particle_db:
            if (particle_db[cname_anti]['name'] == particle_db[cname]['antiName']):
                antiP = cname_anti
                break
        
        string += "\n";
        string += "/** @class " + cname + "\n\n"
        string += " * Particle properties are taken from the PYTHIA8 ParticleData.xml file:<br>\n"
        string += " *  - pdg=" + str(particle_db[cname]['pdg']) +"\n"
        string += " *  - mass=" + str(particle_db[cname]['mass']) + " GeV \n"
        string += " *  - charge= " + str(particle_db[cname]['electric_charge']/3) + " \n"
        string += " *  - name=" + str(cname) + "\n"
        string += " *  - anti=" + str(antiP) + "\n"
        if (particle_db[cname]['isNucleus']):
            string += " *  - nuclear A=" + str(particle_db[cname]['A']) + "\n"
            string += " *  - nuclear Z=" + str(particle_db[cname]['Z']) + "\n"        
        string += "*/\n\n"
        string += "class " + cname + " {\n"
        string += "  public:\n"
        string += "   static constexpr Code GetCode() { return Type; }\n"
        string += "   static constexpr corsika::units::si::HEPMassType GetMass() { return corsika::particles::GetMass(Type); }\n"
        string += "   static constexpr corsika::units::si::ElectricChargeType GetCharge() { return corsika::particles::GetCharge(Type); }\n"
        string += "   static constexpr int16_t GetChargeNumber() { return corsika::particles::GetChargeNumber(Type); }\n"
        string += "   static std::string const& GetName() { return corsika::particles::GetName(Type); }\n"
        string += "   static constexpr Code GetAntiParticle() { return AntiType; }\n"
        string += "   static constexpr bool IsNucleus() { return corsika::particles::IsNucleus(Type); }\n"
        string += "   static constexpr int16_t GetNucleusA() { return corsika::particles::GetNucleusA(Type); }\n"
        string += "   static constexpr int16_t GetNucleusZ() { return corsika::particles::GetNucleusZ(Type); }\n"
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
              '// MANUAL EDITS ON OWN RISK. THEY WILL BE OVERWRITTEN. \n')
    return string


###############################################################
# 
# 
def detail_start():
    string = ('namespace detail {\n\n')
    return string


###############################################################
# 
# 
def detail_end():
    string = "\n}//end namespace detail\n"
    return string

###############################################################
# 
# 
def inc_end():
    string = ""
    return string


###################################################################
# 
# Serialize particle_db into file 
# 
def serialize_particle_db(particle_db, file):
    pickle.dump(particle_db, file)

    
###################################################################
# 
# Main function
# 
if __name__ == "__main__":
    
    if len(sys.argv) != 4:
        print("usage: {:s} <Pythia8.xml> <Nuclei.xml> <ClassNames.xml>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)
        
    print("\n       pdxml_reader.py: automatically produce particle properties from input files\n")
    
    names = class_names(sys.argv[3])
    particle_db = OrderedDict()
    read_pythia_db(sys.argv[1], particle_db, names)
    read_nuclei_db(sys.argv[2], particle_db, names)
    
    with open("GeneratedParticleProperties.inc", "w") as f:
        print(inc_start(), file=f)
        print(gen_internal_enum(particle_db), file=f)
        print(gen_pdg_enum(particle_db), file=f)
        print(detail_start(), file=f)
        print(gen_properties(particle_db), file=f)
        print(gen_conversion_PDG_ngc(particle_db), file=f)
        print(detail_end(), file=f) 
        print(gen_classes(particle_db), file=f)
        print(inc_end(), file=f) 
    
    with open("particle_db.pkl", "wb") as f:
        serialize_particle_db(particle_db, f)
    
