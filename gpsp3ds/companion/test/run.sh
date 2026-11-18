#!/bin/bash
set -euo pipefail
if [ $# -ne 1 ]; then echo "usage: $0 /path/to/pokedaisy/app/src/test/resources/fixtures" >&2; exit 2; fi
fixtures=$(realpath "$1")
root=$(cd "$(dirname "$0")/.." && pwd)
for style in pokedaisy; do
    native=OFF
    [ "$style" != native ] || native=ON
    build="$root/build-test-$style"
    cmake -S "$root" -B "$build" -DCMAKE_BUILD_TYPE=Release -DCOMPANION_TESTS=ON -DCOMPANION_NATIVE=$native
    cmake --build "$build" -j4
    mkdir -p "$build/shots"
    "$build/companion-fixture-test" "$fixtures" "$build/shots"
    "$build/companion-adapter-test" "$build/synthetic-map.gba"
    "$build/companion-filter-test"
    "$build/companion-fast-forward-test"
    "$build/companion-scroll-refresh-test"
done
