# Third-party notices and provenance

Based on PokeDaisy by Lidor Itzhari - https://github.com/lidor30/pokedaisy

- **gpSP core / gpSP 3D:** Exophase, notaz, davidgfnet, bheppe and contributors.
  Source: https://github.com/bheppe/gpsp-3d at
  `717b5372f3bd9f613017d3aa71630fdbc5cea7a2`. GPL-2.0-or-later for the core;
  original notices and GPLv2 text remain in source and `COPYING`.
- **PokeDaisy portable companion:** Lidor Itzhari, GPLv3 with attribution term.
  Source: https://github.com/lidor30/pokedaisy at base
  `0040e61fcb129d7313e3941771572c93d6722816`, imported from a locally modified tree.
  See `gpsp3ds/companion/vendor/{LICENSE,NOTICE,ORIGIN.md}`. Local adaptations
  include RAM/ROM adapters, bounded allocation, read-only UI, art cache,
  trainer-card host, themes and filtering. This is a modified version.
- **RetroArch svchax:** GPLv3, aliaspider and RetroArch contributors; see
  `gpsp3ds/source/ctr_svchax.c`. https://github.com/libretro/RetroArch
- **mGBA GPU support:** Yuri Kunde Schlesner, Jeffrey Pfau and contributors,
  MPL-2.0; notices remain in `gpsp3ds/source/ctr-gpu.*` and shaders.
  https://github.com/mgba-emu/mgba
- **Inter font:** Inter Project Authors, SIL Open Font License 1.1;
  full notice in `gpsp3ds/res/Inter-LICENSE.txt`.
- **libretro-common:** its MIT/BSD and other individual file notices are
  preserved under `libretro/libretro-common/` and the libretro headers.
- **OpenGBA BIOS:** Normmatt and VBA/VBA-M contributors, distributed by gpSP;
  original GPL2 notice and source remain in `bios/` and `COPYING`.
  https://github.com/Nebuleon/ReGBA/tree/master/bios

The combined companion is distributed under GPLv3. This statement does not
replace component licenses or assign ownership of upstream works. Original
copyright notices are preserved. Game art, ROMs and Nintendo BIOS are not
licensed by this project's GPL and are not supplied here.

Changes in this publication snapshot (2026-11-16): integrated the companion into
the gpSP frontend; autostart/config, caching, theme/filter controls and trainer
card; retained original emulator settings; removed experimental badge overlay;
added PokeDaisy credit in the emulator menu. Historical binaries and development
artifacts are excluded.
