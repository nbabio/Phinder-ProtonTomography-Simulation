#ifndef Fiber_h
#define Fiber_h 1

#include "GeomObject.hh"
#include <tuple>
#include "G4Tubs.hh"

class Fiber : GeomObject {

public:

    Fiber(G4double, G4double, G4double, 
         G4double, G4double, G4double, 
         G4double, G4double, G4double,
	     G4int, G4int, G4int,
         G4double, G4double, G4double,
         G4String, G4String);
         

    G4int detId();

    G4int layerId();

    G4int fiberId();

    G4double getCoreRadius();

    G4double getCladdingRadius();

    G4String getCoreMaterial();

    G4String getCladdingMaterial();
   
    void createG4Objects(G4String, G4LogicalVolume *, 
                         std::map<G4String, G4Material*> &,
                         G4SDManager *);
    
    void Print(); 


private:
    G4int ndetId, nlayerId, nfiberId;
    G4double coreRad, claddingRad;
    G4double length;
    G4String coreMaterial, claddingMaterial;
    G4LogicalVolume *logicalVolumeCore, *logicalVolumeCladding;
    G4Tubs *solidVolumeCore, *solidVolumeCladding;
    G4PVPlacement *physicalVolumeCore, *physicalVolumeCladding; 

};



#endif

