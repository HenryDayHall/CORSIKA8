# CORSIKA 8 Framework for Particle Cascades in Astroparticle Physics 

The purpose of CORSIKA is to simulate any particle cascades in
astroparticle physics or astrophysical context. A lot of emphasis is
put on modularity, flexibility, completeness, validation and
correctness. To boost computational efficiency different techniques
are provided, like thinning or cascade equations. The aim is that
CORSIKA remains the most comprehensive framework for simulating
particle cascades with stochastic and continuous processes.

The software makes extensive use of static design patterns and
compiler optimization. Thus, the most fundamental configuration
decision of the user must be performed at compile time. At run time
model parameters can still be changed.

CORSIKA 8 is by default released under the GPLv3 license. See [license
file](https://gitlab.iap.kit.edu/AirShowerPhysics/corsika/blob/master/LICENSE)
which is part of every release and the source code.

If you use, or want to refer to, CORSIKA 8 please cite ["Towards a Next
Generation of CORSIKA: A Framework for the Simulation of Particle
Cascades in Astroparticle Physics", Comput.Softw.Big Sci. 3 (2019)
2](https://doi.org/10.1007/s41781-018-0013-0). We kindly ask (and
require) any relevant improvement or addition to be offered or
contributed to the main CORSIKA 8 repository for the benefit of the
whole community.

When you plan to contribute to CORSIKA 8 check the guidelines outlined here:
[coding
guidelines](https://gitlab.iap.kit.edu/AirShowerPhysics/corsika/blob/master/CONTRIBUTING.md). Code
that fails the review by the CORSIKA author group must be improved
before it can be merged in the official code base. After your code has
been accepted and merged, you become a contributor of the CORSIKA 8
project (code author). 

IMPORTANT: Before you contribute, you need to read and agree to the
[collaboration
agreement](https://gitlab.iap.kit.edu/AirShowerPhysics/corsika/blob/master/COLLABORATION_AGREEMENT.md). The agreement can be discussed, and eventually improved.

We also want to point you to the [MCnet
guidelines](https://gitlab.iap.kit.edu/AirShowerPhysics/corsika/blob/master/MCNET_GUIDELINES), which are very useful also for us.


## Get in contact
  * Connect to https://gitlab.iap.kit.edu register yourself and join the "Air Shower Physics" group. Write to one of the steering comittee members (https://gitlab.iap.kit.edu/AirShowerPhysics/corsika/-/wikis/Steering-Committee) in case there are problems with that. 
  * Connect to corsika-devel@lists.kit.edu (self-register at
    https://www.lists.kit.edu/sympa/subscribe/corsika-devel) to get in
    touch with the project.
    
  * Register on the corsika slack channel: https://corsika.slack.com


## Installation

CORSIKA 8 is tested regularly at least on `gcc11.0.0` and `clang-14.0.0`. 

### Prerequisites

You will also need:

- Python 3 (supported versions are Python >= 3.6), with pip
- conan (via pip)
- cmake 
- git
- g++, gfortran, binutils, make
- optional: FLUKA (see below)

On a bare Ubuntu 20.04, just add:
``` shell
sudo apt-get install python3 python3-pip cmake g++ gfortran git doxygen graphviz
```

On a bare CentOS 7 install python3, pip3 (pip from python3) and cmake3. Any of the devtools 7, 8, 9 should work (at least). 
Also initialize devtools, before building CORSIKA 8:
``` shell
source /opt/rh/devtoolset-9/enable
```

CORSIKA 8 uses the [conan](https://conan.io/) package manager to
manage our dependencies. Currently, version 1.55.0 or higher is required.
If you do not have Conan installed, it can be
installed with:

``` shell
pip install --user conan~=1.57.0
```

### Compiling

Once Conan is installed, follow these steps to download and install CORSIKA 8:

``` shell
git clone --recursive https://gitlab.iap.kit.edu/AirShowerPhysics/corsika.git
mkdir corsika-build
cd corsika-build
../corsika/conan-install.sh
cmake ../corsika -DCMAKE_BUILD_TYPE="RelWithDebInfo" -DCMAKE_INSTALL_PREFIX=../corsika-install
make -j4  #The number should match the number of available cores on your machine
make install
```

### FLUKA support

Warning: may only work when the next version of FLUKA is released (as of 2023.06.15)

For legal reasons we do not distribute/bundle FLUKA together with CORSIKA 8.
If you want to use FLUKA as low-energy hadronic interaction model, you have to download
it separately from (http://www.fluka.org/), which requires registering there as FLUKA user.
You need to download binaries suitable for your system (check compiler and glibc versions)
and unpack the binaries into a directory of your choice. You also need the data archive
(fluka20xy.z-data.tar.gz) unpacked in the same directory. To compile CORSIKA 8 with FLUKA,
you need to specify the path of libflukahp.a when invoking cmake with the option
`-DC8_FLUKALIB=<path>`. CMake will print a status message indicating whether FLUKA support
is enabled or disabled when the library is (not) found.


## Installation (using docker containers)

There are docker containers prepared that bring all the environment and packages you need to run CORSIKA. See [docker hub](https://hub.docker.com/repository/docker/corsika/devel) for a complete overview. 

### Prerequisites

You only need docker, e.g. on Ubunut: `sudo apt-get install docker` and of course root access.

## Compiling

Follow these steps to download and install CORSIKA 8, master development version
```shell
git clone --recursive https://gitlab.iap.kit.edu/AirShowerPhysics/corsika.git
sudo docker run -v $PWD:/corsika -it corsika/devel:clang-8 /bin/bash
mkdir build
cd build
../corsika/conan-install.sh
cmake ../corsika -DCMAKE_INSTALL_PREFIX=../corsika-install
make -j4  #The number should match the number of available cores on your machine
make install
```

## Runing Unit Tests

To run the Unit Tests, just type `ctest` in your build area.


## Running examples

### Building the examples

From your top corsika build directory, (the one that includes `corsika-build` and `corsika-install`) type
```shell
cmake -Dcorsika_DIR=$PWD/corsika-build -S ./corsika/examples -B ./corsika-build-examples
cd corsika-build-examples
make -j4 #The number should match the number of available cores on your machine
```

From any directory, run the program `corsika-build-examples/bin/corsika`. As an example, you can run with the following flags:
```shell
corsika-build-examples/bin/corsika  --pdg 2212 -E 1e5 -f my_shower
```
This will run a vertical 100 TeV proton shower and will create and put the output into `./my_shower`.


### Generating doxygen documentation

To generate the documentation, you need doxygen and graphviz. If you work with 
the docker corsika/devel containers this is already included. 
Otherwise, e.g. on Ubuntu 18.04, do:
```shell
sudo apt-get install doxygen graphviz
```
Switch to the corsika build directory and do
```shell
make docs
make install
```
open with firefox:
```shell
firefox ../corsika-install/share/corsika/doc/html/index.html
```

