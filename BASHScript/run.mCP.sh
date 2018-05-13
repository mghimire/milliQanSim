#!/bin/bash

# in the submit file, there is an option for linux or mac

#Read in paths
source paths.mCP.sh

# Pick Process
# Potentially modify User Configurations below
#	If you choose cosmicmuons or am241, only need to set proc. All the rest is set automatically in one of the if statements below.
proc="JPsi" # "Sample" "mCP_UFO" "JPsi" "Y1S" "Y2S" "Y3S"
#proc="cosmicmuons"
#proc="am241"

#Pick Charges
charges=(0.0010 0.0015 0.0022 0.0034 0.0051 0.0076 0.0114 0.0171 0.0256 0.0384 0.0575 0.1000) #Full Study
#charges=(0.0010 0.0020 0.0030 0.0040 0.0050 0.0060 0.0070 0.0080 0.0090 0.0100 0.0120 0.0140) # Block Study

#Pick particle for simulation
particle="monopole"
#particle="mu-"

#Pick Config File
configFile="onepc" #Omit .ini  # default  onepc fortyninepc

#Pick # of cores to make compile 
nCores=1

# Rescale the y or z position for the direction of the particle gun
yRescale=1.
zRescale=1.

#Pick Cluster run or Laptop run
#clusterCommand="bash" #submits regular bash script locally
clusterCommand="sbatch -p cerberus --ntasks=1"  #submits to a queue

#Pick benchmark masses
if [ "$proc" = "mCP_UFO" ]; then
#	masses=(0.1 0.28 0.43 0.6 0.78 1.0 1.25 1.52 1.84 2.2 2.6 3.04 3.54 4.1 4.71 5.4 6.15 6.98 7.9 8.9 10.0 11.2 12.5 14.0 15.5 17.2 19.1 21.1 23.3 25.6 28.2 30.9 33.9 37.1 40.5 44.2 48.2 52.5 57.1 62.1 67.4 73.0 79.1 85.6 92.6 99.9) # Full Study
	masses=(0.1 99.9)
elif [ "$proc" = "JPsi" ]; then
	masses=(0.1 0.28 0.43 0.6 0.78 1.0 1.25)
elif [ "$proc" = "Y1S" ] || [ "$proc" = "Y2S" ] || [ "$proc" = "Y3S" ]; then
	masses=(0.1 0.28 0.43 0.6 0.78 1.0 1.25 1.52 1.84 2.2 2.6 3.04 3.54 4.1)
fi

#Pick User Configurations for Sample Process
if [ "$proc" = "Sample" ]; then
        masses=(0.28 8.9 92.6) #Overwrites masses above
        charges=(0.0034 0.0076) #Overwrites charges above 
        proc="mCP_UFO" #Sample Process
        nEv=10000 #Number of events in Simulation
	SampleFlag="On" # Leave this On
fi
if [ "$proc" = "cosmicmuons" ]; then
        masses=(0.105) #Overwrites masses above
        charges=(1.000) #Overwrites charges above 
        proc="cosmicmuons" #Sample Process
        nEv=100 #Number of events in Simulation
	SampleFlag="On" # Leave this On
	configFile="cosmicmuons"
	particle="mu-"
fi
if [ "$proc" = "am241" ]; then
        masses=(0.000) #Overwrites masses above
        charges=(0.000) #Overwrites charges above 
        proc="am241" #Sample Process
        nEv=3900 #Number of events in Simulation
	SampleFlag="On" # Leave this On
#	configFile="Am241DistributionCenteredFarEnd10and60keV"
#	configFile="Am241DistributionCloseEnd"
#	configFile="Am241DistributionFarEnd"
	configFile="Am241DistributionMiddle"
	particle="gamma"
fi


# Reset NEventsInitialFile
rm $RESULTS/NEventsInitial."$proc"."$configFile"Config.dat


#Submit simulation
for mass in ${masses[*]}
do

	for charge in ${charges[*]}
	do
		#Define statistics based on charge
		cnew=$(echo $charge | sed 's/0.//')
		if [ "$SampleFlag" != "On" ]; then
	                if [ "$cnew" -le "0030" ]; then
				nEv=25000
			elif [ "$cnew" -le "0050" ]; then
				nEv=10000
	                elif [ "$cnew" -le "0090" ]; then
	                        nEv=1000
			elif [ "$cnew" -le "0200" ]; then
		                nEv=400
			elif [ "$cnew" -le "0500" ]; then
		                nEv=100
			elif [ "$cnew" -le "2000" ]; then
		                nEv=50
			elif [ "$cnew" -gt "2000" ]; then
				nEv=20
			fi
		fi
	
		echo Now submitting charge $charge  mass $mass  nEvent $nEv  process $proc
		$clusterCommand submit.mcp.exclusions.sh $charge $mass $nEv $proc $configFile $nCores $particle $yRescale $zRescale

	done
done
