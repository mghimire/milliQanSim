/*
*       This script written by Gabriel Magill for the milliQan collaboration
*       December 5, 2016
*       Used to calculate Offline efficiencies given Geant4 simulation  
*       Used to handle ROOT files from Geant4 simulation and plot interesting quantities
*
*
*/

//Standard c++ libraries
#include <iostream>
#include <fstream>
#include <string>
#include "sstream"
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>

//ROOT Libraries
#include <TRandom1.h>
#include <TH1F.h>
#include <TApplication.h>
#include "TMath.h"
#include "TF1.h"
#include "TGraph.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include "TFrame.h"
#include "TStyle.h"
#include "Math/WrappedTF1.h"
#include "Math/RootFinder.h"
#include "Math/BrentRootFinder.h"
#include <Riostream.h>
#include "TROOT.h"
#include "TTree.h"
#include "TFile.h"
#include "TSystem.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"

// Boost Libraries
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;

// This is a verbose mode, which prints out a lot of information
bool DEBUG=false;
// Output first N events
int EventRangeBegin=0; // Set to 0 for all events
int EventRangeEnd=1200000000; // Set to 1200000000 for all events
//int EventRangeBegin=15; 
//int EventRangeEnd=19; 

// Settings related to the G4 simulation that has been performed
// And the details of the analysis
// All these are set based on the .ini file
vector<int> NBlocks;
int NStacks;
int NBlocksPerStack;
float WAVEFORMCUTOFF;
float WAVEFORMWIDTH;
float TimePerSample;
string PEAKFINDEROPTIONS;
float PeakFinderPCHeightCutoff;
float ATWDwindow;
float TimeOffsetBtwCoincidences;
string OfflineTriggerStrategy;

// Reads in vectors of arbitrary type from a .ini file
template<typename T> std::vector<T> ptree_array(const boost::property_tree::ptree pt, const std::string sEntry) {
  const std::string sConfig = pt.get<std::string>(sEntry);

  std::vector<T> v;
  std::stringstream ss(sConfig);
  std::string item;

  while(std::getline(ss, item, ','))  v.push_back(boost::lexical_cast<T>(item));

  return v;
}

// Converts Channel Number to a set of x, y, z coordinates
vector<int> ChannelsToCoordinates( int channel) {

    // In the simulation, the coordinates are defined as follows:
    // Suppose we have a 1x2x3 (x,y,z) layer, the numbering goes as:
    //      +y
    //      ^
    //      | 3 4 5
    //      | 0 1 2
    //      ------> +z
    //     /
    //    /
    //  \/
    // -x
    // The rest of the detector goes in the +x direction

    // This function maintains the same convention 

    vector<int> xyzCoordinate(3);

    // Figures which stack it's in
    xyzCoordinate[0] = channel / NBlocksPerStack;
    // Figures which row (along y) it is
    xyzCoordinate[1] = (channel - NBlocksPerStack*xyzCoordinate[0])/NBlocks[2];
    // Figures which column (along z in picture above) it is
    xyzCoordinate[2] = (channel - NBlocksPerStack*xyzCoordinate[0]-NBlocks[2]*xyzCoordinate[1]);

    if(false) { 
        cout << " channel:" << channel;
        cout << " x:" << xyzCoordinate[0] ;
        cout << " y:" << xyzCoordinate[1] ;
        cout << " z:" << xyzCoordinate[2] ;
        cout << " " << endl ;
    }

    return xyzCoordinate;
}

// This Function is used to write a vector per event, where the vector is of length total # channels, to a Mathematica friendly format
// ActivePMT contains the channel numbers that are active
// Would be nice if it would plot directly to ROOT TCanvas!
template<typename T> void WriteVectorChannelsToFile( TTree *t1, vector<int> *ActivePMT, vector<T> *v, string name) {
    int n = t1->GetEntries();
    if(n > EventRangeEnd) n = EventRangeEnd;
 
    ofstream output;
    string outname="Geant4Output/"+name+".m";
    output.open (outname);

    if(DEBUG && (name == "PmtMedianHitTimes")) cout<<" Number of entries in t1 "<< n << endl;;
                
    int idPMT;
    T dat;
    output << "{";

    // Loop over events
    for(int i = EventRangeBegin; i < n; i++) {
        output << "{" ;
        t1->GetEntry(i);

        // Loop over channels that have seen activity
        for (unsigned long j=0; j<ActivePMT->size() ; j++) {
            // Get that channel ID (position in the *v vector)
            idPMT = ActivePMT->at(j);
            dat = v->at(idPMT);
            output << "{" << idPMT << "," << dat << "}";
            if(j < ActivePMT->size() - 1) output <<",";
        }
        output << "}";
        if(i < n-1) output << ",";
    }
    output << "}";
    output.close();
}

