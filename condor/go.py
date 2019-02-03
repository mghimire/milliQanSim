#!/usr/bin/env python

import subprocess
import os
import shutil
import sys

jobNumber = int(sys.argv[1])
proc = str(sys.argv[2]) # Sample, mCP_UFO, JPsi, Y1S, Y2S, Y3S, cosmicmuons, am241

charges = [0.0010, 0.0015, 0.0022, 0.0034, 0.0051, 0.0076, 0.0114, 0.0171, 0.0256, 0.0384, 0.0575, 0.1000] #Full Study
#charges = [0.0010, 0.0020, 0.0030, 0.0040, 0.0050, 0.0060, 0.0070, 0.0080, 0.0090, 0.0100, 0.0120, 0.0140] # Block Study

particle = 'monopole' # mu-

configFile = 'fourlayers'

nCores = 1

# Rescale the y or z position for the direction of the particle gun
yRescale = 1.
zRescale = 1.

########################################################################
# Define masses/charges based on chosen process
########################################################################

if proc == 'mCP_UFO':
    masses = [0.1, 0.28, 0.43, 0.6, 0.78, 
              1.0, 1.25, 1.52, 1.84, 2.2, 2.6, 3.04, 3.54, 4.1, 4.71, 
              5.4, 6.15, 6.98, 7.9, 8.9, 
              10.0, 11.2, 12.5, 14.0, 15.5, 17.2, 19.1, 
              21.1, 23.3, 25.6, 28.2, 
              30.9, 33.9, 37.1, 
              40.5, 44.2, 48.2, 
              52.5, 57.1, 
              62.1, 67.4, 
              73.0, 79.1,
              85.6, 
              92.6, 99.9] # Full Study
elif proc == 'JPsi':
    masses = [0.1, 0.28, 0.43, 0.6, 0.78, 1.0, 1.25]
elif proc == 'Y1S' or proc == 'Y2S' or proc == 'Y3S':
    masses = [0.1, 0.28, 0.43, 0.6, 0.78, 1.0, 1.25, 1.52, 1.84, 2.2, 2.6, 3.04, 3.54, 4.1]

if proc == 'Sample':
    masses = [0.28, 8.9, 92.6]
    charges = [0.0034, 0.0076]
    proc = 'mCP_UFO'
    nEv = 10000

if proc == 'cosmicmuons':
    masses = [0.105]
    charges = [1.000]
    proc = 'cosmicmuons'
    nEv = 100
    SampleFlag = 'On'
    configFile = 'cosmicmuons'
    particle = 'mu-'

if proc == 'am241':
    masses = [0.000]
    charges = [0.000]
    proc = 'am241'
    SampleFlag = 'On'
    configFile = 'Am241DistributionMiddle'
    particle = 'gamma'

########################################################################
# Change values in configuration files
########################################################################

mass = masses[jobNumber / len(charges)]
charge = charges[jobNumber % len(charges)]

if   charge < 0.0030: nEv = 25000
elif charge < 0.0050: nEv = 10000
elif charge < 0.0090: nEv = 1000
elif charge < 0.0200: nEv = 400
elif charge < 0.0500: nEv = 100
elif charge < 0.2000: nEv = 50
else: nEv = 20

if   charge < 0.0100: sourcecharge = 0.001
elif charge < 0.1000: sourcecharge = 0.01
else: sourcecharge = 0.1

#Define names
outputname = proc + '.' + str(mass) + 'GeV.' + str(charge) + 'Q.' + configFile + 'Config'
sourcename = proc + '/' + str(mass) + '/' + str(sourcecharge) + '/hit_4_vecs.txt'

print outputname, nEv

subprocess.call('tar -xzf build.tgz', shell = True)
os.chdir('geant4-build')

subprocess.call('sed -i "s/^yParticleGunRescale.*/yParticleGunRescale = ' + str(yRescale) + '/g" config/particles.ini', shell = True)
subprocess.call('sed -i "s/^zParticleGunRescale.*/zParticleGunRescale = ' + str(zRescale) + '/g" config/particles.ini', shell = True)
subprocess.call('sed -i "s/^ElectricCharge.*/ElectricCharge = ' + str(charge) + '/g" config/particles.ini', shell = True)
subprocess.call('sed -i "s/^MonopoleMass.*/MonopoleMass = ' + str(mass) + '/g" config/particles.ini', shell = True)
subprocess.call('sed -i "s/^Particle.*/Particle = ' + particle + '/g" config/particles.ini', shell = True)

if proc != 'cosmicmuons' and proc != 'am241':
    subprocess.call('sed -i "s|FileName.*|FileName = ' + sourcename + '|g" config/particles.ini', shell = True)

subprocess.call('sed -i "s~PathName.*~PathName = /data/users/bfrancis/condor/milliqan/SourceFiles/~g" config/particles.ini', shell = True)
subprocess.call('sed -i "s/.*beamOn.*/\/run\/beamOn ' + str(nEv) + '/g" config/mcp.mac', shell = True)

if 'Am241' in configFile:
    subprocess.call('sed -i "s/^FileName.*/FileName = ' + configFile + '.dat/g" config/Am241/Am241particles.ini', shell = True)
    configFile = 'am241'

########################################################################
#Run Program
########################################################################

subprocess.call('./MilliQ config/mcp.mac config/' + configFile + '.ini', shell = True)

shutil.move('MilliQ.root', '../' + outputname + '.root')

fOut = open('../NEventsInitial.' + outputname + '.dat', 'w')
fOut.write(str(mass) + ' ' + str(charge) + ' ' + str(nEv) + '\n')
fOut.close()

os.chdir('../')
subprocess.call('rm -r geant4-build/', shell = True)
subprocess.call('rm build.tgz', shell = True)

