#!/bin/bash

# Set Paths
source paths.mCP.sh

# Set arguments
charge=$1
mass=$2
nEv=$3
proc=$4
configFile=$5
nCores=$6
particle=$7
yRescale=$8
zRescale=$9

mac=true
linux=false


if [ $mac = true ]; then
    sedcommand="sed -i ''"
fi
if [ $linux = true ]; then
    sedcommand="sed -i"
fi 


#Pick correct distribution files
if [ "$proc" = "mCP_UFO" ]; then
    cnew="$(echo $charge | sed 's/0.//')"
    if [ "$cnew" -lt "0100" ]; then
	sourcecharge=0.001
    elif [ "$cnew" -lt "1000" ]; then
	sourcecharge=0.01
    elif [ "$cnew" -ge "1000" ]; then
	sourcecharge=0.1
    fi
fi

#Define names
outputname="$proc"."$mass"GeV."$charge"Q."$configFile"Config
if [ "$proc" = "mCP_UFO" ]; then
    sourcename="$proc"/"$mass"/"$sourcecharge"/hit_4_vecs.txt
fi
JOB=$SCRATCH/Scratchy."$outputname"
SRC=$JOB/geant4/src
CONFIG=$JOB/geant4/config
echo "$outputname" "$nEv"

# Prepare configuration
rm -r $JOB
mkdir $JOB
cp -r $GEANT $JOB
cd $JOB

$sedcommand 's/^yParticleGunRescale.*/yParticleGunRescale = '"$yRescale"'/g' $CONFIG/particles.ini
$sedcommand 's/^zParticleGunRescale.*/zParticleGunRescale = '"$zRescale"'/g' $CONFIG/particles.ini

$sedcommand 's/^ElectricCharge.*/ElectricCharge = '"$charge"'/g' $CONFIG/particles.ini
$sedcommand 's/^MonopoleMass.*/MonopoleMass = '"$mass"'/g' $CONFIG/particles.ini
$sedcommand 's/^Particle.*/Particle = '"$particle"'/g' $CONFIG/particles.ini
if [ "$proc" != "cosmicmuons" ] && [ "$proc" != "am241" ]; then
    $sedcommand 's~FileName.*~FileName = '$sourcename'~g' $CONFIG/particles.ini
fi
$sedcommand 's~PathName.*~PathName = '$DATA'/~g' $CONFIG/particles.ini
$sedcommand 's/.*beamOn.*/\/run\/beamOn '"$nEv"'/g' $CONFIG/mcp.mac


if [[ $configFile == *"Am241"* ]]; then
    $sedcommand 's/^FileName.*/FileName = '"$configFile"'.dat/g' $CONFIG/Am241/Am241particles.ini
    configFile="am241"
fi



cmake -DGeant4_DIR=$G4DIR geant4/

#Run Program
make -j $nCores MilliQ
./MilliQ $CONFIG/mcp.mac $CONFIG/"$configFile".ini

# Collect Output
cp MilliQ.root $RESULTS/"$outputname".root
echo $mass $charge $nEv >> $RESULTS/NEventsInitial."$proc"."$configFile"Config.dat
rm -r $JOB
