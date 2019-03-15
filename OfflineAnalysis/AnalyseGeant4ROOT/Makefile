CC=g++
CFLAGS=-c -Wall -Wextra `root-config --cflags` -I/cvmfs/cms.cern.ch/slc6_amd64_gcc630/external/boost/1.63.0-fmblme/include -std=c++14
LDFLAGS=`root-config --libs` `root-config --glibs` -lgmp -lm -lSpectrum

SOURCES=AnalyseGeant4ROOT_4layers.cc
#SOURCES=AnalyseGeant4ROOT.cc

OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=AnalyseGeant4ROOT

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm ./*.o ./AnalyseGeant4ROOT
