#!/usr/bin/env python3

import sys, math, itertools, re, csv, pprint
import xml.etree.ElementTree as ET
from collections import OrderedDict

# for testing
def sib_to_pdg(sib_id): # adapted from sibyll2.3c.f
    idmap = [22,-11,11,-13,13,111,211,-211,321,-321,
          130,310,2212,2112,12,-12,14,-14,-2212,-2112,         
          311,-311,221,331,213,-213,113,323,-323,313,          
          -313,223,333,3222,3212,3112,3322,3312,3122,2224,     
          2214,2114,1114,3224,3214,3114,3324,3314,3334,0,     
          202212,202112,212212,212112,0,0,0,0,411,-411,     
          900111,900211,-900211,0,0,0,0,0,0,0,                        
          421,-421,441,431,-431,433,-433,413,-413,423,     
          -423,0,443,4222,4212,4112,4232,4132,4122,-15,     
          15,-16,16,4224,4214,4114,4324,4314,4332]
          
    pida = abs(sib_id)
    if pida != 0:
        ISIB_PID2PDG = idmap[pida-1]
    else:
        return 0

    isign = lambda a, b: abs(a) if b > 0 else -abs(a)
    
    if sib_id < 0:
        ISIB_PID2PDG = isign(ISIB_PID2PDG,sib_id)
    return ISIB_PID2PDG

def parse(filename):
    tree = ET.parse(filename)
    root = tree.getroot()
        
    for particle in root.iter("particle"):
        name = particle.attrib["name"]
        pdg_id = int(particle.attrib["id"])
        mass = float(particle.attrib["m0"]) # GeV
        electric_charge = int(particle.attrib["chargeType"]) # in units of e/3
        
        decay_width = float(particle.attrib.get("mWidth", 0)) # GeV
        lifetime = float(particle.attrib.get("tau0", math.inf)) # mm / c
        
        yield (pdg_id, name, mass)
                
        # TODO: read decay channels from child elements
        
        if "antiName" in particle.attrib:
            name = particle.attrib['antiName']
            yield (-pdg_id, name, mass)
            

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
        

def build_pythia_db(filename):
    particle_db = OrderedDict()
    
    for (pdg, name, mass) in parse(filename):
        c_id = c_identifier(name)
        
        #~ print(name, c_id, sep='\t', file=sys.stderr)
        #~ enums += "{:s} = {:d}, ".format(c_id, corsika_id)
        particle_db[c_id] = {
            "name" : name,
            "pdg" : pdg,
            "mass" : mass, # in GeV
        }
    
    return particle_db
    

def read_sibyll(filename):
    with open(filename, "rt", newline='') as file:
        reader = csv.reader(file, delimiter=' ')
        for c_id, sib_code in reader:
            yield (c_id, {"sibyll" : int(sib_code)})
            

def gen_convert_sib_int(pythia_db):
    min_sib = min((pythia_db[p]['sibyll'] for p in pythia_db if "sibyll" in pythia_db[p]))
    max_sib = max((pythia_db[p]['sibyll'] for p in pythia_db if "sibyll" in pythia_db[p]))
    
    table_size = max_sib - min_sib + 1
    
    map_sib_int = [None] * table_size
    
    for p in filter(lambda _: True, pythia_db.values()):
        map_sib_int[min_sib + p['sibyll']] = (p['ngc_code'], p["name"])
    
    string = ("constexpr int8_t min_sib = {min_sib:d};\n"
              "\n"
              "constexpr std::array<int16_t, {size:d}> map_sibyll_internal = {{{{\n").format(size = table_size, min_sib = min_sib)
              
    for val in map_sib_int:
        internal, name = (*val,) if val else (0, "UNUSED")
        string += "    {code:d}, // {name:s}\n".format(code = internal, name = name)
    
    string += "}};\n"   
    return string
    
def gen_sibyll_enum(pythia_db):
    string = "enum class SibyllParticleCode : int8_t {\n"
    
    for k in filter(lambda k: "sibyll" in pythia_db[k], pythia_db):
        string += "  {key:s} = {sib:d},\n".format(key = k, sib = pythia_db[k]['sibyll'])
    
    string += "};"
    return string
    

def gen_convert_int_sib(pythia_db):
    map_int_sib_size = len(pythia_db)
    map_int_sib = [None] * map_int_sib_size
    
    for p in pythia_db:
        map_int_sib[pythia_db[p]['ngc_code']] = pythia_db[p]['sibyll'] if "sibyll" in pythia_db[p] else 0

    map_int_sib_table = "constexpr std::array<int8_t, {size:d}> map_internal_sibyll = {{{{".format(size = len(map_int_sib))
    
    for k, p in zip(map_int_sib, pythia_db.values()):
        map_int_sib_table += "  {:d}, // {:s}\n".format(k, p['name'])
        
    map_int_sib_table += "}};"
    
    return map_int_sib_table
    
    
def gen_internal_enum(pythia_db):
    string = "enum class InternalParticleCode : uint8_t {\n"
    
    for k in filter(lambda k: "ngc_code" in pythia_db[k], pythia_db):
        string += "  {key:s} = {sib:d},\n".format(key = k, sib = pythia_db[k]['ngc_code'])
    
    string += "};"
    return string


def gen_properties(pythia_db):
    string = "static constexpr std::size_t size = {size:d};\n".format(size = len(pythia_db))
              
    string += "static constexpr std::array<double const, size> masses{{\n"
              
    for p in pythia_db.values():
        string += "  {mass:f}, // {name:s}\n".format(mass = p['mass'], name = p['name'])
              
    string += ("}};\n"
               "static constexpr std::array<PDGCode const, size> pdg_codes{{\n")
               
    for p in pythia_db.values():
        string += "  {pdg:d}, // {name:s}\n".format(pdg = p['pdg'], name = p['name'])
    
    string += ("}};\n"
               "static const std::array<std::string const, size> names{{\n")

    for p in pythia_db.values():
        string += "  \"{name:s}\",\n".format(name = p['name'])
            
    string += "}};\n"
    
    return string

    
if __name__ == "__main__":
    pythia_db = build_pythia_db("ParticleData.xml")
    
    for c_id, sib_info in read_sibyll("sibyll_codes.dat"):
        #~ print(c_id, sib_info)
        pythia_db[c_id] = {**pythia_db[c_id], **sib_info}
        
    counter = itertools.count(0)
    
    not_modeled = []
    for p in pythia_db:
        if 'sibyll' not in pythia_db[p]:
            not_modeled += [p]
        else:
            pythia_db[p]['ngc_code'] = next(counter)
            
    #~ print(not_modeled)
    for p in not_modeled:
        pythia_db.pop(p, None)
        
    
    #~ # cross check hand-written tables vs sibyll's conversion
    #~ for p in pythia_db:
        #~ sib_db = pythia_db[p]['sibyll']
        #~ pdg = pythia_db[p]['pdg']
        #~ table = sib_to_pdg(sib_db)
        #~ if table != pdg:
            #~ raise Exception(p, sib_db, pdg, table)

    with open("generated_particle_properties.inc", "w") as f:
        print(gen_internal_enum(pythia_db), file=f)
        print(gen_properties(pythia_db), file=f)
    
    with open("generated_sibyll.inc", "w") as f:
        print(gen_sibyll_enum(pythia_db), file=f)
        print(gen_convert_sib_int(pythia_db), file=f)
        print(gen_convert_int_sib(pythia_db), file=f)

    #~ print(pdg_id_table, mass_table, name_table, enums, sep='\n\n')
