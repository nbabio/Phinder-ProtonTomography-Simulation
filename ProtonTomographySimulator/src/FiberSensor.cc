#include "FiberSensor.hh"
#include "FiberSensorHit.hh"
#include "G4HCofThisEvent.hh"
#include "G4TouchableHistory.hh"
#include "G4Track.hh"
#include "G4Step.hh"
#include "G4Event.hh"
#include "G4SDManager.hh"
#include "G4EventManager.hh"
#include "G4Navigator.hh"
#include "G4ios.hh"
#include "CLHEP/Random/RandGaussQ.h"
#include "CLHEP/Random/RandFlat.h"
#include "G4OpticalPhoton.hh"

#include <tuple>

//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
FiberSensor::FiberSensor(G4String name, G4String collection)
    :G4VSensitiveDetector(name)
{
    G4String HCname;
    collectionName.insert(collection);
    HCID = -1;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
FiberSensor::~FiberSensor() {
    ;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Constructor                                                          //
//----------------------------------------------------------------------//
void FiberSensor::Initialize(G4HCofThisEvent*HCE)
{
    hitsCollection = new FiberSensorHitsCollection(SensitiveDetectorName, collectionName[0]);
    if(HCID<0) HCID = G4SDManager::GetSDMpointer()->GetCollectionID(hitsCollection);
    HCE->AddHitsCollection(HCID,hitsCollection);
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Process the hits and fills the relevant information                  //
//----------------------------------------------------------------------//
G4bool FiberSensor::ProcessHits(G4Step*aStep,G4TouchableHistory*  /*ROhist*/) {

    // Optical photons are simulated, but not stored as hits.
    if(aStep->GetTrack()->GetDefinition() == G4OpticalPhoton::Definition()) return false;

    G4Event *event = G4EventManager::GetEventManager()->GetEventManager()->GetNonconstCurrentEvent();
    if(event == NULL) false;
    G4int enumber = event->GetEventID();
    G4StepPoint* preStepPoint = aStep->GetPreStepPoint();
    G4TouchableHistory* theTouchable = (G4TouchableHistory*)(preStepPoint->GetTouchable());
  
    G4ThreeVector worldPos = preStepPoint->GetPosition();
    G4ThreeVector localPos = theTouchable->GetHistory()->GetTopTransform().TransformPoint(worldPos);
    G4int detector = fiber->detId();
    G4int layer = fiber->layerId();
    G4int sensor = fiber->fiberId();
    
    G4double energy = aStep->GetTotalEnergyDeposit();
    G4double genEnergy = aStep->GetPreStepPoint()->GetTotalEnergy();
    G4int genID = aStep->GetTrack()->GetParticleDefinition()->GetPDGEncoding();
    G4int genTrackID = aStep->GetTrack()->GetTrackID();

    //Simulating resolution
    FiberSensorHit* aHit = new FiberSensorHit();
    aHit->SetEventNumber(enumber);
    aHit->SetDetectorID(detector);
    aHit->SetLayerID(layer);
    aHit->SetFiberID(sensor);
    aHit->SetLocalPos(localPos);
    aHit->SetGlobalPos(worldPos);
    aHit->SetEnergy(energy);
    aHit->SetGenEnergy(genEnergy);
    aHit->SetGenID(genID);
    aHit->SetGenTrackID(genTrackID);

    
    hitsCollection->insert(aHit);

    return true;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// Set Fiber                                                             //
//----------------------------------------------------------------------//
void FiberSensor::setFiber(Fiber *a) {
    fiber = a;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

//----------------------------------------------------------------------//
// Get Fiber                                                             //
//----------------------------------------------------------------------//
Fiber * FiberSensor::getFiber() {
    return fiber;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Needed by the parent class                                           //
//----------------------------------------------------------------------//
void FiberSensor::EndOfEvent(G4HCofThisEvent* /*HCE*/) {
    ;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