// Used to write a variable per event to Mathematica friendly format
template<typename T> void WriteVarToFile( TTree *t1, T *v, string name) {

    int n = t1->GetEntries();
    if(n > EventRangeEnd) n = EventRangeEnd;

    ofstream output;
    string outname="Geant4Output/"+name+".m";
    output.open (outname);

    output << "{";
    // Loop over events
    for(int i = EventRangeBegin; i < n; i++) {
        t1->GetEntry(i);
        output << boost::lexical_cast<T>(*v);
        if(i < n-1) output << ",";
    }
    output << "}";
    output.close();

    return;
}

// Used to write a vector *v per event to file.
// The length of Translations is total # of channels
// The first Translattion->at(0) elements of *v correspond to channel 0,
// The next Translation->at(1) elements of *v correspond to channel 1, etc.
template<typename T> void WriteVectorToFile( TTree *t1, vector<int> *Translation, vector<T> *v, string name) {
        
    // Because of the nature of PmtAllHitTimes, we need to deal with it differently
    int n = t1->GetEntries();
    if(n > EventRangeEnd) n = EventRangeEnd;
         
    ofstream output;
    string outname="Geant4Output/"+name+".m";
    output.open (outname);
                
    int nHits, counter;
    T dat;

    output << "{";
    // Loop over number of events
    for(int i = EventRangeBegin; i < n; i++) {
        output << "{" ;
        t1->GetEntry(i);
        counter=0;

        // Loop over channels that have been hit
        for (unsigned long j=0; j<Translation->size() ; j++) {
            nHits = Translation->at(j);
            if(nHits == 0) continue;

            // Loop over number of PMT hits in each channel
            for ( int k=counter; k < counter + nHits; k++ ) {
                dat = v->at(k);
                output << "{" << j << "," << dat << "}";
                output <<",";
            }
            counter+=nHits;

        }
        output << "NULL";

        output <<  "}";
        if(i < n-1) output << ",";
    }
    output << "}";
    output.close();
}

// Given a Waveform, this function finds the peaks, and stores their position (in Digitizer Samples) in Peaks vector
// This function is called for each channel, and is fed the background subtracted Waveform 
// All Waveforms for each channel are assumed to have the same number of sample counts, and the first and last samples correspond
// to the same start and end global times across all channels
void FindPeaks(vector<float> &Waveform, vector<float> &Peaks) {

    int ChannelSize = Waveform.size();
    if(ChannelSize > 0) {
        TH1F *h = new TH1F("", "h", ChannelSize, 0, ChannelSize);
        for(int i = 0; i < ChannelSize; i++) h->Fill(i, Waveform[i]);

        //Use TSpectrum to find the peak candidates
        TSpectrum *s = new TSpectrum(ChannelSize*2);

        // Peaks that are 1% the size of the highest peak are ignored, ok because already subtracted background
        // Tag peaks given a 0.5 sigma width
        Int_t nfound = s->Search(h,WAVEFORMWIDTH,PEAKFINDEROPTIONS.c_str(),PeakFinderPCHeightCutoff);
        Double_t xp, yp;
        Int_t bin;

        if(DEBUG) printf("Found %d candidate peaks to fit\n",nfound);

        Double_t *xpeaks = s->GetPositionX();
        // Loop over all found peaks, write their location to Peaks vector
        for (int p=0;p<nfound;p++) {
            xp = xpeaks[p];
            bin = h->GetXaxis()->FindBin(xp);
            yp = h->GetBinContent(bin);
            Peaks.push_back( xp );   
            if(DEBUG) cout<< "Bin location:"<<xp << " Bin value:" << yp << endl;      
        }
        
        delete(s,h);
    }

}

