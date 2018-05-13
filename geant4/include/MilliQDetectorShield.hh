
#ifndef MilliQDetectorShield_H
#define MilliQDetectorShield_H 1

#include "G4PVPlacement.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Trd.hh"
#include "G4Material.hh"
#include "G4LogicalVolume.hh"
#include "G4OpticalSurface.hh"


class MilliQDetectorShield
{
  public:

    MilliQDetectorShield(G4LogicalVolume*,
   		 G4double dTotalStackStart, G4double dTotalStackEnd,
		 G4ThreeVector InnerShieldThickness, G4ThreeVector OutterShieldThickness,
		 G4ThreeVector ShieldDimensions, G4double ScintHouseThick,
		 G4Material* InnerShieldMaterial, G4Material* OutterShieldMaterial);

    G4LogicalVolume* ConstructShield();

  private:
    G4ThreeVector fInnerShieldThickness;
    G4ThreeVector fOutterShieldThickness;
    G4ThreeVector fDetectorInnerShieldGap;
	G4LogicalVolume* fmotherLV;
	G4double fTotalStackStart;
	G4double fTotalStackEnd;
	G4ThreeVector shield1InnerHL;
    G4double fScintHouseThick;
    G4Material* fInnerShieldMaterial;
    G4Material* fOutterShieldMaterial;



};

#endif
