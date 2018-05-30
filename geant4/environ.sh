#!/bin/bash
USERBASE=`pwd`
ARCH=x86_64-slc6-gcc49-opt

alias root=$USERBASE/myROOT.sh
    
# GCC 4.9.3
GCC_ROOT="/cvmfs/sft.cern.ch/lcg/contrib/gcc/4.9.3/x86_64-slc6"
GCC_VERSION="4.9.1"
GCC_REVISION="1"
GCC_CATEGORY="lcg"
export PATH="/cvmfs/sft.cern.ch/lcg/contrib/gcc/4.9.3/x86_64-slc6/bin:$PATH"
export LD_LIBRARY_PATH="/cvmfs/sft.cern.ch/lcg/contrib/gcc/4.9.3/x86_64-slc6/lib64:$LD_LIBRARY_PATH"

# geant4
cwd=$(pwd)
#geant4dir="/cvmfs/geant4.cern.ch/geant4/10.4.p02/x86_64-centos7-gcc63-opt-MT"
geant4dir="/cvmfs/geant4.cern.ch/geant4/10.2/x86_64-slc6-gcc49-opt"
source $geant4dir/CMake-setup.sh
cd $geant4dir/bin
source geant4.sh
cd $cwd

# QT
export QTHOME=/cvmfs/sft.cern.ch/lcg/external/qt/4.8.4/x86_64-slc6-gcc48-opt
export QTLIBPATH=$QTHOME/lib

# CMake
CMAKE_ROOT="/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.3.2/Linux-x86_64"
CMAKE_VERSION="3.3.2"
CMAKE_REVISION="1"
CMAKE_CATEGORY="lcg"
export PATH="/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.3.2/Linux-x86_64/bin:$PATH"
export LD_LIBRARY_PATH="/cvmfs/sft.cern.ch/lcg/contrib/CMake/3.3.2/Linux-x86_64/lib:$LD_LIBRARY_PATH"

# expat
EXPAT_ROOT="/cvmfs/sft.cern.ch/lcg/external/expat/2.0.1/x86_64-slc6-gcc48-opt"
EXPAT_VERSION="2.0.1"
EXPAT_REVISION="1"
EXPAT_CATEGORY="lcg"
export PATH="/cvmfs/sft.cern.ch/lcg/external/expat/2.0.1/x86_64-slc6-gcc48-opt/bin:$PATH"
export LD_LIBRARY_PATH="/cvmfs/sft.cern.ch/lcg/external/expat/2.0.1/x86_64-slc6-gcc48-opt/lib:$LD_LIBRARY_PATH"

# bz2, zlib
source /cvmfs/cms.cern.ch/slc6_amd64_gcc491/external/bz2lib/1.0.5-cms/etc/profile.d/init.sh
source /cvmfs/cms.cern.ch/slc6_amd64_gcc491/external/zlib/1.2.8-cms/etc/profile.d/init.sh

# ROOT
source /cvmfs/cms.cern.ch/slc6_amd64_gcc491/lcg/root/6.02.12/etc/profile.d/init.sh