void GetNextCandidate(bool &TestNextCandidate, vector<int> &CandidateChannels, int ActiveChannel, int &CandidateCounter) {

    // This function will run through all possible candidates of channels that could constitute a signal, and output them to CandidateChannels
    // It starts suggesting configurations based on the initial ActiveChannel hit in the first layer
    // Once it's run through all candidates, it sets TestNextCandidate to false

    TestNextCandidate = true;
    vector<int> NN, NN2z, NN2y, NN3z, NN3y;
    int MaxAttempts;

    if(OfflineTriggerStrategy == "AllNeighbours" || OfflineTriggerStrategy == "ThreeFoldAnywhere" || OfflineTriggerStrategy == "ParticleTrajectory") {
        NN.push_back(0);
        NN.push_back(-1);
        NN.push_back(1);
        MaxAttempts = (int)std::pow(9, NStacks - 1);
    }

    if(OfflineTriggerStrategy == "BackToBack") {
        NN.push_back(0);
        NN.push_back(0);
        NN.push_back(0);
        MaxAttempts = 1;
    }

    int tempcounter = 0;

    // First channel is always ActiveChannel in first layer
    CandidateChannels.clear();
    CandidateChannels.push_back(ActiveChannel);

    if(CandidateCounter >= MaxAttempts) TestNextCandidate = false;

    // Loop over grid in second layer
    if(TestNextCandidate) {
        for(std::vector<int>::iterator z1 = NN.begin() ; z1 != NN.end(); ++z1) {
            for(std::vector<int>::iterator y1 = NN.begin() ; y1 != NN.end(); ++y1) {

                NN2z.clear();
                NN2y.clear();

                // Straight Line, or almost straight line configuration
                if (OfflineTriggerStrategy == "ParticleTrajectory" || OfflineTriggerStrategy == "BackToBack") {
                    NN2z.push_back(0);
                    NN2z.push_back(*z1);
                    NN2y.push_back(0);
                    NN2y.push_back(*y1);
                }

                // Neighbour configuration, or hits anywhere
                if( OfflineTriggerStrategy == "AllNeighbours" || OfflineTriggerStrategy == "ThreeFoldAnywhere"  ) {
                    for(unsigned int jjj = 0; jjj < NN.size(); jjj++) {
                        NN2z.push_back(NN[jjj]);
                        NN2y.push_back(NN[jjj]);
                    }
                }

                // Loop over grid in third layer
                for(std::vector<int>::iterator z2 = NN2z.begin() ; z2 != NN2z.end(); ++z2) {
                    for(std::vector<int>::iterator y2 = NN2y.begin() ; y2 != NN2y.end(); ++y2) {    

                        NN3z.clear();
                        NN3y.clear();

                        // Straight Line, or almost straight line configuration
                        if (OfflineTriggerStrategy == "ParticleTrajectory" || OfflineTriggerStrategy == "BackToBack") {
                            NN3z.push_back(0);
                            NN3z.push_back(*z2);
                            NN3y.push_back(0);
                            NN3y.push_back(*y2);
                        }

                        // Neighbour configuration, or hits anywhere
                        if( OfflineTriggerStrategy == "AllNeighbours" || OfflineTriggerStrategy == "ThreeFoldAnywhere"  ) {
                            for(unsigned int jjj = 0; jjj < NN.size(); jjj++) {
                                NN3z.push_back(NN[jjj]);
                                NN3y.push_back(NN[jjj]);
                            }
                        }

                        // Loop over grid in fourth layer
                        for(std::vector<int>::iterator z3 = NN3z.begin() ; z3 != NN3z.end(); ++z3) {
                            for(std::vector<int>::iterator y3 = NN3y.begin() ; y3 != NN3y.end(); ++y3) {

                                // This hack is used to find where we left off in a previous candidate suggestion
                                if(tempcounter < CandidateCounter) {
                                    tempcounter++;
                                    continue;
                                }

                                CandidateCounter++;
                                tempcounter++;          

                                int layer1 = ActiveChannel + NBlocksPerStack + *z1 + *y1*NBlocks[2];
                                int layer2 = layer1        + NBlocksPerStack + *z2 + *y2*NBlocks[2];
                                int layer3 = layer2        + NBlocksPerStack + *z3 + *y3*NBlocks[2];

                                if(ChannelsToCoordinates(ActiveChannel)[2] == 0              && *z1 == -1) continue;
                                if(ChannelsToCoordinates(ActiveChannel)[2] == NBlocks[2] - 1 && *z1 == 1) continue;
                                if(ChannelsToCoordinates(ActiveChannel)[1] == 0              && *y1 == -1) continue;
                                if(ChannelsToCoordinates(ActiveChannel)[1] == NBlocks[1] - 1 && *y1 == 1) continue;

                                if(ChannelsToCoordinates(layer1)[2] == 0               && *z2 == -1) continue;
                                if(ChannelsToCoordinates(layer1)[2] == NBlocks[2] - 1  && *z2 == 1) continue;
                                if(ChannelsToCoordinates(layer1)[1] == 0               && *y2 == -1) continue;
                                if(ChannelsToCoordinates(layer1)[1] == NBlocks[1] - 1  && *y2 == 1) continue;

                                if(ChannelsToCoordinates(layer2)[2] == 0               && *z3 == -1) continue;
                                if(ChannelsToCoordinates(layer2)[2] == NBlocks[2] - 1  && *z3 == 1) continue;
                                if(ChannelsToCoordinates(layer2)[1] == 0               && *y3 == -1) continue;
                                if(ChannelsToCoordinates(layer2)[1] == NBlocks[1] - 1  && *y3 == 1) continue;

                                // If it has gotten here, it is proposing a new candidate, we we want to exit from the function
                                CandidateChannels.push_back(layer1);
                                CandidateChannels.push_back(layer2);
                                CandidateChannels.push_back(layer3);   
                                return;
                            } // y3
                        } // z3

                    } // z2
                } // y2
    
            } // y1
        } // z1

        // If it has gotten here, it hasn't found any candidates and has exhausted all possibilities
        TestNextCandidate = false;
    } // if(NextTestCandidate)

}


