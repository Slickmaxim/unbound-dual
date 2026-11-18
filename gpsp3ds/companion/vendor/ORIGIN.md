PokeDaisy portable C components
https://github.com/lidor30/pokedaisy
Base commit: 0040e61fcb129d7313e3941771572c93d6722816
Imported from local working tree, including earlier Unbound-only fixes.
GPL-3.0; see LICENSE. Adaptations: segmented ROM reader, streamed art search, read-only tab host.

v3 also imports pd_card.c from the same local tree. pd_ui_card.c uses its renderer with a single bounded buffer and a tap to flip sides, instead of the original multi-image static animation cache. Theme/scaler and art/map cache hooks are local adaptations.

ES v1.08 adaptation: exact-hash ROM configuration, display-table generator and translated Latin-font CRCs; shared Western charset extensions and interface localization hooks. Original notices are preserved. Spanish ROM display text retains its respective authorship; no game graphics are bundled.

Spanish follow-up: separate exact-hash profiles for v1.08 sinza and ES 2.1.1.1, ROM-specific generated display data/natures and font CRCs. The optional nature-name field preserves existing upstream tables when absent. All three variants use their own ROM resources and SHA-keyed cache; no extracted graphics are included.

Spanish party artwork fix: CRC-validated recovery of translated FONT_SMALL/status resources from the user's exact ROM, plus complete-party cache validation and regression checks. No game graphics are bundled.

Bag adaptation: reuse gathered/sorted read-only rows while the inventory/pocket/sort/ROM is unchanged; reset on close. Preserve original painting and touch behavior, with host-side adaptive input scheduling.
