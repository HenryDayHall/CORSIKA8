#!/usr/bin/env python3

import sys, math, itertools, re, csv, pprint
from collections import OrderedDict
import io


##############################################################
# 
# reading input data, return media properties data
# c.f. https://pdg.lbl.gov/2020/AtomicNuclearProperties/Properties8_Key.pdf
# 
def parseData(filename):
    
    with open(filename) as f:

        line = f.readline()
        while (line.strip() != ""):
            
            entry = {}
            
            # main material data line

            entry["sternheimer_index"] = int(line[:5])
            entry["sternheimer_label"] = line[5:10].strip()
            index = 0
            words = line[11:].split()
            entry["sign_figures_weight"] = int(words[index])
            index += 1
            entry["weight"] = float(words[index])
            index += 1
            entry["error_last_digit"] = 0
            if (entry["weight"]>0):
                entry["error_last_digit"] = int(words[index])
                index += 1
            entry["Z_over_A"] = float(words[index])
            index += 1
            entry["sternheimhers_density"] = float(words[index])
            index += 1
            entry["corrected_density"] = float(words[index])
            index += 1
            entry["state"] = words[index] # Solid, Liquid, Gas, Diatomic gas
            index += 1
            entry["num_elements"] = int(words[index])
            index += 1
            entry["num_atoms_1"] = int(words[index])
            index += 1
            entry["num_extra_lines"] = int(words[index])
            index += 1
            entry["type"] = words[index] # Element, Radioactive, Inorg. comp., Org. comp., Polymer, Mixture, Biological/dosimetry
            index += 1

            if (index != len(words)):
                print ("error in line: " + line + " index=" + str(index) + " len=" + str(len(words)))
                sys.exit(1)

            # name line
            line = f.readline()
            words = line.split()
            index = 0
            if (entry["type"] in ["E", "R"] ):
                entry["symbol"] = words[index]
                index += 1
            entry["name"] = words[index]
            index += 1

            if (index != len(words)):
                print ("error in line: " + line + " index=" + str(index) + " len=" + str(len(words)))
                sys.exit(1)

            # name+formula line
            line = f.readline()
            entry["name_and_formula"] = line.strip()


            # ionization data
            line = f.readline()
            words = line.split()
            index = 0
            entry["Ieff"] = float(words[index])
            index += 1
            entry["Cbar"] = float(words[index])
            index += 1
            entry["x0"] = float(words[index])
            index += 1
            entry["x1"] = float(words[index])
            index += 1
            entry["aa"] = float(words[index])
            index += 1
            entry["sk"] = float(words[index])
            index += 1
            entry["dlt0"] = float(words[index])
            index += 1

            if (index != len(words)):
                print ("error in line: " + line + " index=" + str(index) + " len=" + str(len(words)))
                sys.exit(1)


            for i in range(entry["num_elements"]):

                elem = "element_{}".format(i)
                if (not elem in entry):
                    entry[elem] = {}
                
                line = f.readline()
                words = line.split()
                index = 0
                entry[elem]["Z"] = int(words[index])
                index += 1
                entry[elem]["Num_frac"] = float(words[index])
                index += 1
                entry[elem]["weight_frac"] = float(words[index])
                index += 1

                if (index != len(words)):
                    print ("error in line: " + line + " index=" + str(index) + " len=" + str(len(words)))
                    sys.exit(1)

                
            skip = False
            for i in range(entry["num_extra_lines"]):

                # optional lines
                line = f.readline()
                if (skip):
                    continue
                key = line[:5]
                if (key == "     " or
                    key == "Note:"):
                    skip = True
                    continue
                n_empty = 0
                comment = key
                c = 6
                while (c<=25):
                    comment += line[c]
                    c += 1
                    if (line[c].strip() == ''):
                        n_empty += 1
                    else:
                        n_empty = 0
                    if (n_empty > 3):
                        break
                    
                value = float(line[c:45])
                if (not "properties" in entry):
                    entry["properties"] = {}
                entry["properties"][key] = value;

            line = f.readline() # move to separator line "---------------"
            line = f.readline() # move to next entry
            yield entry


def TypeEnum(type):
    if type=='E':
        return "Element"
    elif type=='R':
        return "RadioactiveElement"
    elif type=='I':
        return 'InorganicCompound'
    elif type=='O':
        return "OrganicCompound"
    elif type=="P":
        return "Polymer"
    elif type=='M':
        return "Mixture"
    elif type=="B":
        return "BiologicalDosimetry"
    else:
        return "Unkonwn"