struct WithinRange{
    const float flow, fupp;
    WithinRange(float low, float upp) : flow(low), fupp(upp) {}
    bool operator()(float peak) const { return (peak >= flow && peak <=fupp) ; }
};


// This function is used to check if a set of proposed channels have Waveforms that lie within the coincidence threshold of each other
// Length of Peaks is total number of channels
bool IsCoincidence(vector<vector<float>> Peaks, vector<int> ActiveChannels, float MaxSamples) {
    //Figures out for which sequence all 3 pmt and Scint light up

    vector<int> PeakID;
    vector<int> CandidateChannels;
    bool recordEvent = true;
    bool GoodEvent = false;

    bool GoodEvent0 = false;
    bool GoodEvent1 = false;
    bool GoodEvent2 = false;
    bool GoodEvent3 = false;

    // Loop over ActiveChannels
    for(unsigned int i=0; i < ActiveChannels.size(); i++) {

    // If this is true, then a signal consists of any hits in 3 different layers at any times
    if(OfflineTriggerStrategy == "ThreeFoldAnywhere") {
        int thisLayer = ActiveChannels[i] / NBlocksPerStack;
        if(thisLayer == 0) GoodEvent0 = true;
        else if(thisLayer == 1) GoodEvent1 = true;
        else if(thisLayer == 2) GoodEvent2 = true;
        else if(thisLayer == 3) GoodEvent3 = true;
        else cout << "U wot m8, fifth layer in IsCoincidence?" << endl;
            
        if(GoodEvent0 && GoodEvent1 && GoodEvent2) {
            GoodEvent = true;
            break;
        }
        continue;               
    }

    // If this is true, then a signal consists of any hits in 3 different layers at any times
    if(OfflineTriggerStrategy == "FourFoldAnywhere") {
        int thisLayer = ActiveChannels[i] / NBlocksPerStack;
        if(thisLayer == 0) GoodEvent0 = true;
        else if(thisLayer == 1) GoodEvent1 = true;
        else if(thisLayer == 2) GoodEvent2 = true;
        else if(thisLayer == 3) GoodEvent3 = true;
        else cout << "U wot m8, fifth layer in IsCoincidence?" << endl;
                    
        if(GoodEvent0 && GoodEvent1 && GoodEvent2 && GoodEvent3) {
            GoodEvent = true;
            break;
        }
        continue;               
    }

    // If this is true, then a signal consists of any hits in the first layer
    if(OfflineTriggerStrategy == "SingleLayer") {
        if(ActiveChannels[i] < NBlocksPerStack) GoodEvent = true;
        if(GoodEvent == true) break;
        continue;               
    }

    // Find active channel in first layer, use it as a seed for GetNextCandidate function
    if(ActiveChannels[i] > NBlocksPerStack) continue;

    bool TestNextCandidate = true;  
    int CandidateCounter = 0; // Used to go through all possible candidates given initial seed channel in layer 1
    // Once have exhausted all possibilities, GetNextCandidate will set TestNextCandidate to false
    while( TestNextCandidate == true ) {
        GetNextCandidate(TestNextCandidate, CandidateChannels, ActiveChannels[i], CandidateCounter);
        if(TestNextCandidate == false) continue;

        recordEvent = true;
        PeakID.clear();

        if(false) cout<<" CandidateChannel " << CandidateChannels[0] << " " << CandidateChannels[1] << " " << CandidateChannels[2]<< " " <<endl;

        // Check if proposed channels actually has activity
        for(unsigned int j=0; j < CandidateChannels.size(); j++) {
            auto it = std::find(ActiveChannels.begin(), ActiveChannels.end(), CandidateChannels[j] );
            if(it == ActiveChannels.end() ) {
                recordEvent = false;
                break;
            } else {
                PeakID.push_back(std::distance(ActiveChannels.begin(), it) );
            }       
        }
                

        // If it has activity, check whether Peaks are within coincidence threshold
        if(recordEvent == true) {

            float sampleLowBound = 0.;
            float sampleUppBound = ATWDwindow;
            float ATWDOffset= TimeOffsetBtwCoincidences/TimePerSample;

            if(DEBUG) cout << " This is ATWD offset between concecutive layers " << ATWDOffset << endl;


            // Defines a sliding window of length ATWDwindow, 
            // Checks if all pairs of channels have at least one peak within this window
            while(sampleLowBound <= 2*MaxSamples) {
                GoodEvent0 = false;
                GoodEvent1 = false;
                GoodEvent2 = false;
                GoodEvent3 = false;
 
                if (std::any_of(Peaks[PeakID[0]].cbegin(), Peaks[PeakID[0]].cend(), WithinRange(sampleLowBound,sampleUppBound)))
                    GoodEvent0 = true;
                if (std::any_of(Peaks[PeakID[1]].cbegin(), Peaks[PeakID[1]].cend(), WithinRange(sampleLowBound+ATWDOffset,sampleUppBound+ATWDOffset)))
                    GoodEvent1 = true;
                if (std::any_of(Peaks[PeakID[2]].cbegin(), Peaks[PeakID[2]].cend(), WithinRange(sampleLowBound+2*ATWDOffset,sampleUppBound+2*ATWDOffset)))
                    GoodEvent2 = true;
                if (std::any_of(Peaks[PeakID[3]].cbegin(), Peaks[PeakID[3]].cend(), WithinRange(sampleLowBound+3*ATWDOffset,sampleUppBound+3*ATWDOffset)))
                    GoodEvent3 = true;

                if(GoodEvent0 && GoodEvent1 && GoodEvent2 && GoodEvent3) {
                    GoodEvent = true;
                    break;
                }

                sampleLowBound += 1;
                                        
            }
        }
        
        // Break out of TestNextCandidate while loop if found coincidence
        if(DEBUG==true && GoodEvent==true) {
            for(int ll=0; ll < NStacks; ll++) {
                cout << " PeakID["<<ll<<"]: " << PeakID[ll] << endl;
                cout << Peaks.size() << " Peaks.size() " << endl;
                cout << " ActiveChannels["<<ll<<"]: " << ActiveChannels[ll] << endl;
            }
            cout << ActiveChannels.size() << " ActiveChannels.size() " << endl;
            cout << GoodEvent << " GoodEvent " << endl;
        }

        if(GoodEvent == true) break;  
    }
    // Break out of ActiveChannels if found coincidence
    if(GoodEvent == true) break;

    }       

    if(DEBUG==true) cout << GoodEvent << " GoodEvent " << endl;
 
    return GoodEvent;
}


