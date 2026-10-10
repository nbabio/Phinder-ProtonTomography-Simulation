#include "DetectorConstruction.hh"

#include "G4FieldManager.hh"
#include "G4TransportationManager.hh"
#include "G4Mag_UsualEqRhs.hh"

#include "G4Material.hh"
#include "G4Element.hh"
#include "G4MaterialTable.hh"
#include "G4NistManager.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4SystemOfUnits.hh"

#include "G4VSolid.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4Para.hh"
#include "G4LogicalVolume.hh"
#include "G4VPhysicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4PVParameterised.hh"
#include "G4UserLimits.hh"

#include "G4SDManager.hh"
#include "G4VSensitiveDetector.hh"
#include "G4RunManager.hh"

#include "G4ios.hh"
#include "G4VisAttributes.hh"

#include "G4PVReplica.hh"

#include "G4SubtractionSolid.hh"



//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
DetectorConstruction::DetectorConstruction(ConfigurationGeometry *w) {
    myConf = w;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Destructor                                                           //
//----------------------------------------------------------------------//
DetectorConstruction::~DetectorConstruction() {

    DestroyMaterials();

}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Creates all the geometrical structures                               //
//----------------------------------------------------------------------//
G4VPhysicalVolume* DetectorConstruction::Construct() {

    //Building the materials
    ConstructMaterials();

    //Printing the geometry
    myConf->Print();

    //Manager of objects in memory
    G4SDManager* SDman = G4SDManager::GetSDMpointer();
    G4String SDname;

    //Creating the world
    G4VSolid* worldSolidPrim = new G4Box("worldBoxPrim", 1.1 * myConf->getSizeX()/2.0 , 1.1 * myConf->getSizeY() / 2.0 , 1.1 * myConf->getSizeZ()/2.0 );
    G4LogicalVolume* worldLogicalPrim = new G4LogicalVolume(worldSolidPrim, materials["air"], "worldLogicalPrim",0,0,0);
    G4VPhysicalVolume* worldPhysicalPrim = new G4PVPlacement(0, G4ThreeVector(), worldLogicalPrim, "worldPhysicalPrim", 0, 0, 0);

    G4VSolid* worldSolid = new G4Box("worldBox", myConf->getSizeX()/2.0 , myConf->getSizeY()/2.0 , myConf->getSizeZ()/2.0 );
    G4LogicalVolume* worldLogical = new G4LogicalVolume(worldSolid, materials["air"], "worldLogical",0,0,0);
    G4VPhysicalVolume* worldPhysical = new G4PVPlacement(0, G4ThreeVector(0, 0, 0), worldLogical, "worldPhysical", worldLogicalPrim, false, 0);

    //Both worlds are drawn as wireframe so the inner volumes are visible
    G4VisAttributes *worldVisAtt = new G4VisAttributes();
    worldVisAtt->SetForceWireframe(true);
    worldLogicalPrim->SetVisAttributes(worldVisAtt);
    worldLogical->SetVisAttributes(worldVisAtt);

    myConf->createG4objects(worldLogical, materials, SDman);
 
    DumpGeometricalTree(worldPhysicalPrim, 3);
    
    return worldPhysicalPrim;

}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Construct all the materials                                          //
//----------------------------------------------------------------------//
void DetectorConstruction::ConstructMaterials() {

    G4NistManager* man = G4NistManager::Instance();

    materials.insert(std::pair<G4String, G4Material *>("air", man->FindOrBuildMaterial("G4_AIR")));
    materials.insert(std::pair<G4String, G4Material *>("iron", man->FindOrBuildMaterial("G4_Fe")));
    materials.insert(std::pair<G4String, G4Material *>("uranium", man->FindOrBuildMaterial("G4_U")));
    materials.insert(std::pair<G4String, G4Material *>("aluminium", man->FindOrBuildMaterial("G4_Al")));
    materials.insert(std::pair<G4String, G4Material *>("carbon", man->FindOrBuildMaterial("G4_C")));
    materials.insert(std::pair<G4String, G4Material *>("argon", man->FindOrBuildMaterial("G4_Ar")));
    materials.insert(std::pair<G4String, G4Material *>("lead", man->FindOrBuildMaterial("G4_Pb")));
    materials.insert(std::pair<G4String, G4Material *>("silicon", man->FindOrBuildMaterial("G4_Si")));
    materials.insert(std::pair<G4String, G4Material *>("steel", man->FindOrBuildMaterial("G4_STAINLESS-STEEL")));
    materials.insert(std::pair<G4String, G4Material *>("lung", man->FindOrBuildMaterial("G4_LUNG_ICRP")));
    materials.insert(std::pair<G4String, G4Material *>("bone", man->FindOrBuildMaterial("G4_BONE_COMPACT_ICRU")));
    materials.insert(std::pair<G4String, G4Material *>("fat", man->FindOrBuildMaterial("G4_ADIPOSE_TISSUE_ICRP")));
    materials.insert(std::pair<G4String, G4Material *>("brain", man->FindOrBuildMaterial("G4_BRAIN_ICRP")));

    // Add the optical properties to the materials already defined in G4
    // Define a vector of energies in eV corresponding to the wavelengths of interest for optical properties: 500, 460, 435, 400, 380 nm (E = 1239.84/λ)
    std::vector<G4double> E = {2.48*eV, 2.70*eV, 2.85*eV, 3.10*eV, 5.0*eV};

    // OPTICAL PROPERTIES MUST BE CHANGED DEPENDING ON THE FIBER MATERIALS! (Sería buena idea ponerlo en el archivo de configuración?)

    // Core: Scintillating polystyrene
    G4Material* core = man->FindOrBuildMaterial("G4_POLYSTYRENE");
    auto* coreMPT = new G4MaterialPropertiesTable();
    coreMPT->AddProperty("RINDEX",                  E, {1.59, 1.59, 1.59, 1.59, 1.59}); // Refractive index of the core material is constant for all energies
    coreMPT->AddProperty("ABSLENGTH",               E, {3.5*m, 3.5*m, 3.0*m, 1.0*m, 0.5*m});
    coreMPT->AddProperty("SCINTILLATIONCOMPONENT1", E, {0.10, 0.60, 1.00, 0.40, 0.05}); // Emission spectrum of the scintillator (normalized to 1)
    coreMPT->AddConstProperty("SCINTILLATIONYIELD",         1000./MeV);  // Number of photons per MeV deposited; The real one is 8000; low for testing
    coreMPT->AddConstProperty("RESOLUTIONSCALE",            1.0); // Controls the intrinsic dispersion/variance in photon production (var = R * mean); 1.0 = Poisson statistics; 0.0 = no variance; 
    coreMPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1", 3.2*ns); // Scintillating time decay constant
    core->SetMaterialPropertiesTable(coreMPT); 
    core->GetIonisation()->SetBirksConstant(0.126*mm/MeV); // Light yield per path length as a function of the energy loss per path length for a particle traversing a scintillator
    materials.insert(std::pair<G4String, G4Material *>("scint_core", core));

    // Cladding: PMMA (plexiglass) (Not scintillating; refractive index lower than the core to ensure total internal reflection)
    G4Material* clad = man->FindOrBuildMaterial("G4_PLEXIGLASS");
    auto* cladMPT = new G4MaterialPropertiesTable();
    cladMPT->AddProperty("RINDEX", E, {1.49, 1.49, 1.49, 1.49, 1.49});
    clad->SetMaterialPropertiesTable(cladMPT);
    materials.insert(std::pair<G4String, G4Material *>("scint_clad", clad));

    // Air with RINDEX=1 so that light can exit the fiber without dying at the boundary. Add the optical properties of air
    auto* airMPT = new G4MaterialPropertiesTable();
    airMPT->AddProperty("RINDEX", E, {1.0, 1.0, 1.0, 1.0, 1.0});
    materials["air"]->SetMaterialPropertiesTable(airMPT);
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Destroy all the materials                                            //
//----------------------------------------------------------------------//
void DetectorConstruction::DestroyMaterials() {
    // Destroy all allocated elements and materials
    size_t i;
    G4MaterialTable* matTable = (G4MaterialTable*)G4Material::GetMaterialTable();
    for(i=0; i<matTable->size(); i++) delete (*(matTable))[i];
    matTable->clear();
    G4ElementTable* elemTable = (G4ElementTable*)G4Element::GetElementTable();
    for(i=0; i<elemTable->size(); i++) delete (*(elemTable))[i];
    elemTable->clear();

}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


void DetectorConstruction::DumpGeometricalTree(G4VPhysicalVolume* aVolume,G4int depth)
{

    for(int isp=0; isp<depth; isp++)
    {
        G4cout << "  ";
    }
    G4cout << aVolume->GetName() << "[" << aVolume->GetCopyNo() << "] "
           << aVolume->GetLogicalVolume()->GetName() << " "
           << aVolume->GetLogicalVolume()->GetNoDaughters() << " "
           << aVolume->GetLogicalVolume()->GetMaterial()->GetName();
    if(aVolume->GetLogicalVolume()->GetSensitiveDetector())
    {
        G4cout << " " << aVolume->GetLogicalVolume()->GetSensitiveDetector()->GetFullPathName();
    }
    G4cout << G4endl;
    for(int i=0; i<aVolume->GetLogicalVolume()->GetNoDaughters(); i++)
    {
        DumpGeometricalTree(aVolume->GetLogicalVolume()->GetDaughter(i),depth+1);
    }

}
