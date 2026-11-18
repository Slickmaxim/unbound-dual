# Unbound Dual — Español (versión de prueba)

Unbound Dual combina gpSP 3D con pestañas de PokeDaisy en la pantalla inferior.
Esta rama añade compatibilidad con **Unbound ES v1.08, v1.08 sinza y ES 2.1.1.1** y permite elegir
**English / Español** para los menús y las pestañas.

## ROM compatible

- Archivo probado: `unbound-es-v1.08.gba`, 32 MiB.
- SHA-1: `d5b312465c0a95bcb6428bd61cc170994a2a65fc`.
- Se mantiene la compatibilidad con Unbound 2.1.1.1 English.
- También se admiten `unbound-es-v1.08-sinza.gba` (`117449bd7822fad0db293a4f7e364787e4f8543a`)
  y `Pokémon Unbound v2.1.1.1.gba` (`1104054a3061b35b85b04757fbca5a3686cca60c`).
- Solo se aceptan estos SHA-1; cambiar el nombre del archivo no añade compatibilidad.
- El usuario confirmó v1.08 en consola. Las otras dos ROM han pasado las pruebas de host y necesitan una prueba en 3DS.

No se incluye ningún ROM ni archivo de guardado.

## Idioma e instalación

Instala el CIA de prueba con FBI o abre el 3DSX mediante Homebrew Launcher.
La versión interna del CIA es 0.0.7 y conserva el Title ID anterior.
Haz una copia de seguridad de tu partida antes de probarlo.

Abre **MENU → Display → Interface language → Español**. Para volver a inglés,
usa **MENÚ → Pantalla → Idioma / Language → English**.
La opción se guarda como `interface_language=es` en
`sdmc:/3ds/gpsp3d/companion-gpsp3d.cfg`.

El idioma de la interfaz es independiente del ROM. Los nombres de Pokémon,
movimientos, objetos y lugares corresponden al ROM seleccionado.
Los textos integrados en los gráficos originales conservan su propio idioma.

## Estado de las pruebas

La preparación de recursos, las tablas trasladadas, los nuevos objetos,
la caché y las pestañas se han comprobado en el ordenador con el ROM español
y una captura de RAM inglesa de formato común. Falta validar una partida
española real y el rendimiento en una consola 3DS.

Comprueba el equipo, los PS/PP, objetos, mapa y guardado dentro del juego.
No transfieras save states entre ROMs diferentes.

[README principal](README.md) · [Notas técnicas de prueba](docs/SPANISH_SUPPORT_UK.md)

Based on PokeDaisy by Lidor Itzhari - https://github.com/lidor30/pokedaisy