// This function goes through all events, Unwinds the waveforms, calls PeakFinder function, calls IsCoincidence
// and outputs the total number of coincident events
int CoincidencesManager( TTree *t1, vector<int> *Translation, vector<float> *v) { 
    int n = t1->GetEntries();
    if(n > EventRangeEnd ) n = EventRangeEnd;
        
    int nHits, counter;
    float dat;
    int SignalCounts = 0;

    vector<float> WaveformPerChannel;
    vector<vector<float>> PeakLocations;    
    vector<float> PeakLocationsPerChannel;  
    vector<int> ActiveChannels;     
    float MaxSamples;

    // Loop over number of events
    for(int i = EventRangeBegin; i < n; i++) {
        if(DEBUG) cout<<" Number of Events n: " << n << endl;

        t1->GetEntry(i);
        counter=0;
        PeakLocations.clear();
        ActiveChannels.clear();

        MaxSamples = -1.;

        // Loop over all channels
        for (unsigned long j=0; j<Translation->size() ; j++) {
            WaveformPerChannel.clear();
            PeakLocationsPerChannel.clear();

            // Figures out which channels have been hit
            nHits = Translation->at(j);
            if(nHits == 0) continue;
        
            // Specifies max samples across all channels, across all events
            MaxSamples = std::max(MaxSamples, (float)nHits);

            ActiveChannels.push_back(j);
            // Veto Background Waveform samples, i.e V(t) in mV < WAVEFORMCUTOFF
            for ( int k=counter; k < counter + nHits; k++ ) {
                dat = v->at(k);
                if(dat < WAVEFORMCUTOFF) dat = 0;
                WaveformPerChannel.push_back(dat);
            }
            // We are doing a translation:
            counter+=nHits;

            if(DEBUG) {
                cout <<"Waveform " <<endl;
                for(unsigned long aaa = 0; aaa < WaveformPerChannel.size(); aaa++)
                        cout << WaveformPerChannel[aaa] <<",";
                cout<<endl;
                cout << " ActiveChannels # " << j << endl;
            }
                        
            // For eaach channel, find all the peaks        
            FindPeaks(WaveformPerChannel, PeakLocationsPerChannel);

            // This is then contains all the peaks for each active channels
            // The peaks in PeakLocations[i] corresponds to channel ActiveChannels[i]
            PeakLocations.push_back(PeakLocationsPerChannel);
        }       

        // Checks if there samples that were taken
        assert(MaxSamples > 0);
        // Was there a coincidence anywhere in this event?
        if (  IsCoincidence( PeakLocations, ActiveChannels, MaxSamples ) ) SignalCounts++;

    }

    // Output how many coincidence it found given n events it searched through
    return SignalCounts;
}