def StateEnum(state):
    if state=='S':
        return "Solid"
    elif state=='L':
        return "Liquid"
    elif state=='G':
        return 'Gas'
    elif state=='D':
        return "DiatomicGas"
    else:
        return "Unkonwn"


def ClassName(name):
    name = name.replace('-', '_')
    words = name.split('_')
    str = ""
    for w in words:
        str += w.capitalize()
    if str[0].isdigit():
        str = "_" + str
    return str

    

    
##########################################################
# 
# returns dict containing all data from pythia-xml input
#     
def read_data(filename):

    data = []
    counter = 0
    
    for entry in parseData(filename):
        data.append(entry)

    return data
    

###############################################################
# 
# return string with a list of classes for all particles
# 
def gen_code(media_db):

    string = """
  // enum for all media
  enum class Medium : MediumIntType {
    Unkown, 
"""

    imedium = 0
    for entry in media_db:
        cname = ClassName(entry["name"])
        string += "    {} = {} ,\n".format(cname, imedium)
        imedium += 1
    string += "    {} = {} ,\n".format("First", 0)
    string += "    {} = {} ,\n".format("Last", imedium-1)
    string += "  };\n\n"        
    return string


###############################################################
# 
# return string with a list of classes for all particles
# 
def gen_classes(media_db):

    string = """
  // list of C++ classes to access media properties"

//  typedef std::map<std::string, double> Properties;
//  typedef std::array<Properties, static_cast<MediumIntType>(Medium::Last)+1> Constituents; this is wrong> num_elements

    """
    
    for entry in media_db:

        cname = ClassName(entry["name"])

        constituents = "{"
        comma = ""
        for i in range(entry["num_elements"]):
            elem = "element_{}".format(i)
            constituents += "{}{{ {{ {{\"Z\", {} }}, {{\"NumFrac\", {} }}, {{\"WeightFrac\", {} }} }} }}".format(comma, entry[elem]["Z"],
                                                                                                           entry[elem]["Num_frac"],
                                                                                                           entry[elem]["weight_frac"])
            comma = ", "
        constituents += "}"

        properties = "{"
        if "properties" in entry:
            comma = ""        
            for k,v in entry["properties"].items():
                properties += "{}{{ \"{}\", {} }}".format(comma, k, v)
                comma = ", "
        properties += "}"

        symbol = "Unknown";
        if ("symbol" in entry):
            symbol = entry["symbol"]

            
        class_string = """
  /** 
   * \class {cname}
   *
   * Media properties from properties8.dat file from NIST:
   *  - Sternheimer index {stern_index}, label {stern_label}
  **/

  class {cname} {{
    public:
     static constexpr Medium medium() {{ return Medium::{cname}; }}

     static std::string const name() {{ return data_.name(); }}
     static std::string const pretty_name() {{ return data_.pretty_name(); }}
     static double weight() {{ return data_.weight (); }}
     static int weight_significant_figure() {{ return data_.weight_significant_figure (); }}
     static int weight_error_last_digit() {{ return data_.weight_error_last_digit(); }}
     static double Z_over_A() {{ return data_.Z_over_A(); }}
     static double sternheimer_density() {{ return data_.sternheimer_density(); }}
     static double corrected_density() {{ return data_.corrected_density(); }}
     static State state() {{ return data_.state(); }}
     static MediumType type() {{ return data_.type(); }}
     static std::string const symbol() {{ return data_.symbol(); }}

     static double Ieff() {{ return data_.Ieff(); }}
     static double Cbar() {{ return data_.Cbar(); }}
     static double x0() {{ return data_.x0(); }}
     static double x1() {{ return data_.x1(); }}
     static double aa() {{ return data_.aa(); }}
     static double sk() {{ return data_.sk(); }}
     static double dlt0() {{ return data_.dlt0(); }}

     //static constexpr Constituents constituents() {{ return {constituents}; }}
     //static constexpr Properties properties() {{ return {properties}; }}

     inline static const MediumData data_ {{ "{name}", "{nice_name}", {weight},
     {weight_significant_figure}, {weight_error_last_digit}, {Z_over_A},
     {sternheimer_density}, {corrected_density}, State::{state},
     MediumType::{type}, "{symbol}", {Ieff}, {Cbar}, {x0}, {x1}, {aa}, {sk}, {dlt0} }};
  }};

        """.format(cname=cname,
                   stern_label=entry["sternheimer_label"],
                   stern_index=entry["sternheimer_index"],
                   weight_significant_figure=entry["sign_figures_weight"],
                   weight=entry["weight"],
                   weight_error_last_digit=entry["error_last_digit"],
                   Z_over_A = entry["Z_over_A"],
                   sternheimer_density = entry["sternheimhers_density"],
                   corrected_density = entry["corrected_density"],
                   state=StateEnum(entry["state"]),
                   type=TypeEnum(entry["type"]),
                   symbol=symbol,
                   name=entry["name"],
                   nice_name=entry["name_and_formula"].replace('\\','\\\\'),
                   Ieff=entry["Ieff"],
                   Cbar=entry["Cbar"],
                   x0=entry["x0"],
                   x1=entry["x1"],
                   aa=entry["aa"],
                   sk=entry["sk"],
                   dlt0=entry["dlt0"],
                   constituents=constituents,
                   properties=properties);


     # static std::string const name() {{ return "{name}"; }}
     # static std::string const pretty_name() {{ return "{nice_name}"; }}
     # static constexpr double weight() {{ return {weight} ; }}
     # static constexpr int weight_significant_figure() {{ return {weight_significant_figure} ; }}
     # static constexpr int weight_error_last_digit() {{ return {weight_error_last_digit}; }}
     # static constexpr double Z_over_A() {{ return {Z_over_A}; }}
     # static constexpr double sternheimer_density() {{ return {sternheimer_density}; }}
     # static constexpr double corrected_density() {{ return {corrected_density}; }}
     # static constexpr State state() {{ return State::{state}; }}
     # static constexpr MediumType type() {{ return MediumType::{type}; }}
     # static std::string const symbol() {{ return "{symbol}"; }}

     # static constexpr double Ieff() {{ return {Ieff}; }}
     # static constexpr double Cbar() {{ return {Cbar}; }}
     # static constexpr double x0() {{ return {x0}; }}
     # static constexpr double x1() {{ return {x1}; }}
     # static constexpr double aa() {{ return {aa}; }}
     # static constexpr double sk() {{ return {sk}; }}
     # static constexpr double dlt0() {{ return {dlt0}; }}


        string += class_string


