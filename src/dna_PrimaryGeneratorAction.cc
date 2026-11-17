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
/// \file PrimaryGeneratorAction.cc
/// \brief Implementation of the PrimaryGeneratorAction class

#include "dna_PrimaryGeneratorAction.hh"

// [ADDED: PoPLaR] Snapshot generation and graceful end-of-input handling.
#include "G4Event.hh"
#include "G4PrimaryVertex.hh"
#include "G4Proton.hh"
#include "G4RunManager.hh"
#include <utility>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::PrimaryGeneratorAction(std::shared_ptr<PhaseSpaceData> source)
  : G4VUserPrimaryGeneratorAction(), fpParticleGun(0), fPhaseSpace(std::move(source))
{
  G4int n_particle = 1;
  fpParticleGun = new G4ParticleGun(n_particle);
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
  delete fpParticleGun;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
  // [ADDED: PoPLaR] Preserve the original /gun/ mode when no file was loaded.
  if (fPhaseSpace->enabled) {
    const auto index = fPhaseSpace->next.fetch_add(1, std::memory_order_relaxed);
    if (index >= fPhaseSpace->records.size()) {
      if (!fPhaseSpace->exhaustionReported.exchange(true)) {
        G4cout << "[PoPLaR] Phase-space input exhausted. "
                  "Stopping the run without recycling records." << G4endl;
      }
      // Each worker stops when it reaches EOF; already assigned records finish.
      anEvent->SetEventAborted();
      G4RunManager::GetRunManager()->AbortRun(true);
      return;
    }
    const auto& record = fPhaseSpace->records[index];
    fpParticleGun->SetParticleDefinition(G4Proton::ProtonDefinition());
    fpParticleGun->SetParticlePosition(record.position);
    fpParticleGun->SetParticleMomentumDirection(record.direction);
    fpParticleGun->SetParticleEnergy(record.energy);
    // The input is a common-time snapshot; arrival differences are in positions.
    fpParticleGun->SetParticleTime(0.0);
    fpParticleGun->GeneratePrimaryVertex(anEvent);
    anEvent->GetPrimaryVertex()->SetWeight(record.weight);
    return;
  }
  fpParticleGun->GeneratePrimaryVertex(anEvent);
}
