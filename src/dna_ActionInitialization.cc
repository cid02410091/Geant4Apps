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
/// \file ActionInitialization.cc
/// \brief Implementation of the ActionInitialization class

#include "dna_ActionInitialization.hh"

#include "dna_PrimaryGeneratorAction.hh"
#include "dna_RunAction.hh"
#include "dna_SteppingAction.hh"
#include "dna_TrackingAction.hh"

// [ADDED: PoPLaR] TOPAS ASCII single-proton input and its UI command.
#include "G4GenericMessenger.hh"
#include "G4SystemOfUnits.hh"
#include "G4UIcommand.hh"
#include <cmath>
#include <fstream>
#include <sstream>
#include <utility>

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ActionInitialization::ActionInitialization()
  : G4VUserActionInitialization(), fPhaseSpace(std::make_shared<PhaseSpaceData>())
{
  // [ADDED: PoPLaR] Register before /run/initialize, also in multithreaded mode.
  fMessenger = std::make_unique<G4GenericMessenger>(
      this, "/dna/source/", "Primary source settings");
  auto& command = fMessenger->DeclareMethod(
      "phaseSpaceFile", &ActionInitialization::LoadPhaseSpace,
      "Load TOPAS ASCII records: one proton per history; positions in cm, energy in MeV.");
  command.SetStates(G4State_PreInit).SetToBeBroadcasted(false);
  command.SetParameterName("fileName", false);
  fLoadCommand = command.command;
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

ActionInitialization::~ActionInitialization() {}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::BuildForMaster() const
{
  SetUserAction(new RunAction());
}

//....oooOO0OOooo........oooOO0OOooo........oooOO0OOooo........oooOO0OOooo......

void ActionInitialization::Build() const
{
  SetUserAction(new PrimaryGeneratorAction(fPhaseSpace));
  SetUserAction(new RunAction());
  SetUserAction(new TrackingAction());
  SetUserAction(new SteppingAction());
}

// [ADDED: PoPLaR] Validate the entire file before enabling the new source.
void ActionInitialization::LoadPhaseSpace(G4String fileName)
{
  const auto fail = [this](const G4String& message) {
    G4ExceptionDescription description;
    description << message;
    fLoadCommand->CommandFailed(description);
  };
  // Accept quoted filenames as well as the usual relative path.
  if (fileName.size() >= 2 && fileName.front() == '"' && fileName.back() == '"') {
    fileName = fileName.substr(1, fileName.size() - 2);
  }
  std::ifstream input(fileName);
  if (!input) {
    fail("Cannot open phase-space file: " + fileName);
    return;
  }

  std::vector<PhaseSpaceRecord> records;
  G4String line;
  std::size_t lineNumber = 0;
  while (std::getline(input, line)) {
    ++lineNumber;
    const auto first = line.find_first_not_of(" \t\r");
    if (first == G4String::npos || line[first] == '#') continue;
    G4double x, y, z, u, v, energy, weight;
    G4int pdg, negativeZ, newHistory;
    G4String extra;
    std::istringstream row(line);
    if (!(row >> x >> y >> z >> u >> v >> energy >> weight
              >> pdg >> negativeZ >> newHistory) || (row >> extra)) {
      fail("Expected exactly ten TOPAS ASCII columns at " + fileName + ":"
           + std::to_string(lineNumber));
      return;
    }
    const auto transverse = u*u + v*v;
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)
        || !std::isfinite(u) || !std::isfinite(v) || !std::isfinite(transverse)
        || transverse > 1.0 || !std::isfinite(energy) || energy <= 0.0
        || !std::isfinite(x*cm) || !std::isfinite(y*cm) || !std::isfinite(z*cm)
        || !std::isfinite(energy*MeV)
        || !std::isfinite(weight) || weight <= 0.0
        || pdg != 2212 || (negativeZ != 0 && negativeZ != 1) || newHistory != 1) {
      fail("Invalid single-proton record at " + fileName + ":"
           + std::to_string(lineNumber)
           + "; require finite values, positive energy/weight, valid direction, "
             "PDG=2212, negativeZ=0/1 and newHistory=1.");
      return;
    }
    auto w = std::sqrt(1.0 - transverse);
    if (negativeZ) w = -w;
    records.push_back({G4ThreeVector(x*cm, y*cm, z*cm),
                       G4ThreeVector(u, v, w), energy*MeV, weight});
  }
  if (input.bad() || records.empty()) {
    fail("Empty or unreadable phase-space file: " + fileName);
    return;
  }
  fPhaseSpace->records = std::move(records);
  fPhaseSpace->next.store(0);
  fPhaseSpace->exhaustionReported.store(false);
  fPhaseSpace->enabled = true;
  G4cout << "[PoPLaR] Loaded " << fPhaseSpace->records.size()
         << " proton records from " << fileName
         << "; one record per event, no automatic recycling." << G4endl;
}
