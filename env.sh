#!/bin/bash

# Usage: source env.sh

WORKDIR=`pwd`
RESULTSDIR=$WORKDIR/X_RESULTS
G4DIR=$([ -z ${G4INSTALL+x} ] && echo "$G4INSTALL" || echo "/cvmfs/geant4.cern.ch/geant4/10.2/x86_64-slc6-gcc49-opt/lib64/Geant4-10.2.0/")

SCRATCHDIR=$WORKDIR/BASHScript
DATADIR=$WORKDIR/SourceFiles
GEANTDIR=$WORKDIR/geant4

if [ $RESULTSDIR = $WORKDIR/X_RESULTS ]; then
	echo "Your results directory is still set to X_RESULTS. You should modify env.sh to point to the directory you want results to appear in."
fi
