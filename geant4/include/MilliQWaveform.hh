/*
 * MilliQWaveform.h
 *
 *  Created on: Aug 23, 2016
 *      Author: chaffy
 */
#include "globals.hh"
#include <vector>


#ifndef SOURCE_DIRECTORY__SRC_MILLIQWAVEFORM_H_
#define SOURCE_DIRECTORY__SRC_MILLIQWAVEFORM_H_

class MilliQWaveform {
public:
  MilliQWaveform();

  virtual ~MilliQWaveform();

  std::vector<G4float> GetSinglePhotoelectronWaveform();
  G4float GetBackgroundWaveform();


private:


};

#endif /* SOURCE_DIRECTORY__SRC_MILLIQWAVEFORM_H_ */
