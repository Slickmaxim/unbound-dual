# gpSP Pokémon — Unbound Companion

A Nintendo 3DS application combining [gpSP 3D](https://github.com/bheppe/gpsp-3d)
with read-only PokeDaisy tabs on the bottom screen. Intended for Pokémon Unbound
2.1.1.1 English, including Old 3DS in 2D.

Based on PokeDaisy by Lidor Itzhari - https://github.com/lidor30/pokedaisy

This is a modified community project, maintained by slickmaxim. It is not an
official release of gpSP 3D, PokeDaisy or Pokémon Unbound.

## Features

- gpSP ARM11 JIT, original emulator menu, display/audio settings and save states.
- Bottom screen: party and Pokémon details, bag, map and trainer card.
- Light/dark companion theme and independent nearest/smooth bottom filtering.
- ROM data cache for faster subsequent preparation.
- One HOME Menu CIA with automatic startup of the selected ROM.

The trainer card currently has no badge overlay. ROM badge extraction is unfinished.
Performance depends on the console, game scene and settings; this fork has no
published benchmark guaranteeing 60 FPS in every scene.

## Install

Install `gpsp-pokemon-pokedaisy.cia` with FBI, or put the `.3dsx` on your SD card.
On first launch, select your own patched Unbound 2.1.1.1 English ROM.
Subsequent launches open the configured ROM automatically.
ROM SHA-1: `b4776b82a4c7915d0fadeaa27e013523f99dfd94`.
No ROM, Nintendo BIOS or saves are supplied. An open source BIOS replacement
and its corresponding source are inherited from gpSP in `bios/`.

Configuration: `sdmc:/3ds/gpsp3d/companion-gpsp3d.cfg`.
Use **MENU → System → Save file** to see the active save path.
Use **MENU → System → Unbound ROM** to select another ROM.
[Ukrainian usage notes](README_COMPANION_UK.md) describe saves and controls.

## Build

Linux/WSL with Docker, Internet access and Python 3:

```sh
make -C 3ds 3ds
python3 gpsp3ds/companion/test/verify_cia.py gpsp3ds/out
```

The build uses `devkitpro/devkitarm:latest`, devkitARM/libctru/citro3d, CMake,
makerom 0.18.4 and bannertool 1.2.2. See `gpsp3ds/build-companion.sh` for the
complete commands. Only the PokeDaisy companion is built. Outputs are in
`gpsp3ds/out/`. The Docker image uses `latest`, so the toolchain is not fully pinned.

Host fixture checks require a separate PokeDaisy checkout:

```sh
bash gpsp3ds/companion/test/run.sh /path/to/pokedaisy/app/src/test/resources/fixtures
```

The tests check RAM readers, UI actions, allocations, caching and bottom scaling;
they do not emulate a real 3DS or establish hardware performance.

## Source and licensing

The combined companion application is distributed under GNU GPL version 3,
including PokeDaisy's attribution term in [NOTICE](NOTICE).
See [LICENSE](LICENSE), [COPYING](COPYING) and the component notices in source.
Individual upstream files retain their licenses; they have not been relicensed.
GPU support adapted from mGBA retains MPL-2.0 and Inter retains OFL-1.1.
[Third-party notices](THIRD_PARTY_NOTICES.md) record origin and modifications.

When distributing a compiled build, provide the corresponding source of the
same revision and retain its license and attribution notices alongside it.
ROMs, saves, crash dumps, extracted game art, credentials, build caches and old
releases are excluded from this source repository.

Pokémon and related names/artwork belong to their respective owners. This
project is not affiliated with Nintendo, Game Freak or The Pokémon Company.