// Unpacks the Root File Trees corresponding to the sensitivity information, and launches the CoincidencesManager
void GeantSensitivities(string PathName, vector<string> Particle, string ConfigType, string ConfigDetails, bool OutputWaveformsToFile ) {
    TTree *t1;
    // Read in a ROOT file
    int SignalEvents;

    // Loop over particles for which you want to calculate sensitivities
    // i.e. mCP_UFO, Y1S, Y2S, Y3S, JPsi, specified in .ini file 
    for(unsigned long i = 0; i < Particle.size(); i++ ) {
        std::ifstream infile;
        string fFilename;
        // When running G4 via the bash script, this file is created for every particle,
        // with the details of the number of submitted events, the number of events that passed OnlineTrigger
        // the charge and mass
        fFilename+=PathName+"NEventsInitial."+Particle[i]+"."+ConfigType+".dat";
        std::string line;
        infile.open(fFilename);
                
        string EfficiencyOutputName="EfficiencyDataFiles/Acceptances."+OfflineTriggerStrategy+"."+Particle[i]+"."+ConfigDetails+".dat";
        cout<< " Cleaning ... " << EfficiencyOutputName << endl;
        remove( EfficiencyOutputName.c_str() );

        while (std::getline(infile, line)) {
            string a, b;
            double c;
            std::istringstream iss(line);
            iss.clear();
            iss.str(line);

            //iss >> MASS >> Q >> Number of Submitted Events in G4 before online triggers
            iss >> a >> b >> c;

            // For each mass, Q, particle configuration, it spits out a ROOT file
            string RootFile;
            string fileName=Particle[i]+"."+a+"GeV"+"."+b+"Q"+"."+ConfigType;
            RootFile+=PathName+fileName+".root";

            TFile *fMilliQ = new TFile(RootFile.c_str());
            t1 = (TTree*)fMilliQ->Get("MilliQCAEN");
        
            // Store MilliQCAEN Information
            vector<float>* WaveformVoltage = new vector<float>();
            vector<int>* WaveformLengthPerChannel = new vector<int>();
            float FirstPMTTime;
            float timePERsample;
            // Store MilliQCAEN Information
            t1->SetBranchAddress("WaveformVoltage",&WaveformVoltage);
            t1->SetBranchAddress("WaveformLengthPerChannel",&WaveformLengthPerChannel);
            t1->SetBranchAddress("FirstPMTTime",&FirstPMTTime);
            t1->SetBranchAddress("timePERsample",&timePERsample);

            // We write waveform info to file
            if(OutputWaveformsToFile) WriteVectorToFile(t1, WaveformLengthPerChannel, WaveformVoltage, "WaveformVoltage."+fileName);
        
            // Figures out how many events pass OfflineTriggers     
            SignalEvents = 0;
            SignalEvents = CoincidencesManager(t1, WaveformLengthPerChannel, WaveformVoltage);

            // Calculates Efficiency after OfflineTrigger and before OnlineTrigger 
            double RunEfficiency = (double)SignalEvents / c;
            // Write out good events and efficiencies to file..., just like before
            cout << RunEfficiency << " RunEfficiency " << endl;
                
            // Writes out efficiency information to a .dat file so that it can be read into noWaveformSensitivities
            // for the sensitivity calculations
            ofstream EfficiencyOutput;
            EfficiencyOutput.open (EfficiencyOutputName, std::fstream::in | std::fstream::out | std::fstream::app);
            EfficiencyOutput << a << " " << b << " " << c << " " << SignalEvents << endl ;
            EfficiencyOutput.close();

            fMilliQ->Close();
            delete WaveformVoltage, WaveformLengthPerChannel, fMilliQ;

        }
    }

}


