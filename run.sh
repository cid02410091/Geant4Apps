#!/usr/bin/env bash
set -euo pipefail
project_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)
usage() {
    printf '%s\n' \
        'Usage: bash run.sh dnaphysics [macro.in] [threads]' \
        '       bash run.sh moleculardna [macro.in] [-t threads] [-p physicsOption]' \
        '       bash run.sh chem4 [beam.in] [-mt threads] [-seed seed]' \
        '       bash run.sh chem4_plot [Species.root]' \
        'chem4_plot defaults to build/run/chem4/Species.root.' \
        'Without a macro: dnaphysics/moleculardna open an interactive session; chem4 runs beam.in.' \
        'chem4 also accepts its original options, such as -mac, -gui and -chemOFF.' \
        'Outputs and relative input paths use build/run/<application>/.'
}
case "${1:-}" in
    dnaphysics) app=dnaphysics ;;
    moleculardna) app=molecular ;;
    chem4) app=chem4 ;;
    chem4_plot) shift; exec bash "$project_dir/scripts/plot-chem4.sh" "$@" ;;
    -h|--help) usage; exit 0 ;;
    *) usage >&2; exit 2 ;;
esac
shift
source "$project_dir/scripts/geant4-env.sh"
build_dir="${GEANT4_APPS_BUILD_DIR:-$project_dir/build}"
build_dir=$(realpath -m -- "$build_dir")
if [[ ! -x "$build_dir/bin/$app" ]]; then
    printf 'Build the applications first: bash "%s/build.sh"\n' "$project_dir" >&2
    exit 1
fi
# Refresh the copied macros after edits; C++ changes require build.sh.
cmake -S "$project_dir" -B "$build_dir" -DGeant4_DIR="$Geant4_DIR" >/dev/null
run_dir="$build_dir/run/$app"
if [[ "$app" == molecular && ! -d "$run_dir/geometries" ]]; then
    printf 'Set MOLECULAR_GEOMETRY_DIR when running build.sh.\n' >&2
    exit 1
fi
args=()
if [[ $# -gt 0 && "$1" != -* ]]; then
    if [[ -f "$1" ]]; then
        macro=$(realpath -- "$1")
    elif [[ -f "$run_dir/$1" ]]; then
        macro="$run_dir/$1"
    else
        printf 'Macro not found: %s\n' "$1" >&2
        exit 2
    fi
    shift
    case "$app" in
        molecular) args+=(-m "$macro") ;;
        chem4) args+=(-mac "$macro") ;;
        *) args+=("$macro") ;;
    esac
fi
cd -- "$run_dir"
printf 'Working directory: %s\n' "$run_dir"
exec "$build_dir/bin/$app" "${args[@]}" "$@"
