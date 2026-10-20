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

## Fixes applied

The first commit of this folder (87f3792) holds the verbatim transcription of the 1996 sources. The following commit applies the fixes below, so that the build has no `-Wall` warnings. Every fix is marked in the code with a `RESURRECT FIX` comment that gives the reason and keeps the original text.

- `EDERR_FOUNDPOL`: removed the stray `;` from the macro.
- `MyData`: `PtID1`/`PtID2` are now `PntID`, not `PntID *` (it worked on Win32 only because pointers had the same size).
- `CmpCutDirection`: added the missing `static int` return type.
- `XCALL_(int);` before `Activate`: removed the stray `;`, so `Activate` no longer gets an implicit `int`.
- `DoneLogFile;` is now `DoneLogFile();`, so the log file is really closed.
- `ScanAdiacentPoly` and `FindFirstPoly`: the first parameter is now `void *Data`, as the SDK `PolyScanFunc` type requires, and the typed `MyData *localData` is recovered from it.
- Format strings: `%ld` for `long` values, `%p` with `(void *)` casts for `PntID` and `PolID` handles.
- `CreateLogFile`: if `fopen` fails, logging is disabled; `DoneLogFile` checks `LogF` and sets it to `NULL` after `fclose`.
- `Debug.c`: `const` added to `_StoreLog_PointAtPos` and `_StoreLog_Poly` parameters.
- `Activate`: if `PAN_CREATE` fails, `PAN_POST` is not called and `PAN_KILL` is skipped.

The unused debug helpers and unused `DynaValue` globals are kept on purpose, as tools of the original code. The Makefile turns off `-Wunused-function` and `-Wunused-variable` for them.
