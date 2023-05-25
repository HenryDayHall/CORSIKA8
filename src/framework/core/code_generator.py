#!/usr/bin/env python3

import sys
import math
import itertools
import re
import csv
import pprint
import xml.etree.ElementTree as ET
from collections import OrderedDict
import pickle
import io

GeVfm = 0.19732696312541853
c_speed_of_light = 29.9792458e10  # mm / s
# for nuclear masses
mneutron = 0.9395654133  # GeV
mproton = 0.9382720813  # GeV

namespace = "corsika"

# IDs of Nuclei are 10LZZZAAAI
# used with .format(L=,Z=,A=,I=)
nucleusIdStr = "10{L:01d}{Z:03d}{A:03d}{I:01d}"
nucleusIdOffset = int(nucleusIdStr.format(L=0, A=0, Z=0, I=0))


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
            ctau = GeVfm / \
                float(particle.attrib['mWidth']) * 1e-15 * 1000.0  # mm / s
        # those are certainly not stable....
        elif pdg_id in (0, 423, 433, 4312, 4322, 5112, 5222):
            print("missing ctau0 or width for: " + str(pdg_id) + " " + str(name))
            print("setting ctau to arbitrary small value..")
            ctau = 1.e-15
        else:
            print("missing lifetime: " + str(pdg_id) + " " + str(name))
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
def read_class_names(filename):
    tree = ET.parse(filename)
    root = tree.getroot()

    map = {}

    for particle in root.iter("particle"):
        classname = None
        name = None
        if ("classname" in particle.attrib):
            classname = particle.attrib["classname"]
        if ("name" in particle.attrib):
            name = particle.attrib["name"]
        pdg_id = int(particle.attrib["pdgID"])
        map[pdg_id] = {"classname": classname, "name": name}

    return map

##############################################################
#
# Automatically produce a string qualifying as C++ class name
#
# This function produces names of type "DeltaPlusPlus"
#


def c_identifier_camel(name):
    orig = name
    name = name[0].upper() + name[1:].lower()  # all lower case

    for c in "() /":  # replace funny characters
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
        raise Exception(
            "could not generate C identifier for '{:s}': result '{:s}'".format(orig, name))


