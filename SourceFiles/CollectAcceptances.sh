#!/bin/bash

charges=(0.001 0.01 0.1)

process=("mCP_UFO" "JPsi" "Y1S" "Y2S" "Y3S")

rm $DATADIR/acceptances.geometric.dat

for proc in ${process[*]}
do


	if [ "$proc" = "mCP_UFO" ]; then
        	masses=(0.1 0.28 0.43 0.6 0.78 1.0 1.25 1.52 1.84 2.2 2.6 3.04 3.54 4.1 4.71 5.4 6.15 6.98 7.9 8.9 10.0 11.2 12.5 14.0 15.5 17.2 19.1 21.1 23.3 25.6 28.2 30.9 33.9 37.1 40.5 44.2 48.2 52.5 57.1 62.1 67.4 73.0 79.1 85.6 92.6 99.9)
	elif [ "$proc" = "JPsi" ]; then
	        masses=(0.1 0.28 0.43 0.6 0.78 1.0 1.25)
	elif [ "$proc" = "Y1S" ] || [ "$proc" = "Y2S" ] || [ "$proc" = "Y3S" ]; then
	        masses=(0.1 0.28 0.43 0.6 0.78 1.0 1.25 1.52 1.84 2.2 2.6 3.04 3.54 4.1)
	elif [ "$proc" = "test" ]; then
	        masses=(0.1)
	fi

	for charge in ${charges[*]}
	do
		for mass in ${masses[*]}
		do
			cd $DATADIR/"$proc"/"$mass"/"$charge"
			accept="$(echo $(echo $(cat acceptance.txt | sed -e 's/e/*10^/g') | cut -d' ' -f1))"
			echo "$proc" "$mass" "$charge" "$accept" >> $DATADIR/acceptances.geometric.dat
		done
	done
done

cp $DATADIR/acceptances.geometric.dat $RESULTSDIR
