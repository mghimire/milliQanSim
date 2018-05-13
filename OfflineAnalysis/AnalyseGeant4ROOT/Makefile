CC=g++
CFLAGS=-c -Wall -Wextra `root-config --cflags`
LDFLAGS=`root-config --libs` `root-config --glibs` -lgmp -lm -lSpectrum
SOURCES=AnalyseGeant4ROOT.cc
OBJECTS=$(SOURCES:.cc=.o)
EXECUTABLE=AnalyseGeant4ROOT

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cc.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm ./*.o ./AnalyseGeant4ROOT
