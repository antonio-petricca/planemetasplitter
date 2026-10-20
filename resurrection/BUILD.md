# Building Plane Meta Splitter

This document explains how the resurrected plugin is built, step by step, and how
to check the result.

## Overview

The Makefile can produce two things:

| Build         | Command      | Output             | What it is for |
|---------------|--------------|--------------------|----------------|
| Win32 plugin  | `make win32` | `dist/planespl.p`  | **The real deliverable.** A 32-bit x86 Windows DLL, cross-compiled on Linux with MinGW. LightWave plugins have the `.p` extension. |
| Linux library | `make`       | `planespl.so`      | **Compile check only.** It proves the code compiles and links with a second compiler. No host program loads it. |

`dist/planespl.p` is committed to the repository, so you don't need a toolchain
just to get the plugin.

## Prerequisites

On Ubuntu or Debian:

```sh
sudo apt-get install make unzip gcc gcc-mingw-w64-i686
```

| Package            | Used for                                     |
|--------------------|----------------------------------------------|
| `make`             | runs the Makefile                            |
| `unzip`            | unpacks the SDK (`../sdk/lwsdk50.zip`)       |
| `gcc`              | Linux build (compile check)                  |
| `gcc-mingw-w64-i686` | cross-compiler `i686-w64-mingw32-gcc` for the Win32 build |

Other Linux distributions work too, with the equivalent packages. On Windows you
can do the same inside WSL (use an Ubuntu distribution and the line above).

## What the Makefile does

Run everything from the `resurrection/` folder.

1. **Unpack the SDK.** `../sdk/lwsdk50.zip` (the original NewTek LightWave 5.0
   SDK) is unpacked into `build/sdk/`. The zip and its content are not changed.
2. **Create lowercase names.** The SDK files have uppercase names
   (`INCLUDE/SPLUG.H`, `SOURCE/SERV_W.C`), but `planespl.c` includes
   `<splug.h>` in lowercase, and Linux file names are case sensitive. So the
   Makefile adds lowercase symlinks in `build/include/` and `build/source/`
   that point to the originals. Steps 1 and 2 are marked by `build/.sdk-ready`.
3. **Compile the plugin.** `planespl.c` is compiled. It `#include`s `Debug.c`,
   so `Debug.c` is never compiled alone.
4. **Compile the SDK startup sources.** `serv_w.c` and `slib1.c` ... `slib4.c`
   come from the SDK. They provide the plugin entry point `_mod_descrip` and
   the `ServerDesc`, `Startup` and `Shutdown` glue that the host looks for.
   In the 1996 SDK these were bundled as `server.lib`.
5. **Link.** For Windows, the objects are linked into a DLL. The file
   `planespl.def` exports exactly one symbol, `_mod_descrip` (the SDK's
   `SERV.DEF` says the same, but names the DLL `pluginServer`; ours names it
   `planespl.p`, as the original binary does). The Win32 linker settings also
   follow the 1996 file: GUI subsystem, image base `0x10000000`, no ASLR/DEP flags.

Objects of the two builds live in different folders (`build/win32/` for Windows,
`build/source/` and `.` for Linux), so they never mix.

### Compiler macros

| Macro        | Build | Effect |
|--------------|-------|--------|
| `-D_WIN32 -D_X86_=1` | Win32 | `plug.h` selects the `XCALL_` calling convention for Windows, and `serv_w.c` sets `MOD_MACHINE` to `0x300` (x86). These are the macros the SDK's MSVC makefile used. |
| `-D_XGL`     | Linux | Selects the generic `XCALL_` in `plug.h` and the machine id in `serv_w.c` without needing Windows headers. |

Other flags: `-std=gnu89` (the code is 1996 C), `-Wall`, and `-Wno-unused-function
-Wno-unused-variable` because the original debug helpers and globals are kept on
purpose. The Win32 build adds `-O2`; the Linux build uses `-fPIC`; the DLL is
linked with `-static-libgcc` and `-s` (strip).

## Targets

| Target            | What it does |
|-------------------|--------------|
| `make` / `make all` | Builds the Linux `planespl.so` (compile check). |
| `make win32`      | Builds the Windows plugin `dist/planespl.p`. |
| `make syntax`     | Only parses `planespl.c` (`-fsyntax-only`), no output files. |
| `make clean`      | Removes `build/`, `*.o` and `*.so`. **Keeps `dist/`**, which is committed. |
| `make distclean`  | Does `clean` and also removes `dist/`. |

Both builds finish with no compiler warnings.

## Verifying the result

Check that the DLL is a 32-bit x86 PE file that exports exactly `_mod_descrip`:

```sh
i686-w64-mingw32-objdump -p dist/planespl.p
```

Relevant lines of the output:

```
dist/planespl.p:     file format pei-i386
Characteristics 0x230e
ImageBase		10000000
Subsystem		00000002	(Windows GUI)
DllCharacteristics	00000000
	DLL Name: KERNEL32.dll
	DLL Name: msvcrt.dll
Name 				00009032 planespl.p
[Ordinal/Name Pointer] Table
	[   0] _mod_descrip
```

Compare it with the 1996 original (`objdump -p ../plugin+docs/PLANESPL.P`): same
format `pei-i386`, same image base, same subsystem, and the same single export
`_mod_descrip` (with the leading underscore). Expected differences:

- `Characteristics` is `0x230e` instead of `0x210e`: the extra bit only says
  that debug symbols are stripped.
- The original imports only `KERNEL32.dll` because it carried its own C runtime
  (statically linked MSVC). The new DLL also imports `msvcrt.dll`, the C
  runtime that comes with Windows 98 and later (and NT 4). Plain Windows 95
  first release does not have it.
- The file is smaller (about 25 KB against 43 KB) and the code is of course
  different, since the compiler is different.

## The 1996 toolchain, for reference

The SDK shows how plugins were built then, with Microsoft Visual C++ 2.0 (32-bit):

```
cl -c -W3 -D_X86_=1 -D_WIN32 /Ox  planespl.c serv_w.c
link -dll -export:_mod_descrip -out:planespl.p planespl.obj serv_w.obj server.lib libc.lib kernel32.lib
```

| 1996 (MSVC)                        | Today (MinGW)                          |
|------------------------------------|----------------------------------------|
| `cl ... /Ox`                       | `i686-w64-mingw32-gcc ... -O2`         |
| `-D_X86_=1 -D_WIN32`               | same macros                            |
| `-export:_mod_descrip` / `SERV.DEF`| `planespl.def`                         |
| `server.lib` (slib1..4)             | the same SDK sources compiled directly (`slib1.c`..`slib4.c`) |
| `link -dll -out:planespl.p`        | `-shared -o dist/planespl.p`           |

## Installing in LightWave

The plugin was written for LightWave 5.x Modeler:

1. Copy `planespl.p` into the plugins folder of your LightWave installation.
2. Add it from Modeler's plug-in configuration, so that Modeler loads it.

For what the plugin does and how to use it, see `../plugin+docs/PlaneMetaSplitter.doc`
(the sample objects `Step1.lwo` ... `Step5.lwo` are in the same folder).

## Honest note: never tested in LightWave

No LightWave (or any other host that loads these plugins) is available in the
build environment. So `dist/planespl.p` has been checked only as far as a
binary can be checked from outside: it compiles without warnings, it is a
valid 32-bit PE DLL, and its export table matches the 1996 original. It has
**not** been loaded or run inside LightWave. If you try it, please report what
happens.
