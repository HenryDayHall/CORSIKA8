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
only specific model parameters can still be changed.

CORSIKA 8 is by default released under the GPLv3 license. See [license
file](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/LICENSE)
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
guidelines](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/CONTRIBUTING.md). Code
that fails the review by the CORSIKA author group must be improved
before it can be merged in the official code base. After your code has
been accepted and merged you become a contributor of the CORSIKA 8
project (code author). 

IMPORTANT: Before you contribute, you need to read and agree to the
[collaboration
agreement](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/COLLABORATION_AGREEMENT.md). The
agreement can be discussed, and eventually improved.

We also want to point you to the [MCnet
guidelines](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/MCNET_GUIDELINES),
which are very useful also for us.


## Get in contact
  * Connect to https://gitlab.ikp.kit.edu register yourself and join the "Air Shower Physics" group
  * Connect to corsika-devel@lists.kit.edu (self-register at
    https://www.lists.kit.edu/sympa/subscribe/corsika-devel) to get in
    touch with the project



## Installation (from source)

### Prerequisites

CORSIKA 8 is tested regularly via gitlab-CI using recent gcc and clang
versions.  Additional software prerequisites: cmake, g++, git.
Furthermore, eigen3, boost, catch2, spdlog are shipped in the
ThirdParty directory, so an installation on the system is optional.
Also Pythia 8, CONEX and PROPOSAL are distributed in the ThirdParty
folder. You may also install those packages on your system and use
those; we test with Pythia version 8.235.

On a bare Ubuntu 18.04, just add:
```
sudo apt install cmake g++ git
```
add ```libeigen3-dev``` if you want to use system version of eigen3.

If you work with FreeBSD, run:
```
pkg install git cmake python3 flang
```
or add ```boost-libs``` and ```eigen``` if you want to use the system versions. 

### Compiling

Follow these steps to download and install CORSIKA 8, master development version
```
git clone --recursive https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika.git
cd corsika
mkdir ../corsika-build
cd ../corsika-build
cmake ../corsika -DCMAKE_INSTALL_PREFIX=../corsika-install
make -j8
make install
```

Type `make test` to run the unit test suite.

## Installation (using docker containers)

There are docker containers prepared that bring all the environment and packages you need to run CORSIKA. See [docker hub](https://hub.docker.com/repository/docker/corsika/devel) for a complete overview. 

### Prerequisites

You only need docker, e.g. on Ubunut: `sudo apt-get install docker` and of course root access.

## Compiling

Follow these steps to download and install CORSIKA 8, master development version
```
git clone --recursive https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika.git
cd corsika
sudo docker run -v $PWD:/corsika -it corsika/devel:clang-8 /bin/bash
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install
make -j8
make install
```

Type `make test` to run the unit test suite.

## Running, Examples

There are various examples in the folder `Documentation/Examples`. 

If you want to see how the first simple hadron cascade develops, 
see `Documentation/Examples/cascade_example.cc` for a starting point. 

Run the cascade_example with: 
```
cd ../corsika-install
share/examples/cascade_example
```

Visualize output (needs gnuplot installed): 
```
bash share/tools/plot_tracks.sh tracks.dat 
firefox tracks.dat.gif 
```

Or also consider the `vertical_EAS` example in the same directory, which can 
be configured with command line options. 


### Generating doxygen documentation

To generate the documentation, you need doxygen and graphviz. If you work with the docker corsika/devel containers this is already included. 
Otherwise, e.g. on Ubuntu 18.04, do:
```
sudo apt-get install doxygen graphviz
```
Switch to the corsika build directory and do
```
make doxygen
make install
```
browse with firefox:
```
firefox ../corsika-install/share/doc/html/index.html
```