#         private:\n"
#           static constexpr CodeIntType TypeIndex = static_cast<CodeIntType const>(Type);\n"
    return string

###############################################################
# 
# return string with a list of classes for all particles
# 
def gen_data_array(media_db):

    string = """
  // array of MediumData entries
  static const std::array<const MediumData, static_cast<MediumIntType>(Medium::Last)+1> medium_data = {
    """

    comma=""
    for entry in media_db:
        cname = ClassName(entry["name"])
        string += "{}    {}::data_".format(comma, cname)
        comma = ",\n"
    string += "  };\n\n"
    return string

###############################################################
# 
# return string with a list of classes for all particles
# 
def gen_media_map(media_db):

    string = """
  // mapping of enum Medium to Medium classes
  static auto MediumMap = std::make_tuple(
    """
    comma=""
    for entry in media_db:
        cname = ClassName(entry["name"])
        string += "{}    {}()".format(comma, cname)
        comma = ",\n"
    string += "  );\n\n"        
    return string


###############################################################
# 
# 
def inc_start():
    string = """
// generated by readProperties.py
// MANUAL EDITS ON OWN RISK. THEY WILL BE OVERWRITTEN. 
// since this is automatic code, we don't attempt to generate automatic unit testing, too: LCOV_EXCL_START 
namespace corsika::environment {

"""
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
    string = """
\n} // end namespace corsika::environment
// since this was automatic code, we didn't attempt to generate automatic unit testing, too: LCOV_EXCL_STOP
"""
    return string


    
###################################################################
# 
# Main function
# 
if __name__ == "__main__":
    
    if len(sys.argv) != 2:
        print("usage: {:s} <properties8.dat>".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)
        
    print("\n       readProperties.py: automatically produce media properties from input files\n")
    
    media_db = read_data(sys.argv[1])
    
    with open("GeneratedMediaProperties.inc", "w") as f:
        print(inc_start(), file=f)
        print(gen_code(media_db), file=f)
        print(gen_classes(media_db), file=f)
        print(detail_start(), file=f)
        print(gen_data_array(media_db), file=f)
        print(detail_end(), file=f) 
        print(inc_end(), file=f) 
    

