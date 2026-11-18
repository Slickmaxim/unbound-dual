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
- El usuario confirmó las tres ROM en consola. También confirmó la corrección del diseño del equipo. El indicador de avance rápido también está confirmado. El desplazamiento adaptativo de la mochila necesita una prueba en 3DS.

No se incluye ningún ROM ni archivo de guardado.

## Idioma e instalación

Instala el CIA de prueba con FBI o abre el 3DSX mediante Homebrew Launcher.
La versión interna del CIA es 0.0.10 y conserva el Title ID anterior.
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

## Avance rápido

En Speed se puede elegir 2x, 3x o 4x. Se elimina Max; el antiguo valor Max
pasa a 2x y las velocidades guardadas 2x/3x/4x conservan su significado.
Mientras está activo aparece un pequeño `>> 2x/3x/4x` en la esquina superior
derecha de la pantalla de juego. Indica la velocidad seleccionada;
la velocidad real depende del juego y de la consola. Funcionan Mantener y Alternar.

## Desplazamiento de la mochila

Al arrastrar la lista se intentan 24–30 actualizaciones por segundo, separadas
del sondeo lento de la RAM. Se reutilizan los elementos y su orden mientras
no cambien. Si el dibujo cuesta demasiado, la frecuencia baja para limitar
el impacto en el juego; una mochila inmóvil no se redibuja continuamente.
El objetivo no garantiza el mismo rendimiento en todas las consolas.
