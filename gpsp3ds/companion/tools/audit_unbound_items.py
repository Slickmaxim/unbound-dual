#!/usr/bin/env python3
"""Read-only audit of Unbound item-use entry points. GPL-3.0.

References: PokeDaisy's patch_59 and pf_120 in vendor/pd_games_gen.c.
The inputs are local ROMs. Output contains only audit results.
"""
import argparse
import hashlib
from pathlib import Path
import struct
import zlib

ROM_BASE = 0x08000000
ITEM_PROFILES = {
    "b4776b82a4c7915d0fadeaa27e013523f99dfd94": ("EN 2.1.1.1", 0x08876200, 729),
    "d5b312465c0a95bcb6428bd61cc170994a2a65fc": ("ES v1.08", 0x095FBC90, 767),
    "117449bd7822fad0db293a4f7e364787e4f8543a": ("ES v1.08 sinza", 0x08876200, 729),
    "1104054a3061b35b85b04757fbca5a3686cca60c": ("ES 2.1.1.1", 0x08876200, 729),
    "87d1485eaa64e242d39874add766a34f46cd9b32": ("RU 2.1.1.1 (P.E.)", 0x08876200, 729),
}
FUNCTIONS = {
    "CheckBagHasItem": (134848320, 3540870510),
    "RemoveBagItem": (134848984, 3289117192),
    "ArePlayerFieldControlsLocked": (134650200, 2619788452),
    "GetMonData": (134478824, 2017587717),
    "PokemonItemUseNoEffect": (134489108, 2022356295),
    "ExecuteTableBasedItemEffect": (134484928, 3940206281),
}
MEDICINE_IDS = {*range(13, 34), 38, 44, 63, 64, 65, 66, 67, 70,
                133, 134, 135, 136, 137, 139, 141, 142}
MEDICINE_HANDLER = 0x080A16E1
ITEM_STRIDE = 44


def audit(rom):
    sha = hashlib.sha1(rom).hexdigest()
    if sha not in ITEM_PROFILES:
        raise ValueError("unsupported ROM SHA-1 " + sha)
    label, items, count = ITEM_PROFILES[sha]
    if struct.unpack_from("<I", rom, 0x1C8)[0] != items:
        raise ValueError("active item table address differs from profile")
    for name, (address, expected) in FUNCTIONS.items():
        offset = address - ROM_BASE
        if zlib.crc32(rom[offset:offset + 32]) != expected:
            raise ValueError(f"{name} has an unexpected CRC")
    allowed = []
    for item in sorted(MEDICINE_IDS):
        if item >= count:
            continue
        record = items - ROM_BASE + item * ITEM_STRIDE
        record_id = struct.unpack_from("<H", rom, record + 14)[0]
        handler = struct.unpack_from("<I", rom, record + 28)[0]
        if item == record_id and handler == MEDICINE_HANDLER:
            allowed.append(item)
    expected_samples = {13, 19, 133, 139}
    if not expected_samples.issubset(allowed):
        raise ValueError("expected Potion, Full Restore, Cheri or Oran handler is missing")
    tm = items - ROM_BASE + 289 * ITEM_STRIDE
    tm_handler = struct.unpack_from("<I", rom, tm + 28)[0]
    return label, allowed, tm_handler


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("rom", type=Path, nargs="+")
    args = parser.parse_args()
    for path in args.rom:
        try:
            label, medicine, tm = audit(path.read_bytes())
        except (OSError, ValueError, struct.error) as error:
            parser.exit(1, f"{path.name}: {error}\n")
        print(f"{label}: {len(FUNCTIONS)} native CRCs match. "
              f"{len(medicine)} medicine records match. TM01 field callback {tm:#010x}.")


if __name__ == "__main__":
    main()
