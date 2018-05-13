#include "MilliQDetectorShield.hh"
#include "G4VSensitiveDetector.hh"

#include "G4LogicalSkinSurface.hh"
#include "G4RotationMatrix.hh"
#include "G4SubtractionSolid.hh"
#include "G4Colour.hh"
#include "G4VisAttributes.hh"

#include "G4SystemOfUnits.hh"
#include "G4Material.hh"

MilliQDetectorShield::MilliQDetectorShield(G4LogicalVolume* dmotherLV,
		 G4double TotalStackStart, G4double TotalStackEnd,
		 G4ThreeVector InnerShieldThicknes, G4ThreeVector OutterShieldThicknes,
		 G4ThreeVector ShieldDimensions,G4double ScintHouseThick,
		 G4Material* InnerShieldMaterial, G4Material* OutterShieldMaterial){

	fmotherLV 		= dmotherLV;
	fTotalStackStart 		= TotalStackStart;
	fTotalStackEnd 	 		= TotalStackEnd;
	fInnerShieldThickness	= InnerShieldThicknes;
	fOutterShieldThickness	= OutterShieldThicknes;
	shield1InnerHL 			= ShieldDimensions;
    fScintHouseThick 		= ScintHouseThick;
    fInnerShieldMaterial	= InnerShieldMaterial;
    fOutterShieldMaterial	= OutterShieldMaterial;

  }


G4LogicalVolume* MilliQDetectorShield::ConstructShield() {

  //
  //Shielding Around Experiment
  //

  //Define center location of led shielding (relative to global center)
  G4ThreeVector centreShield1 = G4ThreeVector(0, 0, 0);

  //Define global center for shielding
  G4ThreeVector centreGlobalShield = G4ThreeVector(
						   fScintHouseThick + fTotalStackStart
						   + (fTotalStackEnd - fTotalStackStart) * 0.5, 0, 0);

  //
  // Outter (Polyethylene) Shielding Container (Radiation Shield)
  //
  G4ThreeVector shield2InnerHL = shield1InnerHL + fInnerShieldThickness+G4ThreeVector(0.02*cm,0.02*cm,0.02*cm);
  G4ThreeVector BoxOutSideShield2HL = shield2InnerHL + fOutterShieldThickness;

  G4Box *boxInSideShield2 = new G4Box("BoxInSideShield2",
				      shield2InnerHL.getX(), shield2InnerHL.getY(),
				      shield2InnerHL.getZ());

  G4Box *boxOutSideShield2 = new G4Box("BoxOutSideShield2",
				       BoxOutSideShield2HL.getX(), BoxOutSideShield2HL.getY(),
				       BoxOutSideShield2HL.getZ());

  // boolean logic subtraction

  G4SubtractionSolid* OutMinusInBoxShield2 = new G4SubtractionSolid(
								    "OutMinusInBoxShield2", boxOutSideShield2, boxInSideShield2, 0,
								    G4ThreeVector());

  G4LogicalVolume *OutMinusInBoxShield2LV = new G4LogicalVolume(
								OutMinusInBoxShield2, fOutterShieldMaterial, "OutMinusInBoxShield2LV", 0, 0,
								0);
  new G4PVPlacement(0, centreGlobalShield, OutMinusInBoxShield2LV,
		    "OutMinusInBoxShield2PV", fmotherLV, false, 0, false);

  OutMinusInBoxShield2LV->SetVisAttributes(G4Colour(0., 0., 1.)); //Blue

  //
  // Inner (Led) Shielding Container (Neutron Shield)
  //

  G4ThreeVector BoxOutSideShield1HL = shield1InnerHL + fInnerShieldThickness;

  G4Box *boxInSideShield1 = new G4Box("BoxInSideShield1",
				      shield1InnerHL.getX(), shield1InnerHL.getY(),
				      shield1InnerHL.getZ());

  G4Box *boxOutSideShield1 = new G4Box("BoxOutSideShield1",
				       BoxOutSideShield1HL.getX(), BoxOutSideShield1HL.getY(),
				       BoxOutSideShield1HL.getZ());

  // boolean logic subtraction

  G4SubtractionSolid* OutMinusInBoxShield1 = new G4SubtractionSolid(
								    "OutMinusInBoxShield1", boxOutSideShield1, boxInSideShield1, 0,
								    G4ThreeVector());

  // Visualisation attributes of Shield1
  G4VisAttributes * grayBox = new G4VisAttributes(G4Colour(0.5, 0.5, 0.5));


  G4LogicalVolume *OutMinusInBoxShield1LV = new G4LogicalVolume(
								OutMinusInBoxShield1, fInnerShieldMaterial, "OutMinusInBoxShield1LV", 0, 0, 0);

  OutMinusInBoxShield1LV->SetVisAttributes(grayBox);


  new G4PVPlacement(0, G4ThreeVector(), OutMinusInBoxShield1LV,
		    "OutMinusInBoxShield1PV", OutMinusInBoxShield2LV, false, 0,false);

  return OutMinusInBoxShield1LV;


}
