#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")/.."
export DEVKITPRO=/opt/devkitpro DEVKITARM=/opt/devkitpro/devkitARM CTRULIB=/opt/devkitpro/libctru
export PATH="$DEVKITPRO/tools/bin:$DEVKITARM/bin:$PATH"
# The mounted checkout belongs to the host user; this container is ephemeral.
git config --global --add safe.directory "$PWD"
mkdir -p gpsp3ds/gen gpsp3ds/lib/lib gpsp3ds/out
make platform=ctr PROFILE=1 -j4
cp gpsp_libretro_ctr.a gpsp3ds/lib/lib/libgpsp.a
cd gpsp3ds
picasso -o gen/uishader.shbin -h gen/uishader.shbin.h res/uishader.v.pica res/uishader.g.pica
(cd gen && raw2c uishader.shbin)
for style in pokedaisy; do
    native=OFF
    [ "$style" != native ] || native=ON
    cmake -S companion -B companion/build-$style -DCMAKE_TOOLCHAIN_FILE=/repo/gpsp3ds/companion/toolchain.cmake -DCMAKE_BUILD_TYPE=Release -DCOMPANION_NATIVE=$native
    cmake --build companion/build-$style -j4
    cp companion/build-$style/libcompanion.a lib/lib/libcompanion-$style.a
    rm -f gpsp-pokemon-$style.smdh
    make -f gpsp3d.mk CORE=.. BUILD=build-$style TARGET=gpsp-pokemon-$style COMPANION_LIB=companion-$style APP_TITLE="gpSP Pokemon - $style v4" -j4
    cp gpsp-pokemon-$style.3dsx out/
done

# CIA tools are pinned official releases, cached locally outside source packages.
python3 tools/companion-cia-tools.py
export PATH="$PWD/.cia-tools:$PATH"
mkdir -p build-cia-companion
bannertool makesmdh -s "Pokémon Unbound" -l "Pokémon Unbound - Companion v4" -p "gpSP / PokeDaisy" -f visible,allow3d,recordusage,extendedbanner -i res/icon.png -o build-cia-companion/app.icn
makerom -f cia -o out/gpsp-pokemon-pokedaisy.cia -rsf res/app.rsf -exefslogo -elf gpsp-pokemon-pokedaisy.elf -icon build-cia-companion/app.icn -banner res/banner3d.bnr -DAPP_TITLE="UnboundComp" -DAPP_PRODUCT_CODE="CTR-UBCP" -DAPP_UNIQUE_ID=0xBAC82 -DAPP_SYSTEM_MODE=64MB -DAPP_SYSTEM_MODE_EXT=124MB -DAPP_ENCRYPTED=false -major 0 -minor 0 -micro 4
cp ../LICENSE ../COPYING ../NOTICE ../THIRD_PARTY_NOTICES.md out/
