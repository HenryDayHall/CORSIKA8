#!/usr/bin/env python3

import os
import sys
import shutil

if (not os.path.exists('bin/pythia8-config') or
    not os.path.exists('include/Pythia8/Pythia.h') or
    not os.path.exists('src/Event.cc')) :
    print ("You can only run this script from within an unpacked PYTHIA tar filedirectory. This was not detected!")
    sys.exit(1)

# find version
version = 'undefined'
pythia_header = open('include/Pythia8/Pythia.h')
for line in pythia_header:
    line_list = line.strip().split(' ')
    if (len(line_list) == 3 and
        line_list[0] == '#define' and
        line_list[1] == 'PYTHIA_VERSION_INTEGER') :
        version = line_list[2]
        break
pythia_header.close()

if (version == 'undefined'):
    print ("Could not find Pythia version info")
    sys.exit(2)

print ("Pythia version: {}".format(version))

if (os.path.exists('c8_package')) :
    print ('Temporary directory \"c8_package\" already exists. Cleanup first.')
    sys.exit(3)

tmpdir = 'c8_package/pythia{}'.format(version)
print ('create \"{}\"'.format(tmpdir))
os.makedirs(tmpdir)

copy_files_s = "AUTHORS  bin  CODINGSTYLE  configure  COPYING  GUIDELINES  include  Makefile  Makefile.inc  README  share  src"
copy_files = copy_files_s.split()

for filename in os.listdir('./') :
    file = os.path.join('./', filename)
    fileNew = os.path.join(tmpdir, filename)
    if (filename in copy_files) :
        isdir = ''
        if (os.path.isdir(file)): isdir = ' [dir]'
        print ('Keep \"{}\"{}'.format(file, isdir))
        if (os.path.isfile(file)):
            shutil.copy(file, tmpdir)
        else:
            shutil.copytree(file, fileNew)

print ('Remove \"include/Pythia8Plugins\"')
plugindir = os.path.join(tmpdir, 'include/Pythia8Plugins')
if (os.path.exists(plugindir)):
    shutil.rmtree(plugindir)
    
# remove anything  in share/Pythia but xmldoc that contains parameter definitions \"<parm\"
sharedir = os.path.join(tmpdir, 'share/Pythia8')
if (not os.path.isdir(sharedir)) :
    print ('Why there is no \"share/Pythia8\" directory?')
else :
    print ('Only keep \"xmldoc\" inside \"share/Phythia8\"')

    for filename in os.listdir(sharedir) :
        file = os.path.join(sharedir, filename)
        if (os.path.isdir(file)) :
            if (not 'xmldoc' in file) :
                print ('Remove \"{}\"'.format(file))
                shutil.rmtree(file)
            else :
                cleanupXMLDOC = False
                if (cleanupXMLDOC) :
                    for filename_xml in os.listdir(file) :
                        xml = os.path.join(file, filename_xml)
                        xml_data = open(xml)
                        keep = False
                        for xml_line in xml_data:
                            if ('<parm ' in xml_line) :
                                keep = True
                                break
                        xml_data.close()
                        if (not keep):
                            print ('Removing {}'.format(xml))
                            os.unlink(xml)

# ... 'examples' subdirectory must exist ...
os.mkdir(os.path.join(tmpdir, 'examples'))

print ('Create tar file \"pythia{version}-stripped.tar.bz2\"'.format(version=version))
os.system('cd c8_package && tar cjf pythia{version}-stripped.tar.bz2 pythia{version} && mv pythia{version}-stripped.tar.bz2 ..'.format(version=version))
os.system('md5sum pythia{version}-stripped.tar.bz2'.format(version=version))

# cleanup
shutil.rmtree('c8_package')


