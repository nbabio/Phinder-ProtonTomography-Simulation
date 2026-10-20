#ifndef FiberSensorHit_h
#define FiberSensorHit_h 1

#include "G4VHit.hh"
#include "G4THitsCollection.hh"
#include "G4Allocator.hh"
#include "G4ThreeVector.hh"
#include "G4LogicalVolume.hh"
#include "G4Transform3D.hh"
#include "G4RotationMatrix.hh"

class G4AttDef;
class G4AttValue;

class FiberSensorHit : public G4VHit
{
public:

    FiberSensorHit();
    virtual ~FiberSensorHit();
    FiberSensorHit(const FiberSensorHit &right);
    const FiberSensorHit& operator=(const FiberSensorHit &right);
    int operator==(const FiberSensorHit &right) const;

    inline void *operator new(size_t);
    inline void operator delete(void *aHit);

    inline float x();
    inline float y();

    virtual const std::map<G4String,G4AttDef>* GetAttDefs() const;
    virtual std::vector<G4AttValue>* CreateAttValues() const;
    virtual void Print();

private:
    G4int eventNumber;
    G4int detectorID;
    G4int layerID;
    G4int FiberID;
    G4double energy;
    G4double genEnergy;
    G4double stepLength;
    G4double time;
    G4int genID;
    G4int genTrackID;
    G4ThreeVector localPos;
    G4ThreeVector globalPos;
    

public:
    inline void SetEventNumber(G4int z) {
        eventNumber = z;
    }
    inline G4int GetEventNumber() const {
        return eventNumber;
    }
    inline void SetDetectorID(G4int z) {
        detectorID = z;
    }
    inline G4int GetDetectorID() const {
        return detectorID;
    }
    inline void SetLayerID(G4int z) {
        layerID = z;
    }
    inline G4int GetLayerID() const {
        return layerID;
    }
     inline void SetFiberID(G4int z) {
        FiberID = z;
    }
    inline G4int GetFiberID() const {
        return FiberID;
    }
    inline void SetEnergy(G4double t) {
        energy = t;
    }
    inline G4double GetEnergy() const {
        return energy;
    }
    inline void SetGenEnergy(G4double e) {
        genEnergy = e;
    }
    inline G4double GetGenEnergy() const {
        return genEnergy;
    }
    inline void SetStepLength(G4double l) {
        stepLength = l;
    }
    inline G4double GetStepLength() const {
        return stepLength;
    }
    inline void SetTime(G4double t) {
        time = t;
    }
    inline G4double GetTime() const {
        return time;
    }
    inline void SetGenID(G4int a) {
        genID = a;
    }
    inline G4int GetGenID() {
        return genID;
    }
    inline void SetGenTrackID(G4int a) {
        genTrackID = a;
    }
    inline G4int GetGenTrackID() {
        return genTrackID;
    }
    inline void SetLocalPos(G4ThreeVector xyz) {
        localPos = xyz;
    }
    inline G4ThreeVector GetLocalPos() const {
        return localPos;
    }
    inline void SetGlobalPos(G4ThreeVector xyz) {
        globalPos = xyz;
    }
    inline G4ThreeVector GetGlobalPos() const {
        return globalPos;
    }
    

};


//----------------------------------------------------------------------//
// Collections of this                                                  //
//----------------------------------------------------------------------//
typedef G4THitsCollection<FiberSensorHit> FiberSensorHitsCollection;

extern G4Allocator<FiberSensorHit> FiberSensorHitAllocator;
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Redefinition of new operator                                         //
//----------------------------------------------------------------------//
inline void* FiberSensorHit::operator new(size_t) {
    void* aHit;
    aHit = (void*)FiberSensorHitAllocator.MallocSingle();
    return aHit;
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//


//----------------------------------------------------------------------//
// Redefinition of delete operator                                      //
//----------------------------------------------------------------------//
inline void FiberSensorHit::operator delete(void* aHit) {
    FiberSensorHitAllocator.FreeSingle((FiberSensorHit*) aHit);
}
//----------------------------------------------------------------------//
//----------------------------------------------------------------------//

#endif


