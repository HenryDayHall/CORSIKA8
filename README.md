# CORSIKA8 Framework for Particle Cascades in Astroparticle Physics**

Documentation and reference guide for the CORSIKA8 software framework
for air shower simulations. We aim that CORSIKA remains the most comprehensive
framework for simulating particle cascades with stochastic and continuous processes. The purpose of CORSIKA is to 
simulate any particle cascades in astroparticle physics or astrophysical context. A lot of emphasis is put on modularity, completeness, validation and correctness. To boost computational efficiency different techniques are
provided, like thinning or cascade equations. 

The software makes extensive use of static design patterns and
compiler optimization. Thus, the most fundamental configuration
decision of the user must be performed at compile time. At run time
only specific model parameters can still be changed.

CORSIKA8 is released under the GPL3 license. See [license file](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/LICENSE) which is part of every release and the source code. 

When you contribute to CORSIKA check the guidelines outlined here:
[coding guidelines](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/GUIDELINES.md). Code that fail the review by the CORSIKA author group must be improved before it can be merged in the official code base. After your code has been accepted and merged you become a contributor of the CORSIKA project and you should include yourself in the [AUTHORS](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/AUTHORS) file. 

You need to read and agree to the [collaboration agreement](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/COLLABORATION_AGREEMENT.md). The agreement can be discussed, and eventually improved. 

We also want to point you to the [MCnet guidelines](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/MCNET_GUIDELINES), which are very useful also for us. 

## Installation

Prerequisites: eigen3, cmake, g++, git. On Ubuntu 18.04, just do:
```
sudo apt-get install libeigen3-dev cmake g++ git
```

Follow these steps to download and install CORSIKA8-milestone1
```
git clone git@gitlab.ikp.kit.edu:AirShowerPhysics/corsika.git
cd corsika
git checkout milestone1
mkdir ../corsika-build
cd ../corsika-build
cmake ../corsika -DCMAKE_INSTALL_PREFIX=../corsika-install
make -j8
make install
make test
```
and if you want to see how the Heitler model works and is implemented, see `Framework/Cascade/testCascade.cc` for a starting point. 
