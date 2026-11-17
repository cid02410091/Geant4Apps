#!/usr/bin/env bash
# Open the official chem4 ROOT GUI. The input defaults to this project's run output.
set -euo pipefail
project_dir=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
usage() {
    printf '%s\n' \
        'Usage: geant4apps chem4_plot [Species.root]' \
        'No file: open build/run/chem4/Species.root.' \
        'Relative file: look in the current directory, then build/run/chem4/.' \
        'Absolute file: open that exact path.'
}
if [[ "${1:-}" == -h || "${1:-}" == --help ]]; then
    usage
    exit 0
fi
if [[ $# -gt 1 ]]; then
    usage >&2
    exit 2
fi
root_dir="${ROOTSYS:-$HOME/Applications/ROOT/6.40.04/root}"
if [[ ! -f "$root_dir/bin/thisroot.sh" ]]; then
    printf 'ROOT not found: %s\nSet ROOTSYS to your ROOT installation.\n' "$root_dir" >&2
    exit 1
fi
source "$root_dir/bin/thisroot.sh"
build_dir="${GEANT4_APPS_BUILD_DIR:-$project_dir/build}"
build_dir=$(realpath -m -- "$build_dir")
program="$build_dir/analysis/chem4_plot/plotG"
input="${1:-$build_dir/run/chem4/Species.root}"
if [[ ! -f "$input" && "$input" != /* && -f "$build_dir/run/chem4/$input" ]]; then
    input="$build_dir/run/chem4/$input"
fi
if [[ ! -x "$program" ]]; then
    printf 'Build plotG first: bash "%s/build.sh" -DROOT_DIR="%s/cmake"\n' "$project_dir" "$root_dir" >&2
    exit 1
fi
if [[ ! -f "$input" ]]; then
    printf 'ROOT file not found: %s\n' "$input" >&2
    exit 1
fi
input=$(realpath -- "$input")
cd -- "$(dirname -- "$input")"
exec "$program" "$input"
