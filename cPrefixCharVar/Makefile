# ************************************************************************************
# *******  Users SHOULD modify the makefile variables at the top of this file ********
# ************************************************************************************

# Set variable ROSE_INSTALL to your ROSE_INSTALL installation like this.
# ROSE_INSTALL = /home/demo/ROSE/ROSE_CompileTree/svn_LINUX-64bit-4.2.2_install

# If ROSE is configured with Dwarf support then we need libdwarf.a
# ROSE_OPTIONAL_LIBS = /home/demo/ROSE/DWARF/dwarf-20081013/libdwarf/libdwarf.a -lelf

# A version of libcompass.so will be built locally to allow the checker to run
LOCAL_COMPASS_LIB = -L`pwd` -Wl,-rpath `pwd` -lcompass 

# Link syntax required for ROSE (using instaled version of ROSE)
ROSE_LIB = -L$(ROSE_INSTALL)/lib -Wl,-rpath $(ROSE_INSTALL)/lib -lrose 

# Specify specific path to find libstdc++.so (required when not using the default system compiler)
# This is required at least on the LLNL computers using non-default versions of GNU compilers.
# PATH_TO_SPECIFIC_LIBSTDCXX = -Wl,--rpath -Wl,/usr/apps/gcc/4.2.2-64bit/lib64

# Boost is required in ROSE, so we need the path.
BOOST_INCLUDE_PATH = /home/demo/local/boost_1_37_0_installTree-gxx-4.2.2/include/boost-1_37

# If ROSE is configured with optional libs then we need the associated include paths.
# ROSE_OPTIONAL_LIBS_INCLUDE = -I/home/demo/ROSE/DWARF/dwarf-20081013/libdwarf

# ************************************************************************************
# **********  Users should NOT have to modify the makefile below this point **********
# ************************************************************************************

CHECKER_NAME = testChecker

CXX_FLAGS = -I$(ROSE_INSTALL)/include $(ROSE_OPTIONAL_LIBS_INCLUDE) -I$(BOOST_INCLUDE_PATH)

LINKER_FLAGS = $(ROSE_LIB) $(ROSE_OPTIONAL_LIBS) $(LOCAL_COMPASS_LIB) $(PATH_TO_SPECIFIC_LIBSTDCXX)

all: cPrefixCharVarTest

libcompass.so: compass.h compass.C
	g++ -fPIC -Wall -shared -o libcompass.so compass.C $(CXX_FLAGS)

cPrefixCharVarTest: cPrefixCharVar.C cPrefixCharVarMain.C libcompass.so
	g++ -fPIC -Wall -o cPrefixCharVarTest cPrefixCharVarMain.C cPrefixCharVar.C $(CXX_FLAGS) $(LINKER_FLAGS)

test: cPrefixCharVarTest cPrefixCharVarTest1.C
	./cPrefixCharVarTest cPrefixCharVarTest1.C

clean:
	-rm -f libcompass.so "cPrefixCharVarTest"

