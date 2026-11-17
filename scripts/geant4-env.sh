#!/usr/bin/env bash
# Sourced by build.sh and run.sh. Override GEANT4_ROOT for another installation.
export GEANT4_ROOT="${GEANT4_ROOT:-$HOME/Applications/GEANT4}"
export Geant4_DIR="${Geant4_DIR:-$GEANT4_ROOT/geant4-install/lib/cmake/Geant4}"
export GEANT4_DATA_DIR="${GEANT4_DATA_DIR:-$GEANT4_ROOT/G4DATA}"
export LD_LIBRARY_PATH="$GEANT4_ROOT/geant4-install/lib${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
if [[ ! -f "$Geant4_DIR/Geant4Config.cmake" ]]; then
    printf 'Geant4Config.cmake not found in %s; set Geant4_DIR.\n' "$Geant4_DIR" >&2
    return 1
fi
