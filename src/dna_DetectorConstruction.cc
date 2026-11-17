//
// ********************************************************************
// * License and Disclaimer                                           *
// *                                                                  *
// * The  Geant4 software  is  copyright of the Copyright Holders  of *
// * the Geant4 Collaboration.  It is provided  under  the terms  and *
// * conditions of the Geant4 Software License,  included in the file *
// * LICENSE and available at  http://cern.ch/geant4/license .  These *
// * include a list of copyright holders.                             *
// *                                                                  *
// * Neither the authors of this software system, nor their employing *
// * institutes,nor the agencies providing financial support for this *
// * work  make  any representation or  warranty, express or implied, *
// * regarding  this  software system or assume any liability for its *
// * use.  Please see the license in the file  LICENSE  and URL above *
// * for the full disclaimer and the limitation of liability.         *
// *                                                                  *
// * This  code  implementation is the result of  the  scientific and *
// * technical work of the GEANT4 collaboration.                      *
// * By using,  copying,  modifying or  distributing the software (or *
// * any work based  on the software)  you  agree  to acknowledge its *
// * use  in  resulting  scientific  publications,  and indicate your *
// * acceptance of all terms of the Geant4 Software license.          *
// ********************************************************************
//
// This example is provided by the Geant4-DNA collaboration
// Any report or published results obtained using the Geant4-DNA software
// shall cite the following Geant4-DNA collaboration publications:
// Med. Phys. 45 (2018) e722-e739
// Phys. Med. 31 (2015) 861-874
// Med. Phys. 37 (2010) 4692-4708
// Int. J. Model. Simul. Sci. Comput. 1 (2010) 157–178
//
// The Geant4-DNA web site is available at http://geant4-dna.org
//
/// \file DetectorConstruction.cc
/// \brief Implementation of the DetectorConstruction class

#include "dna_DetectorConstruction.hh"
#include "dna_DetectorMessenger.hh"

#include "G4Box.hh"
#include "G4Exception.hh"
#include "G4LogicalVolume.hh"
#include "G4NistManager.hh"
#include "G4PVPlacement.hh"
#include "G4RunManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4VisAttributes.hh"

#include <cmath>

namespace
{
G4bool FiniteVector(const G4ThreeVector& v)
{
  return std::isfinite(v.x()) && std::isfinite(v.y()) && std::isfinite(v.z());
}
G4bool PositiveDimensions(const G4ThreeVector& v)
{
  return FiniteVector(v) && v.x() > 0. && v.y() > 0. && v.z() > 0.;
}
}

DetectorConstruction::DetectorConstruction(PhysicsList* physics)
  : fWorldDimensions(1*mm, 1*mm, 1*mm),
    fMaterialDimensions(100*um, 100*um, 100*um),
    fMaterialPosition()
{
  fTargetMaterial = G4NistManager::Instance()->FindOrBuildMaterial("G4_WATER");
  fDetectorMessenger = new DetectorMessenger(this, physics);
}

DetectorConstruction::~DetectorConstruction()
{
  delete fDetectorMessenger;
}

G4bool DetectorConstruction::TargetFits(const G4ThreeVector& world,
                                        const G4ThreeVector& size,
                                        const G4ThreeVector& position) const
{
  return std::abs(position.x()) + size.x()/2. < world.x()/2.
      && std::abs(position.y()) + size.y()/2. < world.y()/2.
      && std::abs(position.z()) + size.z()/2. < world.z()/2.;
}

G4VPhysicalVolume* DetectorConstruction::Construct()
{
  if (fPhysiWorld) return fPhysiWorld;
  if (!TargetFits(fWorldDimensions, fMaterialDimensions, fMaterialPosition)) {
    G4Exception("DetectorConstruction::Construct", "TargetOutsideWorld", FatalException,
                "The complete Target volume must fit strictly inside the vacuum World.");
    return nullptr;
  }
  auto* vacuum = G4NistManager::Instance()->FindOrBuildMaterial("G4_Galactic");
  const auto worldHalf = fWorldDimensions/2.;
  auto* worldSolid = new G4Box("World", worldHalf.x(), worldHalf.y(), worldHalf.z());
  auto* worldLogic = new G4LogicalVolume(worldSolid, vacuum, "World");
  fPhysiWorld = new G4PVPlacement(nullptr, G4ThreeVector(), worldLogic, "World",
                                 nullptr, false, 0);
  worldLogic->SetVisAttributes(G4VisAttributes::GetInvisible());

  const auto targetHalf = fMaterialDimensions/2.;
  auto* targetSolid = new G4Box("Target", targetHalf.x(), targetHalf.y(), targetHalf.z());
  fLogicTarget = new G4LogicalVolume(targetSolid, fTargetMaterial, "Target");
  new G4PVPlacement(nullptr, fMaterialPosition, fLogicTarget, "Target", worldLogic,
                    false, 0, true);
  G4VisAttributes appearance(G4Colour(0.529, 0.808, 0.922));
  appearance.SetForceSolid(true);
  fLogicTarget->SetVisAttributes(appearance);
  G4cout << "[Geometry] World: G4_Galactic; full size [mm] " << fWorldDimensions/mm
         << "; Target: " << fTargetMaterial->GetName()
         << "; full size [mm] " << fMaterialDimensions/mm
         << "; center [mm] " << fMaterialPosition/mm << G4endl;
  return fPhysiWorld;
}

G4bool DetectorConstruction::SetMaterial(const G4String& name)
{
  auto* material = G4NistManager::Instance()->FindOrBuildMaterial(name);
  if (!material) return false;
  fTargetMaterial = material;
  if (fLogicTarget) fLogicTarget->SetMaterial(material);
  G4RunManager::GetRunManager()->PhysicsHasBeenModified();
  return true;
}

G4Material* DetectorConstruction::MaterialWithDensity(G4String name, G4double density)
{
  // Retain the original example's water-density command for the Target only.
  return G4NistManager::Instance()->BuildMaterialWithNewDensity(name, "G4_WATER", density);
}

void DetectorConstruction::RebuildGeometry()
{
  if (!fPhysiWorld) return;
  fPhysiWorld = nullptr;
  fLogicTarget = nullptr;
  G4RunManager::GetRunManager()->ReinitializeGeometry(true);
}

G4bool DetectorConstruction::SetWorldDimensions(const G4ThreeVector& size)
{
  if (!PositiveDimensions(size)) return false;
  if (fPhysiWorld && !TargetFits(size, fMaterialDimensions, fMaterialPosition)) return false;
  fWorldDimensions = size;
  RebuildGeometry();
  return true;
}

G4bool DetectorConstruction::SetMaterialDimensions(const G4ThreeVector& size)
{
  if (!PositiveDimensions(size)) return false;
  if (fPhysiWorld && !TargetFits(fWorldDimensions, size, fMaterialPosition)) return false;
  fMaterialDimensions = size;
  RebuildGeometry();
  return true;
}

G4bool DetectorConstruction::SetMaterialPosition(const G4ThreeVector& position)
{
  if (!FiniteVector(position)) return false;
  if (fPhysiWorld && !TargetFits(fWorldDimensions, fMaterialDimensions, position)) return false;
  fMaterialPosition = position;
  RebuildGeometry();
  return true;
}