// Main Function to output pre packaged data from simulation
// Geant4 calculates a bunch of truth information, and this is output to mathematica for plotting
void KinematicPlots(string PathName, vector<string> PlotParticle, string ConfigType, vector<string> PlotCharge, vector<string> PlotMass) {
    TTree *t1;
    TTree *t2;

    // We loop over the particles, masses and charges we are interested in plotting 
    for(unsigned long i = 0; i < PlotParticle.size(); i++) {
        for(unsigned long a = 0; a < PlotMass.size(); a++) {
            for(unsigned long b = 0; b < PlotCharge.size(); b++) {
                // We extract these data from the root files
                string RootFile;
                RootFile+=PathName+PlotParticle[i]+"."+PlotMass[a]+"GeV"+"."+PlotCharge[b]+"Q"+"."+ConfigType+".root";
                TFile *fMilliQ = new TFile(RootFile.c_str());
                t1 = (TTree*)fMilliQ->Get("MilliQAll");
                t2 = (TTree*)fMilliQ->Get("MilliQDedx");

                // Store MilliQAll Information
                vector<int>    * ActivePMT = new vector<int>();
                vector<double> * PmtMedianHitTimes = new vector<double>();
                vector<double> * PmtAllHitTimes = new vector<double>();
                vector<double> * TimeOfFlight = new vector<double>();
                vector<double> * TotalEnergyDeposit = new vector<double>();
                vector<int>    * NumberPMTHits = new vector<int>();
                int FirstHitScintillator;
                int PhotonCountAllScintillators;
                // Set MilliQAll Addresses
                t1->SetBranchAddress("ActivePMT",&ActivePMT);
                t1->SetBranchAddress("PmtMedianHitTimes",&PmtMedianHitTimes);
                t1->SetBranchAddress("PmtAllHitTimes",&PmtAllHitTimes);
                t1->SetBranchAddress("TimeOfFlight",&TimeOfFlight);
                t1->SetBranchAddress("TotalEnergyDeposit",&TotalEnergyDeposit);
                t1->SetBranchAddress("NumberPMTHits",&NumberPMTHits);
                t1->SetBranchAddress("FirstHitScintillator",&FirstHitScintillator);
                t1->SetBranchAddress("PhotonCountAllScintillators",&PhotonCountAllScintillators);

                // Stores and sets MilliQDedx
                double EKinMeV;
                double MeVpermm;
                t2->SetBranchAddress("EKinMeV",&EKinMeV);
                t2->SetBranchAddress("MeVpermm",&MeVpermm);
                
                string nameExt = "."+PlotParticle[i]+"."+PlotMass[a]+"GeV."+PlotCharge[b]+"Q."+ConfigType;

                // Write to file MilliQAll information 
                WriteVectorChannelsToFile(t1, ActivePMT, PmtMedianHitTimes, "PmtMedianHitTimes"+nameExt);
                WriteVectorChannelsToFile(t1, ActivePMT, TimeOfFlight, "TimeOfFlight"+nameExt);
                WriteVectorChannelsToFile(t1, ActivePMT, TotalEnergyDeposit, "TotalEnergyDeposit"+nameExt);
                WriteVectorChannelsToFile(t1, ActivePMT, NumberPMTHits, "NumberPMTHits"+nameExt);
                WriteVarToFile(t1, &FirstHitScintillator, "FirstHitScintillator"+nameExt);
                WriteVarToFile(t1, &PhotonCountAllScintillators, "PhotonCountAllScintillators"+nameExt);
                WriteVectorToFile(t1, NumberPMTHits, PmtAllHitTimes, "PmtAllHitTimes"+nameExt);

                // Write MilliQDedx
                WriteVarToFile(t2, &EKinMeV, "EKinMeV"+nameExt);
                WriteVarToFile(t2, &MeVpermm, "MeVpermm"+nameExt);
                
                if(DEBUG) {
                    ChannelsToCoordinates(3);
                    ChannelsToCoordinates(400);
                    ChannelsToCoordinates(420);
                    ChannelsToCoordinates(1079);
                    ChannelsToCoordinates(977);
                    ChannelsToCoordinates(1199);
                }
        
                delete fMilliQ;
                delete ActivePMT, PmtMedianHitTimes, PmtAllHitTimes, TimeOfFlight, TotalEnergyDeposit, NumberPMTHits;
            }
        }
    }

    return;
}

