#!/bin/bash

# This is a little hacky . . . 

option=$1
proc=$2

CONFIG="ConfigFiles/fortyninepcConfig.ini"
strategy="BackToBack"
Name="FortyNinePCFullSettings.5b5.refl0.99.LG.dat"
DataFiles="../../../DataFiles"

echo $CONFIG

cd /Users/chaffy/Work/Repository_MilliCharged/milliQan/OfflineAnalysis/AnalyseGeant4ROOT/

make

if [ $option = 1 ]; then
	sed -i '' 's/^Particle =.*/Particle = '"$proc"'/g' $CONFIG
fi

if [ $option = 2 ]; then

	cd EfficiencyDataFiles/

	cat Acceptances.$strategy.mCP_UFO.$Name | sort > temp
	mv temp $DataFiles/Acceptances.$strategy.mCP_UFO.$Name

	cat Acceptances.$strategy.JPsi.$Name | sort > temp
	mv temp $DataFiles/Acceptances.$strategy.JPsi.$Name

	cat Acceptances.$strategy.Y1S.$Name | sort > temp
	mv temp $DataFiles/Acceptances.$strategy.Y1S.$Name

	cat Acceptances.$strategy.Y2S.$Name | sort > temp
	mv temp $DataFiles/Acceptances.$strategy.Y2S.$Name

	cat Acceptances.$strategy.Y3S.$Name | sort > temp
	mv temp $DataFiles/Acceptances.$strategy.Y3S.$Name

fi

if [ $option = 3 ]; then

	cd ../Sensitivities/ 
	make
	./noWaveformSensitivities $strategy /../../DataFiles/ $Name acceptances.geometric.v3

fi
