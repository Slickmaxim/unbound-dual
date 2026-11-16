# Working on Unbound Dual

This is a GPLv3 companion fork of gpSP 3D with PokeDaisy portable C components.
Read README.md, NOTICE and THIRD_PARTY_NOTICES.md before changing attribution.
Preserve: Based on PokeDaisy by Lidor Itzhari - https://github.com/lidor30/pokedaisy

Build only PokeDaisy: `make -C 3ds 3ds`. Never build the native companion.
Keep ROMs, saves, dumps, extracted game art and credentials out of Git.
Runtime drawing and JIT must remain independent of companion reads. Keep SD
writes deferred to the menu/exit. Target Old 3DS; do not promise measured FPS
without actual measurements. The companion remains read-only.

Source: core in repository root, 3DS frontend in gpsp3ds/source, companion in
gpsp3ds/companion, vendor adaptations in gpsp3ds/companion/vendor.
Host checks: gpsp3ds/companion/test/run.sh requires external PokeDaisy fixtures.
CIA verification: python3 gpsp3ds/companion/test/verify_cia.py gpsp3ds/out.
Update ROADMAP.md for behavior changes. Preserve authorship and per-file notices.
