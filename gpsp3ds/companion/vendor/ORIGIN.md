PokeDaisy portable C components
https://github.com/lidor30/pokedaisy
Base commit: 0040e61fcb129d7313e3941771572c93d6722816
Imported from local working tree, including earlier Unbound-only fixes.
GPL-3.0; see LICENSE. Adaptations: segmented ROM reader, streamed art search, read-only tab host.

v3 also imports pd_card.c from the same local tree. pd_ui_card.c uses its renderer with a single bounded buffer and a tap to flip sides, instead of the original multi-image static animation cache. Theme/scaler and art/map cache hooks are local adaptations.

ES v1.08 adaptation: exact-hash ROM configuration, display-table generator and translated Latin-font CRCs; shared Western charset extensions and interface localization hooks. Original notices are preserved. Spanish ROM display text retains its respective authorship; no game graphics are bundled.
