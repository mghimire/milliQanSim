/*
 * MilliQWaveform.cpp
 *
 *  Created on: Aug 23, 2016
 *      Author: chaffy
 */

#include "../include/MilliQWaveform.hh"
#include "Randomize.hh"
#include "time.h"

MilliQWaveform::MilliQWaveform(){


}

std::vector<G4float> MilliQWaveform::GetSinglePhotoelectronWaveform(){

  /*	std::ifstream infile;

  // in mV
  std::string fileLocation = "../../SourceFiles/Waveform.dat";

  G4cout << "Reading Single Photon Electron Waveform from "<<fileLocation << G4endl;

  infile.open(fileLocation);
  G4String line;
  G4double WF;
  std::vector<G4float> singlePEWaveform;
  while (std::getline(infile, line)) {

  std::istringstream iss(line);
  iss.clear();
  iss.str(line);

  if (!(iss >> WF))
  break;

  singlePEWaveform.push_back( WF );

  }
  */
  static const G4float arr[] = {0.2737, 2.274, 6.358, 8.274, 7.453, 5.326, 2.884, 1.516, 1.116, 0.6316, 0.04211, 0.1684, 0.4842, 0.04211, -0.2105, 0.1684, 0.2737};
  std::vector<G4float> singlePEWaveform (arr, arr + sizeof(arr) / sizeof(arr[0]) );

  return singlePEWaveform;

}

G4float MilliQWaveform::GetBackgroundWaveform(){

  G4float randomBkg = (0.3*((double)rand()) / ((double)RAND_MAX)-0.02);

  //	G4cout << " the random bkg Waveform " << randomBkg << G4endl;

  return randomBkg;

}




MilliQWaveform::~MilliQWaveform() {
  // TODO Auto-generated destructor stub
}
