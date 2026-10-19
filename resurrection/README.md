# Resurrection

The original C sources of Plane Meta Splitter 1.0 (Delirio, November 1996) were lost.
Only the printouts survived, scanned into `../papers`. This folder holds a faithful
transcription of those scans, plus what's needed to compile it today.

| File          | Origin                                                         |
|---------------|----------------------------------------------------------------|
| `planespl.c`  | Transcription of `papers/planespl_c_01..12_of_12.jpg`          |
| `Debug.c`     | Transcription of `papers/debug_c_1..5_of_5.jpg`, `#include`d by `planespl.c` |
| `lwpanel.h`   | **Reconstruction**: the LWPanels header isn't in the 5.0 SDK, so it declares only what the plugin uses |
| `Makefile`    | Builds against the original NewTek SDK in `../sdk/lwsdk50.zip` |

## Transcription rules

- The code is exactly as printed. Nothing was fixed or modernised, including the original quirks.
- Long lines that the printer wrapped at the page edge have been joined back into single lines.
- The transcription was proofread line by line against the scans.

## Build

```sh
make syntax   # syntax check only
make          # builds planespl.so (gcc, -std=gnu89 -D_XGL), linked with the SDK's serv_w.c/slib*.c
make clean
```

The Makefile unpacks the SDK into `build/` and creates lowercase names for its
uppercase headers. The zip itself is not changed. It needs `gcc`, `make` and `unzip`.

## Warnings from the original code

The build has no errors. With `-Wall`, a modern gcc warns about some 1996-era constructs, kept on purpose:

- `PntID *PtID1, *PtID2;` in `MyData` is declared as a pointer to `PntID`, but it is used as a `PntID`.
  This was harmless on 32-bit Win32, where all pointers have the same size.
- `DoneLogFile;` is written without `()`, so it never runs. The log file is closed only when the host process exits.
- `XCALL_(int);` has a stray `;`, so `Activate` gets an implicit `int` return type. The same happens with `CmpCutDirection`.
- `%d`/`%i` are used with `long`s and handles, and `polyScan` callbacks take `MyData *` instead of `void *`.
- Some debug helpers are unused.