##########################################################
#
# returns dict containing all data from pythia-xml input
#
def read_pythia_db(filename, particle_db, classnames):

    counter = itertools.count(len(particle_db))

    for (pdg, name, mass, electric_charge, antiName, lifetime) in parsePythia(filename):

        c_id = "Unknown"
        if pdg in classnames and classnames[pdg]["classname"] != None:
            c_id = classnames[pdg]["classname"]
        else:
            c_id = c_identifier_camel(name)  # the camel case names

        if pdg in classnames and classnames[pdg]["name"] != None:
            name = classnames[pdg]["name"]

        hadron = abs(pdg) > 100

        if c_id in particle_db.keys():
            raise RuntimeError("particle '{:s}' already known (new PDG id {:d}, stored PDG id: {:d})".format(
                c_id, pdg, particle_db[c_id]['pdg']))

        particle_db[c_id] = {
            "name": name,
            "antiName": antiName,
            "pdg": pdg,
            "mass": mass,  # in GeV
            "electric_charge": electric_charge,  # in e/3
            "lifetime": lifetime,
            "ngc_code": next(counter),
            "isNucleus": False,
            "isHadron": hadron,
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
        if pdg in classnames and classnames[pdg]["classname"]:
            c_id = classnames[pdg]["classname"]
        else:
            c_id = c_identifier_camel(name)

        if pdg in classnames and classnames[pdg]["name"] != None:
            name = classnames[pdg]["name"]

        particle_db[c_id] = {
            "name": name,
            "antiName": antiName,
            "pdg": pdg,
            "mass": mass,  # in GeV
            "electric_charge": electric_charge,  # in e/3
            "lifetime": lifetime,
            "ngc_code": int(nucleusIdStr.format(L=0, A=A, Z=Z, I=0)),
            "A": A,
            "Z": Z,
            "isNucleus": True,
            "isHadron": True,
        }

    return particle_db


###############################################################
#
# build conversion table PDG -> ngc
#
def gen_conversion_PDG_ngc(particle_db):
    # todo: find a optimum value, think about cache miss with array vs lookup time with map
    P_MAX = 500  # the maximum PDG code that is still filled into the table

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

    oprint(
        f"static std::array<Code, {len(conversionTable)}> constexpr conversionArray {{")
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
def gen_internal_enum(particle_db, nuclei_db):
    string = ("//! @cond EXCLUDE_DOXY\n"
              "enum class Code : CodeIntType {\n"
              "  FirstParticle = 1, // if you want to loop over particles, you want to start with \"1\"  \n")  # identifier for eventual loops...

    # non-nuclei loop
    for k in filter(lambda k: "ngc_code" in particle_db[k] and not particle_db[k]["isNucleus"],
                    particle_db):
        last_ngc_id = particle_db[k]['ngc_code']
        string += "  {key:s} = {code:d},\n".format(key=k, code=last_ngc_id)

    # identifier for eventual loops...
    string += "  LastParticle = {:d},\n".format(last_ngc_id + 1)

    if last_ngc_id > 0x7fffffff:  # does not fit into int32_t, this will never happen....
        raise Exception(
            "Integer overflow in internal particle code definition prevented!")

    if last_ngc_id + 1 >= nucleusIdOffset:
        raise Exception(
            "Too many particles. Fix conflict with Code::Nuclear == {:d} (just increase id...) !").format(nucleusIdOffset)

    # marker to mark the beginning of generic nuclear IDs: 1aaazzz
    # identifier for Nuclei
    string += "  Nucleus = {:d},\n".format(nucleusIdOffset)

    # nuclei loop
    for k in filter(lambda k: "ngc_code" in nuclei_db[k] and nuclei_db[k]["isNucleus"],
                    nuclei_db):
        last_ngc_id = nuclei_db[k]['ngc_code']
        string += "  {key:s} = {code:d},\n".format(key=k, code=last_ngc_id)

    string += "}; //! @endcond"
    return string


###############################################################
#
# return string with enum of all PDG particle codes
#
def gen_pdg_enum(particle_db):
    string = ("//! @cond EXCLUDE_DOXY\n"
              "enum class PDGCode : PDGCodeIntType {\n")

    for cId in particle_db:
        pdgCode = particle_db[cId]['pdg']
        string += "  {key:s} = {code:d},\n".format(key=cId, code=pdgCode)

    string += " }; //! @endcond \n"

    return string


###############################################################
#
# return string with all data arrays
#
def gen_properties(particle_db):

    # number of particles, size of tables
    string = "static constexpr std::size_t size = {size:d};\n".format(
        size=len(particle_db))
    string += "\n"

    # all particle initializer_list
    string += "constexpr std::initializer_list<Code> all_particles = {"
    for k in particle_db:
        string += "  Code::{name:s},\n".format(name=k)
    string += "};\n"
    string += "\n"

    # particle masses table
    string += "static constexpr std::array<HEPMassType const, size> masses = {\n"
    for p in particle_db.values():
        string += "  {mass:e} * 1e9 * electronvolt, // {name:s}\n".format(
            mass=p['mass'], name=p['name'])
    string += "};\n\n"

    # particle threshold table, initially set to 0
    string += "static std::array<HEPEnergyType, size> propagation_thresholds = {\n"
    for k in particle_db:
        string += " 1e9 * electronvolt, // {name:s}\n".format(
            name=k)
    string += "};\n\n"
    string += "static HEPEnergyType threshold_nuclei = 0_eV;\n"

    # particle production_threshold table, initially set to 1 MeV
    string += "static std::array<HEPEnergyType, size> production_thresholds = {\n"
    for p in particle_db.values():
        string += "  1e6 * electronvolt, // {name:s}\n".format(
            name=p['name'])
    string += "};\n\n"

    # PDG code table
    string += "static constexpr std::array<PDGCode, size> pdg_codes = {\n"
    for p in particle_db.keys():
        string += f"  PDGCode::{p},\n"
    string += "};\n"

    # name string table
    string += "static constexpr std::array<std::string_view, size> names = {\n"
    for p in particle_db.values():
        string += "  \"{name:s}\",\n".format(name=p['name'])
    string += "};\n"

    # electric charges table
    string += "static constexpr std::array<int32_t, size> electric_charges = {\n"
    for p in particle_db.values():
        string += "  {charge:d},\n".format(charge=p['electric_charge'] // 3)
    string += "};\n"

    # anti-particle table
    #    string += "static constexpr std::array<size, size> anti_particle = {\n"
    #    for p in particle_db.values():
    #        string += "  {anti:d},\n".format(charge = p['anti_particle'])
    #    string += "};\n"

    # lifetime
    #string += "static constexpr std::array<TimeType const, size> lifetime = {\n"
    string += "static constexpr std::array<double const, size> lifetime = {\n"
    for p in particle_db.values():
        if p['lifetime'] == float("Inf"):
            # * second, \n"
            string += "  std::numeric_limits<double>::infinity(), \n"
        else:
            string += "  {tau:e}, \n".format(tau=p['lifetime'])
            #string += "  {tau:e} * second, \n".format(tau = p['lifetime'])
    string += "};\n"

    # is Hadron flag
    string += "static constexpr std::array<bool, size> isHadron = {\n"
    for p in particle_db.values():
        value = 'false'
        if p['isHadron']:
            value = 'true'
        string += "  {val},\n".format(val=value)
    string += "};\n"

    return string


###############################################################
#
# return string with a list of classes for all particles
#
def gen_classes(particle_db, nuclei_db):

    string = ("// list of C++ classes to access particle properties\n"
              "/** @defgroup ParticleClasses \n"
              "    @{ */\n")

    common_db = OrderedDict(
        list(particle_db.items()) + list(nuclei_db.items()))
    for cname in common_db:
        if cname == "Nucleus":
            string += "// skipping Nucleus"
            continue

        antiP = 'Unknown'
        for cname_anti in common_db:
            if (common_db[cname_anti]['name'] == common_db[cname]['antiName']):
                antiP = cname_anti
                break

        string += "\n"
        string += "/** @class " + cname + "\n\n"
        string += " * Particle properties are taken from the PYTHIA8 ParticleData.xml file:<br>\n"
        string += " *  - pdg=" + str(common_db[cname]['pdg']) + "\n"
        string += " *  - mass=" + str(common_db[cname]['mass']) + " GeV \n"
        string += " *  - charge= " + \
            str(common_db[cname]['electric_charge'] // 3) + " \n"
        string += " *  - name=" + str(cname) + "\n"
        string += " *  - anti=" + str(antiP) + "\n"
        if (common_db[cname]['isNucleus']):
            string += " *  - nuclear A=" + str(common_db[cname]['A']) + "\n"
            string += " *  - nuclear Z=" + str(common_db[cname]['Z']) + "\n"
        string += "*/\n\n"
        string += "class " + cname + " {\n"
        string += "   /** @cond EXCLUDE_DOXY */ \n"
        string += "  public:\n"
        string += "   " + cname + "() = delete;\n"
        string += "   static constexpr Code code{Code::" + cname + "};\n"
        string += "   static constexpr Code anti_code{Code::" + antiP + "};\n"
        string += "   static constexpr HEPMassType mass{corsika::get_mass(code)};\n"
        string += "   static constexpr ElectricChargeType charge{corsika::get_charge(code)};\n"
        string += "   static constexpr int charge_number{corsika::get_charge_number(code)};\n"
        string += "   static constexpr std::string_view name{corsika::get_name(code)};\n"
        string += "   static constexpr bool is_nucleus{corsika::is_nucleus(code)};\n"
        if common_db[cname]['isNucleus']:
            string += "   static constexpr int nucleus_A{corsika::get_nucleus_A(code)};\n"
            string += "   static constexpr int nucleus_Z{corsika::get_nucleus_Z(code)};\n"
        string += " private:\n"
        string += "   static constexpr CodeIntType TypeIndex = static_cast<CodeIntType>(code);\n"
        string += "   /** @endcond */ \n"
        string += "};\n"

    string += "  //! @}\n"

    return string


###############################################################
#
#
def inc_start():
    string = ('// generated by framework/core/code_generator.py\n'
              '// MANUAL EDITS ON OWN RISK. THEY WILL BE OVERWRITTEN. \n'
              '\n'
              'namespace corsika {\n'
              '/** @ingroup Particles \n'
              '    @{ \n'
              '  */ \n'
              )
    return string


###############################################################
#
#
def detail_start():
    string = ('/** @cond EXCLUDE_DOXY */\n'
              ' namespace particle::detail {\n\n')
    return string


###############################################################
#
#
def detail_end():
    string = "\n}//end namespace particle::detail\n /** @endcond */"
    return string

###############################################################
#
#


def inc_end():
    string = "/** @} */\n} // end namespace corsika"
    return string


###################################################################
#
# Serialize particle_db into file
#
def serialize_particle_db(particle_db, nuclei_db, file):
    common_db = OrderedDict(
        list(particle_db.items()) + list(nuclei_db.items()))
    pickle.dump(common_db, file)


###################################################################
#
# Main function
#
if __name__ == "__main__":

    if len(sys.argv) != 4:
        print("usage: {:s} <ParticleData.xml> <NuclearData.xml> <ParticleClassNames.xml>".format(
            sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    print("\n       code_generator.py: automatically produce particle properties from input files\n")

    names = read_class_names(sys.argv[3])  # re-names and conventions
    particle_db = OrderedDict()  # the DB for pythia8 pdg particles
    read_pythia_db(sys.argv[1], particle_db, names)  # pythia8 pdg DB
    nuclei_db = OrderedDict()  # the DB for specific nuclei
    read_nuclei_db(sys.argv[2], nuclei_db, names)  # list of nuclei

    with open("GeneratedParticleProperties.inc", "w") as f:
        print(inc_start(), file=f)
        print(gen_internal_enum(particle_db, nuclei_db), file=f)
        print(gen_pdg_enum(particle_db), file=f)
        print(detail_start(), file=f)
        print(gen_properties(particle_db), file=f)
        print(gen_conversion_PDG_ngc(particle_db), file=f)
        print(detail_end(), file=f)
        print(inc_end(), file=f)

    with open("GeneratedParticleClasses.inc", "w") as f:
        print(inc_start(), file=f)
        print(gen_classes(particle_db, nuclei_db), file=f)
        print(inc_end(), file=f)

    with open("particle_db.pkl", "wb") as f:
        serialize_particle_db(particle_db, nuclei_db, f)
