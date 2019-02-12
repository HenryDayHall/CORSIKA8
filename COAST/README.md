# COAST interface {#COAST}

With the COAST interface of CORSIKA 8 you can write a CORSIKA 8
"process" class and link it to CORSIKA 7. This can be very powerful to
benchmark new CORSIKA 8 physics code in the context of "old" CORSIKA 7
simulations. 

This is based on corsika7/trunk/coast/CoastOptions/example with an
additional interface to CORSIKA 8. See code in namespace corsika::coast for all details.

We provide a step-by-step example for a "COAST user library" using CORSIKA 8
technology. It explains the steps, how to use the COAST_USER_LIB
option of CORSIKA together with CORSIKA8/COAST


Step 1:
-------

Configure CORSIKA8 with 'cmake -DWITH_COAST=1' and other options you
prefer. You have to define COAST_DIR environment variable to the
location of your existing CORSIKA7 installation. Check that 'ls
$COAST_DIR/include/interface' contains the file
'CorsikaInterface.h'. If this is not the case, this is not a valid or
proper installation of CORSIKA. First compile CORSIKA, e.g. with
ROOTOUT option, or CoReas to get COAST installed!


Step 2:
-------

Compile CORSIKA8, edit COAST/COASTProcess.cc to modify/add your
physics module. 

There should be a libCOAST.so as a result! This is what you need.


Step 3:
-------

Create COAST_USER_LIB environment variable to point at your current
directory, where now the 'libCOAST.so' library is located.
Add the path in $COAST_USER_LIB to your LD_LIBRARY_PATH environment
variable.


Step 4:
-------

Go back to your CORSIKA directory and re-start 'coconut'. The option
COAST_USER_LIB will now be visible. Please select it, copile CORSIKA
and start the executable. In the generated console output you will the
statements from your COAST library during Init and Close of the
simulation.  
Add any kind of code now working on the CParticle or
CInteraction class to start using the full power of COAST.

Note: the default COASTProcess just prints out tons of ASCII. This is
not very useful, don't run a full MC with this...


Step 5:
-------

enjoy...

