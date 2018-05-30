#!/bin/bash

# In order for this to work, pick a HOME directory, and inside that path, put in milliq (from git), SourceFiles, LaptopResults. 
# Also point it to where G4 is installed.


#Paths Laptop
HOME=/home/gregory/work/milliq/simulation/MilliQ/milliQan
RESULTS=$HOME/LaptopResults #Save results location
G4DIR="/cvmfs/geant4.cern.ch/geant4/10.2/x86_64-slc6-gcc49-opt/lib64/Geant4-10.2.0/" #Geant4 Build/lib/Geant4 location 


#Paths Cluster
#HOME=/xfs1/gmagill/Repository_MilliCharged/milliQan
#RESULTS=$HOME/ClusterResults #Save results location
#G4DIR="$HOME""/../../geant4.10.02-build/lib/Geant4-10.2.0/" #Geant4 Build/lib/Geant4 location



#Sets these automatically
SCRATCH=$HOME/BASHScript #SCRATCH Location
DATA=$HOME/SourceFiles #Input Kinematic Distributions Location
GEANT=$HOME/geant4 #milliq Save location

