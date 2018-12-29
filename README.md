# CORSIKA8 Framework for Particle Cascades in Astroparticle Physics**

Documentation and reference guide for the CORSIKA8 software framework
for air shower simulations. We aim that CORSIKA remains the most comprehensive
framework for simulating particle cascades with stochastic and continuous processes. The purpose of CORSIKA is to 
simulate any particle cascades in astroparticle physics or astrophysical context. A lot of emphasis is put on modularity, flexibility, completeness, validation and correctness. To boost computational efficiency different techniques are
provided, like thinning or cascade equations. 

The software makes extensive use of static design patterns and
compiler optimization. Thus, the most fundamental configuration
decision of the user must be performed at compile time. At run time
only specific model parameters can still be changed.

CORSIKA8 is released under the GPL3 license. See [license file](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/LICENSE) which is part of every release and the source code. 

If you use, or want to refer to, CORSIKA8 please cite ["Towards a Next Generation of CORSIKA: A Framework for the Simulation of Particle Cascades in Astroparticle Physics", Comput.Softw.Big Sci. 3 (2019) 2](https://doi.org/10.1007/s41781-018-0013-0). We kindly ask (and expect) any relevant improvement or addition to be offered or contributed to the main CORSIKA8 repository for the benefit of the whole community. 

When you contribute to CORSIKA check the guidelines outlined here:
[coding guidelines](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/CONTRIBUTING.md). Code that fails the review by the CORSIKA author group must be improved before it can be merged in the official code base. After your code has been accepted and merged you become a contributor of the CORSIKA project and you should include yourself in the [AUTHORS](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/AUTHORS) file. 

IMPORTANT: Before your contribute, you need to read and agree to the [collaboration agreement](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/COLLABORATION_AGREEMENT.md). The agreement can be discussed, and eventually improved. 

We also want to point you to the [MCnet guidelines](https://gitlab.ikp.kit.edu/AirShowerPhysics/corsika/blob/master/MCNET_GUIDELINES), which are very useful also for us. 

## Installation

CORSIKA8 is tested regularly at least on gcc7.3.0 and clang-6.0.0.
Additional software prerequisites: eigen3, boost, cmake, g++, git. 
On a bare Ubuntu 18.04, just add:
```
sudo apt-get install libeigen3-dev libboost-dev cmake g++ git
```

Follow these steps to download and install CORSIKA8-milestone2
```
git clone git@gitlab.ikp.kit.edu:AirShowerPhysics/corsika.git
cd corsika
git checkout milestone2
mkdir ../corsika-build
cd ../corsika-build
cmake ../corsika -DCMAKE_INSTALL_PREFIX=../corsika-install
make -j8
make install
make test
```
and if you want to see how the first simple hadron cascade develops, see `Documentation/Examples/cascade_example.cc` for a starting point. 

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

### Generating doxygen documentation

To generate the documentation, you need doxygen and graphviz. On Ubuntu 18.04, do:
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
