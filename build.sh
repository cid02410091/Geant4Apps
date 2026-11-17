#!/usr/bin/env bash
set -euo pipefail
project_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
source "$project_dir/scripts/geant4-env.sh"
build_dir="${GEANT4_APPS_BUILD_DIR:-$project_dir/build}"
cmake -S "$project_dir" -B "$build_dir" \
    -DGeant4_DIR="$Geant4_DIR" -DCMAKE_BUILD_TYPE=Release "$@"
cmake --build "$build_dir" --parallel "${BUILD_JOBS:-2}"
