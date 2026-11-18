# gpSP 3D roadmap

What's been reported, what's done, and what's next. Reports come from
[Issues](../../issues), the Reddit thread and Discord. Status as of v0.1.7 (updated Nov 8).

**Status key:** ✅ done · 🧪 done, needs testing on real hardware · 🔧 next up · 📋 planned · 💤 later / big project

## Local Unbound Dual work

| Requested by | Change | Status |
|---|---|---|
| slickmaxim | Unbound 2.1.1.1 EN read-only PARTY / BAG / MAP; PokeDaisy and gpSP styles, native MENU and original settings; bounded 2 MiB allocations, segmented ROM reads, deferred redraw | 🧪 cross-build and host fixtures; O3DS 2D speed, actual ROM art and save/load acceptance pending |

| slickmaxim | v2: ignore hidden snapshot counters for dirty redraw; Display Scaling: Nearest without the linear upscale pass | 🧪 host regression; console retest pending (v1 works, minor periodic stutter reported) |

## Released

| Version | Change | Status |
|---|---|---|
| v0.1.0 | First release: per-layer 3D, Smart layers, presets, per-game profiles, GPU display, touch menu | ✅ |
| v0.1.1 | **Crash at launch on New 3DS** fixed (heap too big for the New 3DS memory layout) | ✅ confirmed by New 3DS users (#2, #3, closed) |
| v0.1.1 | **Menu reacts instantly**: the game preview redraws after you stop pressing | ✅ |
| v0.1.1 | **3D → Floor tilt**: bottom of the screen nearer, top further, for single-layer roads | ✅ |
| v0.1.1 | **Gyro sensor** for WarioWare: Twisted, from the 3DS gyroscope | 🧪 needs the game |
| v0.1.1 | **Forwarder support**: a ROM path as the launch argument plays that game directly | ✅ (with Icon Maker) |
| v0.1.1 | Ko-fi / GitHub Sponsors button, README Support section | ✅ |
| v0.1.1 | **Display → Screen effect**: Scanlines, LCD grid | 🧪 |
| v0.1.1 | **HOME Menu icons per game**: `make-forwarders.sh` builds one forwarder CIA per game | 🧪 |
| v0.1.1 | **Auto frame skip** goes down to 1 of 6 frames on very heavy 3D screens (Mario Kart menus), so the game and sound never slow down | ✅ |
| v0.1.2 | **Closing the lid no longer freezes the game** (it waited forever for sound the system had stopped for sleep) | ✅ |
| v0.1.2 | **Faster menus**: the in-game menu looked for state files on the SD card every frame; the game list scrolls faster when held | ✅ |
| v0.1.2 | **Sharp scaling at Fit/Stretch** keeps every pixel the same size | ✅ |
| v0.1.2 | **800px wide mode in 2D** (3D slider down) at Fit / Stretch; not on Old 2DS | 🧪 (#4, closed) |
| v0.1.2 | **OpenLara black screen** fixed: an interrupt during FIQ mode corrupted r8-r12 | 🧪 (#5, closed; reopen if OpenLara still shows a black screen) |
| v0.1.2 | Dynarec: writes over RAM code drop only the pages written to (Mario Golf) | ✅ |
| v0.1.3 | **Icon Maker**: HOME Menu icons for GBA games made on the 3DS, no PC; own or generated icon and banner, a whole folder at once, delete from the app | ✅ |
| v0.1.3 | **Icon Maker: SNES games too**, opened in Snes9x 3D (v1.61-layers6+); SNES cartridge icon, GBA/SNES tags in My icons; GBA + SNES app icon and banner | ✅ |
| v0.1.3 | `make-forwarders.sh`: custom banner per game (`<game>.banner.png`, optional `.banner.wav`) | 🧪 |
| v0.1.3 | Dynarec: old RAM translations reused when the same code comes back, writes drop only the blocks they cover, cheaper patched-routine handling (Mario Golf, Pokémon GS Chronicles) | 🧪 |
| v0.1.3 | Dynarec: code run from VRAM translated instead of interpreted, and it no longer flushes all RAM code (Super Mario Galaxy GBA port) | 🧪 |
| v0.1.3 | Dynarec (ARM): RAM/ROM loads done inline when the flags aren't needed; memory handlers return in a way the CPU can predict | 🧪 |
| v0.1.3 | Render thread: bitmap modes 3-5 copy each row with its line instead of VRAM blocks | 🧪 |
| v0.1.4 | **Fast forward** on Y (hold or toggle), Max / 2x / 3x / 4x | 🧪 |
| v0.1.4 | **Zipped games work** (#6: a .zip was loaded as if it were the ROM, which hung, then crashed): unzipped once to a cache on the SD card | 🧪 (#6, closed) |
| v0.1.4 | **System -> Save file** shows where the current game's save is | 🧪 |
| v0.1.4 | **Guide viewer**: a game's `.txt` guide on the bottom screen while playing (Game -> Show guide); drag or page to scroll, place remembered per game | 🧪 (cabjoy) |
| v0.1.4 | **Own save / state folder and `.sav` saves** (`save_dir`, `state_dir`, `save_ext` in `gpsp3d.cfg`) to share saves with mGBA and open_agb_firm; `.sav` files have the real save size, old saves are still found | 🧪 (michvllni, #7 / #8) |
| v0.1.4 | **Universal-Updater store** with gpSP 3D, Icon Maker and Snes9x 3D ([bheppe.github.io/gpsp-3d](https://bheppe.github.io/gpsp-3d/)); devcontainer, CI builds for every PR, Nightly pre-release from `main`, release workflow | ✅ (michvllni, #9-#12) |

## Next up

**gpSP 3D v0.1.5: fixes and quick wins** (first release through the CI pipeline: test the Nightly, then publish)

| Change | Asked by | Status |
|---|---|---|
| **Freezes on Old 3DS**: the log was written to the SD card while playing; now it's kept in memory and written only when the menu opens or the game closes | miguelcepoi (Discord) | 🧪 waiting on his test |
| **No SD card writes while playing** at all (log, as above; saves were already written only at the menu, sleep and exit) | bheppe | 🧪 |
| **System → Real-time clock**: Auto / On / Off, for Pokémon games whose clock the auto check misses | Individual_Emu673 (Reddit) | 🧪 |
| **System → Startup**: the GBA BIOS intro, when `gba_bios.bin` is in `sdmc:/3ds/gpsp3d/` | Individual_Emu673 (Reddit) | 🧪 |
| **Tilt sensor** for Yoshi Topsy-Turvy and Koro Koro Puzzle Happy Panechu, from the 3DS accelerometer | After-Pop-2000 (Reddit) | 🧪 needs the games |
| **System → Bottom screen**: Simple (a steady screen: the game's name, "Tap the screen for the menu"; the default) / Performance (the stats) / Off after 10 s (backlight off, a tap wakes it) | PJ_MJ (Discord) | 🧪 |
| **Smoother guide scrolling** with flick (from Snes9x 3D) | cabjoy (Discord) | 🧪 |
| **System → Credits**: gpSP's authors and contributors | bheppe | 🧪 |
| **Icon Maker 0.1.4: accented letters** (Pokémon no longer shows "Pok??mon") | PJ_MJ (Discord) | 🧪 |
| **Icon Maker 0.1.4: banner style** (SELECT): 3D (the default), blue background, or no background | PJ_MJ (Discord), bheppe | ✅ |
| **New app icons** ("gp" and "s9x" monograms) so gpSP 3D and Snes9x 3D are easy to tell apart | TheBigMacWhopperGuy (Reddit) | ✅ |
| **3D HOME Menu banners** for gpSP 3D, Icon Maker, Snes9x 3D and every game icon: the icon on a turning, bobbing tile, the name under it, a soft chime | bheppe | ✅ tested on Old 3DS |
| **HOME button froze the 3DS** in v0.1.5 test builds (the bottom backlight kept a system service open) | bheppe | ✅ fixed before release |
| **Profiler no longer writes to the SD card while playing** (it wrote every 5 s with Show stats on) | bheppe | 🧪 |
| `OPT=-O3` test build of the app code (the core is already -O3) | Gaming4LifeDE (Discord) | ✅ measured on Mario Kart: no difference, stays -O2 |

**gpSP 3D v0.1.6: menu fixes, clock adjust, screen effects, overlays, update check, button mapping**

| Change | Asked by | Status |
|---|---|---|
| **Menu touch scrolling** no longer changes the setting the drag starts on: taps act when the finger lifts (sliders still follow the finger) | Gaming4LifeDE (Discord) | ✅ |
| **Speed → Show stats removed**: it only worked with the Performance bottom screen; **System → Bottom screen → Performance** now always shows everything | Gaming4LifeDE (Discord) | ✅ |
| **2DS / New 2DS XL**: the 3D tab is greyed out ("no 3D screen") | Gaming4LifeDE (Discord) | 🧪 needs a 2DS to confirm |
| **Skipped frames** on the Performance screen; the log (`gpsp3d.log`) notes the console, 3D on/off and skipped frames, for compatibility reports | Gaming4LifeDE (Discord) | ✅ |
| **System → Clock adjust**: days and hours added to the 3DS clock for the game's real-time clock, for a console whose clock is wrong | Surge And Finn (Discord) | ✅ |
| **Game report form** on GitHub Issues (asks for console, 3D on/off, skipped frames) | Gaming4LifeDE (Discord) | ✅ |
| **Icon Maker 0.1.5: JPEG pictures, any size, pad or crop** (X on the Games tab) (#17, #18) | michvllni (GitHub) | ✅ |
| **Update check**: at startup (with Wi-Fi) asks GitHub for the latest release and says so at the bottom of the game list and in System -> Version; System -> Check for updates turns it off | DukeOfJelly (Discord) | ✅ |
| **More screen effects**: Light scanlines, Light grid (GBA SP), Dot matrix | King_Hikikomori (Reddit) | ✅ |
| **Overlays**: your own 400x240 PNG over the game (`overlay.png`, or `overlays/<game>.png` per game) | King_Hikikomori (Reddit) | ✅ |
| **Button mapping** (Keys tab): each 3DS button -> any GBA button, Turbo A/B, fast forward or nothing; all games or per game | Strange_Wall_4062, Ferhath2015 (Reddit) | ✅ |

**Snes9x 3D v1.61-layers8: controls** ([snes9x_3ds](https://github.com/bheppe/snes9x_3ds))

| Change | Asked by | Status |
|---|---|---|
| **Hotkey combos** | cabjoy (Discord) | ✅ |
| **True 1:1 integer scale** (No Stretch 1:1) | Finn (Discord) | ✅ |
| **Two depth profiles per game**, switched with a hotkey (e.g. overworld vs battle in EarthBound) | Finn (Discord) | ✅ |
| **Guide viewer** on the bottom screen, like gpSP 3D's | cabjoy (Discord) | ✅ |
| **Mario is Missing!** in the Recommended list | SWChris (Discord) | ✅ |
| **New app icon, 3D banner and a soft chime** (replaces the startup sound) | bheppe | ✅ |
| Donkey Kong Country 2: thin white outline behind characters against the sky | margret (Discord) | 📋 needs investigating |
| Tiny Toon Adventures: Buster Busts Loose in the Recommended list (as Auto) | SWChris (Discord) | 📋 needs its CRC32 |

**v0.1.6, updated Nov 1: Icon Maker 0.1.7 fixes** (gpSP 3D unchanged)

| Change | Asked by | Status |
|---|---|---|
| **Icon Maker 0.1.7: faster previews, covers of any size** (#19, #20) | michvllni (GitHub) | ✅ |
| **Custom banners back**: a game's own `.banner` picture always wins, also with 3D chosen | Surge And Finn (Discord) | ✅ |
| **Chime on flat banners** too (they were silent) | Surge And Finn (Discord) | ✅ |
| **Remaking an icon with the other banner type** failed (C8E083FC): the old icon is now replaced | Surge And Finn (Discord) | ✅ |
| Custom banner sounds (`Game.banner.wav`) in Icon Maker | Surge And Finn (Discord) | 📋 next |

**gpSP 3D v0.1.8: performance**

| Plan | Status |
|---|---|
| **New 3DS: drawing on its own CPU core** instead of sharing core 2 with the system | 📋 |
| **Dynarec register allocation**: Mario Golf, the Super Mario Galaxy port, Golden Sun and WarioWare: Twisted on Old 3DS (#8, #10) | 📋 |

**Icon Maker v0.2: box art** (released with gpSP 3D v0.1.7)

| Plan | Status |
|---|---|
| **Icon Maker: box art download** (L on the Games tab; L on a folder: every game without a picture) from libretro-thumbnails (no account), found in libretro's No-Intro list by file name or CRC32 (a zip's from the zip), other regions tried when its own has none | ✅ Gaming4LifeDE, Surge, Finn (Discord) |
| Ready-to-show copies of the thumbnails (saved at 128 px on the shorter side), so making a whole folder stays fast | 🧪 |
| **Box art folder**: `boxart_dir=` in `iconmaker.cfg` (next to the game by default) | ✅ michvllni (GitHub) |
| **Your own box art libraries** (ROM hacks, translations): `boxart_source=` lines in `iconmaker.cfg`, laid out like libretro-thumbnails with a `list.txt` of CRC32s; tried before libretro | ✅ michvllni (GitHub) |
| Community art repo for original icons and banners (no copies of official box art) | 📋 |

**Later**

| Plan | Status |
|---|---|
| Tilt the layers with the 3DS (parallax from the gyroscope) | 💤 |
| Snes9x 3D: separate depths for high- and low-priority tiles of a layer (Earthworm Jim 2's flipped BG3) | 💤 SWChris (Discord) |
| DS game forwarders in Icon Maker (nds-bootstrap) | 💤 |
| Game library with RomM sync | 💤 Gaming4LifeDE |
| **Genesis 3D** | 💤 next big project |
| Screenshots and 3D GIFs in the READMEs | 📋 |
| Submit to Universal-DB (the main Universal-Updater store) | 📋 later |

## Open

| # | Problem or request | Reported by | Status |
|---|---|---|---|
| 6 | **New 3DS: use the extra CPU cores** for drawing instead of sharing core 2 with the system | — | 📋 v0.1.8 |
| 7 | Need for Speed Underground: ~4 fps on the title screen, then freezes | myownfriend (#2) | 🔧 needs the game to reproduce |
| 8 | Stutter in Mario Golf: Advance Tour, Super Mario Galaxy (GBA port), NFS Underground; WarioWare: Twisted a little choppy on Old 3DS (smooth on New 3DS) | Discord, After-Pop-2000 (Reddit) | 🔧 Mario Golf: much better in v0.1.3 (RAM code rewrites cost about half), still slows down on the course view and the swing on Old 3DS. Galaxy port: 55% -> about 76% speed on Old 3DS; the rest needs a bigger dynarec change (register allocation), on hold. Pokémon GS Chronicles: fine |
| 9 | Classic NES Series: Castlevania doesn't boot | Male_Inkling | 📋 |
| 10 | Golden Sun: big frame drops that then settle (Old 3DS) | ClockIllustrious2077 | 📋 also Camelot: may have the same cause as Mario Golf (#8), try v0.1.3 |
| 11 | Doom II: 11 fps, no depth (one bitmap layer) | Male_Inkling | 📋 depth: try Floor tilt |
| 12 | Faster 3D drawing on Old 3DS (3D pictures refresh ~15-30/s) | bheppe | 📋 |
| 13 | RetroAchievements | MysticFlipFlop (#1, open) | 📋 casual mode first; hardcore needs 6 months of releases |
| 14 | Link cable / wireless adapter multiplayer, e.g. **Pokémon trading with RetroArch's gpSP** on a PC, phone or another 3DS | Han_Solo6712, Shot_Jicama4942 (Reddit), Discord | 📋 after Genesis 3D. The gpSP core already emulates the wireless adapter and link cable (packets over RetroArch netplay); gpSP 3D needs RetroArch's netplay protocol (packet mode) and a host/join menu |
| 15 | In-app About screen crediting gpSP's authors | bheppe | 🧪 System → Credits in v0.1.5 |
| 16 | Per-layer 3D for other emulators (PS1 / PCSX) | _SleepyLark_, LargoLaGrande1, goldaxis | 💤 |
| 17 | Smashing Drive: 60-70% speed on Old 3DS (software 3D) | Advil Lavs (Discord) | 🔧 needs a profile log from a race |
| 18 | Random freezes on Old 3DS after a while | miguelcepoi (Discord) | 🧪 likely the SD log writes, fixed in v0.1.5 |

Tip while waiting for fixes: if the stats panel says **Core 2: off (drawing on main core)**, turn on
**System → Draw on core 2**; drawing on the main core costs a lot of speed.

## Works well (community reports, Old 3DS)

Sonic Advance 1 and 2, Ace Combat Advance, Asterix, Downhill Jam, Pokémon Emerald
(and the ARC hack), Mario Kart Super Circuit, Sword of Mana.

### Local companion build preference

- ✅ slickmaxim requested no further native companion builds. Build and package scripts now include PokeDaisy and the upstream baseline only; historical native files are kept.

### Unbound Dual v3 (slickmaxim)

| Change | Status |
|---|---|
| ROM-keyed versioned art/map cache, corruption fallback, atomic write and preparation stages | 🧪 Host round-trip/error tests; O3DS cold/warm timing pending |
| Light/Dark and independent bottom Nearest/Smooth settings; nearest by default | 🧪 Host theme/scaler tests; console review pending |
| Read-only PokeDaisy trainer card, five-tab footer, bounded lazy workspace | 🧪 host fixtures and CIA metadata verified; Old 3DS visual/save acceptance pending |
| Separate Companion CIA + Pokémon Unbound shortcut, configurable ROM path and one-time settings import | 🧪 Cross-build and CIA metadata/hash checks; install/HOME testing pending |

### Unbound Dual v4 (slickmaxim)

- 🧪 One CIA named Pokémon Unbound, automatic configured-ROM startup without a separate forwarder. Same title ID as the v3 application, CIA micro version 4. Missing ROM opens the picker; START cancels and exits. ROM selection remains available in System/config. Build/package only the single CIA; keep v3 historical artifacts. Console installation/autostart test pending.

- ✅ slickmaxim: clean publication source snapshot, PokeDaisy GPLv3 credit/NOTICE and source-only Git repository.

- ✅ slickmaxim: publication README with original SVG banner, install/save/settings guide and preserved PokeDaisy attribution.

- ✅ slickmaxim: add supplied top/bottom screen captures from 2026-11-16 to the publication README, converted losslessly from BMP to PNG.

- ✅ slickmaxim: embed supplied screen captures in the SVG banner; retain GitHub Actions build logs on failures. Root cause of reported CI failure awaits the failed run log.

- 🧪 slickmaxim: fix Actions missing stb_image header after Icon Maker removal; preserve header and license in gpsp3ds/third_party; trust the mounted checkout inside the build container.

- 🧪 slickmaxim: rename companion to Unbound Dual in docs, banners, HOME Menu metadata, artifact names and releases; CIA 0.0.5 retains Title ID and existing save/config paths.

- 🧪 slickmaxim: exact-hash ES v1.08 support with relocated tables/Z-A forms/stones; independent English/Español interface catalogue, accented fonts, Spanish ROM font validation and optional real-ROM integration check. Hardware Spanish-save validation pending.

- 🧪 Spanish follow-up: exact-hash profiles for v1.08 sinza and ES 2.1.1.1, separate original-size tables/natures and font CRCs. Real-ROM host checks cover all three ES builds with English overworld/battle RAM fixtures and per-ROM warm caches. User confirmed v1.08 and language switching on console; new variants still require hardware/save checks. CIA test version 0.0.7.

- Fixed Spanish party artwork fallback: recover exact-ROM translated small font/status sheets, validate required slot/backdrop resources when reading existing caches, and automatically rebuild incomplete caches. Real-ROM regression checks require the native party renderer and cover valid-but-incomplete cache repair. CIA test version 0.0.8.
- Party art lifecycle: reset owning slot/backdrop/type-sheet pointers before the allocation pool is freed, avoiding stale image frees on ROM reopen; integration checks redraw party/map after cache repair and warm reopen.

- Fast forward: removed Max, default/migrate to 2x while preserving stored 2x/3x/4x values; retained Hold/Toggle. Added an allocation-free per-frame top-right speed badge for CPU/GPU, 400/800px and stereo output, independent of bottom-screen mode and filters. GPU rectangle clipping now follows the current viewport so the 800px badge is not culled. Host checks cover compatibility, cycling and rotated framebuffer badge bounds/removal. CIA test version 0.0.9; console validation pending.

- Fixed-rate Bag presentation (CIA 0.0.11): reverted ce6daba's adaptive scrolling and inventory-row cache, restoring those paths to 135cc49. While Bag is open, force draw/present on a 33333 us timer regardless of input, dirty state, spare time or drawing cost. Other tabs keep original scheduling. No catch-up bursts after stalls; profile actual delivered Bag FPS and mean work cost. Fixed cadence host tests pass; actual console throughput and game-speed impact await validation.

- Bag scroll updates (CIA 0.0.12). Keep the 30 Hz scroll target and redraw only the list and scrollbar for drag frames. Retain the sorted rows and static artwork between full draws, and update the affected rectangle in the bottom framebuffer once both buffers contain the current page. Stationary Bag uses the existing RAM polling and redraws on visible changes. Actual game and scroll performance await console validation.


- Check Unbound Dual stable releases at startup using its own GitHub repository and project version. Show the release in settings, with installation through GitHub Releases. CIA test build 0.0.13.

- Install updates from System. GitHub HTTPS certificate verification is required. The 3DSX installer validates the executable header and keeps a .bak file. The CIA installer validates the installed title ID and requires a higher package version before streaming to Application Manager. Release assets use the names unbound-dual.cia and unbound-dual.3dsx. Console validation requires a newer stable GitHub release.
