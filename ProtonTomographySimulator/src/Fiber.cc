

#include "Fiber.hh"
#include "FiberSensor.hh"
#include "G4VisAttributes.hh"

//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
Fiber::Fiber(G4double xPos, G4double yPos, G4double zPos,
           G4double xRot, G4double yRot, G4double zRot,
           G4double xSize, G4double ySize, G4double zSize,
           G4int ndet, G4int nlayer, G4int nfiber_, 
           G4double coreRad_, G4double claddingRad_, G4double length_,
           G4String coreMaterial_, G4String claddingMaterial_):
           GeomObject(xPos, yPos, zPos, xRot, yRot, zRot, xSize, ySize, zSize) {
            ndetId = ndet;
            nlayerId = nlayer;
            nfiberId = nfiber_;
            coreRad = coreRad_;
            claddingRad = claddingRad_;
            length = length_;
            coreMaterial = coreMaterial_;
            claddingMaterial = claddingMaterial_;
};
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// Return core radius                                                   //
//----------------------------------------------------------------------//
G4double Fiber::getCoreRadius() {
	return coreRad;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// Return cladding radius                                               //
//----------------------------------------------------------------------//
G4double Fiber::getCladdingRadius() {
	return claddingRad;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// Return core material                                                 //
//----------------------------------------------------------------------//
G4String Fiber::getCoreMaterial() {
	return coreMaterial;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// Return cladding material                                             //
//----------------------------------------------------------------------//
G4String Fiber::getCladdingMaterial() {
	return claddingMaterial;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// Return detId                                                         //
//----------------------------------------------------------------------//
G4int Fiber::detId() {
	return ndetId;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// Return layerId                                                       //
//----------------------------------------------------------------------//
G4int Fiber::layerId() {
	return nlayerId;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// Return fiberId                                                         //
//----------------------------------------------------------------------//
G4int Fiber::fiberId() {
	return nfiberId;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// createG4Objects                                                      //
//----------------------------------------------------------------------//
void Fiber::createG4Objects(G4String name, G4LogicalVolume *mother,
                               std::map<G4String, G4Material*> &materials,
                               G4SDManager *SDman) {

    //This is the Fiber	 
    G4String FiberNameCore = G4String("FiberCore_") + name;  
    solidVolumeCore = new G4Tubs(FiberNameCore, 0.0, coreRad, length, 0.0, 2.0*CLHEP::pi);
    logicalVolumeCore = new G4LogicalVolume(solidVolumeCore, materials[coreMaterial], FiberNameCore);
    G4String FiberCorePhysicalName = G4String("FiberCorePhys_") + name;
    physicalVolumeCore = new G4PVPlacement(getRot(), getPos(), 
                                       logicalVolumeCore, FiberCorePhysicalName,
                                       mother, false, 0, true);
    G4String FiberNameCladding = G4String("FiberCladding_") + name;  
    solidVolumeCladding = new G4Tubs(FiberNameCladding, coreRad, claddingRad, length, 0.0, 2.0*CLHEP::pi);
    logicalVolumeCladding = new G4LogicalVolume(solidVolumeCladding, materials[claddingMaterial], FiberNameCladding);
    G4String FiberCladdingPhysicalName = G4String("FiberCladdingPhys_") + name;
    physicalVolumeCladding = new G4PVPlacement(getRot(), getPos(), 
                                       logicalVolumeCladding, FiberCladdingPhysicalName,
                                       mother, false, 0, true);                   
    

    //We need to make this object sensitive
    G4String SDname = G4String("FiberSensor") + name;
    G4String Collection = G4String("HitsCollection_") + name;
    FiberSensor *fiberSensor = new FiberSensor(SDname = SDname, Collection);
    fiberSensor->setFiber(this);
    SDman->AddNewDetector(fiberSensor);
    logicalVolumeCore->SetSensitiveDetector(fiberSensor);                                  

}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Print                                                                //
//----------------------------------------------------------------------//
void Fiber::Print() {

    G4cout << "\033[1;34m" << "Fiber" << "\033[0m" << G4endl;
    G4cout << "\033[1;34m" << "Location x: " << pos.x()/CLHEP::cm << ", y: " << pos.y()/CLHEP::cm << ", z: " << pos.z()/CLHEP::cm << G4endl;
    G4cout << "\033[1;34m" << "Rotation x: " << rots.x() << ", y: " << rots.y() << ", z: " << rots.z() << G4endl;
    G4cout << "\033[1;34m" << "Sizes x: " << sizes.x()/CLHEP::cm << ", y: " << sizes.y()/CLHEP::cm << ", z: " << sizes.z()/CLHEP::cm << G4endl;
    G4cout << "\033[0m" << G4endl;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