int main(int argc, char* argv[]) {

    // Need to feed it a configuration file
    if (argc !=2 ) {
            cout << " Need to specify a configuration File from /OfflineAnalysis/AnalyseGeant4ROOT/ConfigFiles" << endl;
            exit(0);
    }

    // Read in configuration file 
    string configFile = argv[1];
    boost::property_tree::ptree pt;
    try {
            boost::property_tree::ini_parser::read_ini(configFile, pt);
    }
    catch(boost::property_tree::ptree_error &e) {
            cout << "File not valid ConfigFileReadError" << endl;
    }

    // Read information based on G4 Simulation that has been performed
    std::vector<string> Particle = ptree_array<string>(pt, "G4RootFiles.Particle");
    string ConfigType = pt.get<string>("G4RootFiles.ConfigType");
    string ConfigDetails = pt.get<string>("G4RootFiles.ConfigDetails");
    string PathName = pt.get<string>("G4RootFiles.PathName");
    NBlocks = ptree_array<int>(pt, "G4RootFiles.NBlocks");
    NStacks = pt.get<int>("G4RootFiles.NStacks");
    NBlocksPerStack = NBlocks[0]*NBlocks[1]*NBlocks[2];

    // Read Details of which offline Analysis to perform
    bool DoSensitivities = pt.get<bool>("AnalysisDetails.DoSensitivities");
    OfflineTriggerStrategy = pt.get<string>("AnalysisDetails.OfflineTriggerStrategy");
    bool OutputWaveformsToFile = pt.get<bool>("AnalysisDetails.OutputWaveformsToFile");
    bool DoHistograms = pt.get<bool>("AnalysisDetails.DoHistograms");
    vector<string> PlotCharge = ptree_array<string>(pt, "AnalysisDetails.PlotCharge");
    vector<string> PlotMass   = ptree_array<string>(pt, "AnalysisDetails.PlotMass");
    std::vector<string> PlotParticle = ptree_array<string>(pt, "AnalysisDetails.PlotParticle");


    // Read Digitizer Settings
    WAVEFORMCUTOFF = pt.get<float>("DigitizerInformation.WAVEFORMCUTOFF");
    float CoincidenceThreshold = pt.get<float>("DigitizerInformation.CoincidenceThreshold");                          
    TimePerSample = pt.get<float>("DigitizerInformation.TimePerSample");
    WAVEFORMWIDTH = pt.get<float>("DigitizerInformation.WAVEFORMWIDTH");
    PEAKFINDEROPTIONS = pt.get<string>("DigitizerInformation.PEAKFINDEROPTIONS");
    PeakFinderPCHeightCutoff = pt.get<float>("DigitizerInformation.PeakFinderPCHeightCutoff");
    TimeOffsetBtwCoincidences = pt.get<float>("DigitizerInformation.TimeOffsetBtwCoincidences");
    ATWDwindow = CoincidenceThreshold / TimePerSample;

    // If we are interested in doing the sensitivities and waveform business
    if (DoSensitivities) GeantSensitivities(PathName, Particle, ConfigType, ConfigDetails,OutputWaveformsToFile  );

    // If we are interested in plotting and studying the truth G4 information that is precalculated in the simulation
    if (DoHistograms) KinematicPlots(PathName, PlotParticle, ConfigType, PlotCharge, PlotMass);

    return 0;
}

