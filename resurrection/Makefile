# Makefile -- builds the resurrected Plane Meta Splitter plugin.
#
#   make        Linux .so, a compile check only (no host can load it)
#   make win32  32-bit Windows plugin dist/planespl.p, the real deliverable
#
#
# The original LightWave SDK uses uppercase file names (INCLUDE/SPLUG.H),
# while planespl.c includes <splug.h>. We unpack the SDK into build/ and
# create lowercase symlinks there, leaving the zip itself untouched.

SDK_ZIP   = ../sdk/lwsdk50.zip
BUILD     = build
SDK_DIR   = $(BUILD)/sdk
INC_DIR   = $(BUILD)/include
SRC_DIR   = $(BUILD)/source
STAMP     = $(BUILD)/.sdk-ready

CC        = gcc
# -D_XGL selects XCALL_ in plug.h and MOD_MACHINE in serv_w.c
# The unused debug helpers (Debug.c) and the unused DynaValue globals are tools
# of the original code and are intentionally kept, so their warnings are off.
CFLAGS    = -std=gnu89 -D_XGL -fPIC -Wall -Wno-unused-function -Wno-unused-variable -I. -I$(INC_DIR)

# SDK startup/library sources, compiled from the lowercase copies
SDK_OBJS  = $(SRC_DIR)/serv_w.o $(SRC_DIR)/slib1.o $(SRC_DIR)/slib2.o \
            $(SRC_DIR)/slib3.o $(SRC_DIR)/slib4.o

.PHONY: all win32 syntax clean distclean
.DEFAULT_GOAL := all

all: planespl.so

# ---------------------------------------------------------------------------
# Win32 build: cross-compiled with MinGW as a 32-bit x86 DLL (.p = plugin)
# ---------------------------------------------------------------------------

WIN_CC      = i686-w64-mingw32-gcc
# -D_WIN32 -D_X86_=1 are the same macros the SDK's MSVC makefile used.
# No -fPIC: Windows DLLs are relocated by the loader.
WIN_CFLAGS  = -std=gnu89 -D_WIN32 -D_X86_=1 -O2 -Wall -Wno-unused-function -Wno-unused-variable -I. -I$(INC_DIR)
WIN_OBJ_DIR = $(BUILD)/win32
WIN_OUT_DIR = dist
WIN_TARGET  = $(WIN_OUT_DIR)/planespl.p
# Export list: only _mod_descrip, like the SDK's SERV.DEF, but with the library
# name planespl.p (SERV.DEF would name the DLL pluginServer.dll)
WIN_DEF     = planespl.def
# Linker settings that match the 1996 binary: GUI subsystem, image base 0x10000000,
# and no ASLR/DEP flags (the old host knows nothing about them)
WIN_LDFLAGS = -shared -static-libgcc -s -Wl,--subsystem,windows -Wl,--image-base,0x10000000 \
              -Wl,--disable-dynamicbase -Wl,--disable-nxcompat

WIN_SDK_OBJS = $(WIN_OBJ_DIR)/serv_w.o $(WIN_OBJ_DIR)/slib1.o $(WIN_OBJ_DIR)/slib2.o \
               $(WIN_OBJ_DIR)/slib3.o $(WIN_OBJ_DIR)/slib4.o
WIN_OBJS     = $(WIN_OBJ_DIR)/planespl.o $(WIN_SDK_OBJS)

win32: $(WIN_TARGET)

$(WIN_OBJ_DIR):
	mkdir -p $@

# The SDK sources only exist after unpacking; this lets make know they come
# with the stamp (empty recipe), so the pattern rules below can match them.
$(SRC_DIR)/%.c: $(STAMP) ;
.PRECIOUS: $(SRC_DIR)/%.c

# Debug.c is #included by planespl.c, so it is a dependency but not compiled alone
$(WIN_OBJ_DIR)/planespl.o: planespl.c Debug.c lwpanel.h $(STAMP) | $(WIN_OBJ_DIR)
	$(WIN_CC) $(WIN_CFLAGS) -c planespl.c -o $@

$(WIN_OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(STAMP) | $(WIN_OBJ_DIR)
	$(WIN_CC) $(WIN_CFLAGS) -c $< -o $@

# -s strips symbols; the .def file makes _mod_descrip the only export
$(WIN_TARGET): $(WIN_OBJS) $(WIN_DEF)
	mkdir -p $(WIN_OUT_DIR)
	$(WIN_CC) $(WIN_LDFLAGS) -o $@ $(WIN_OBJS) $(WIN_DEF)

# ---------------------------------------------------------------------------
# Linux build (compile check)
# ---------------------------------------------------------------------------

# Unpack the SDK and create lowercase names for headers and sources
$(STAMP): $(SDK_ZIP)
	rm -rf $(BUILD)
	mkdir -p $(SDK_DIR) $(INC_DIR) $(SRC_DIR)
	unzip -q -o $(SDK_ZIP) -d $(SDK_DIR)
	for f in $(SDK_DIR)/INCLUDE/*.H; do \
	    n=$$(basename $$f | tr 'A-Z' 'a-z'); \
	    ln -s ../sdk/INCLUDE/$$(basename $$f) $(INC_DIR)/$$n; \
	done
	for f in $(SDK_DIR)/SOURCE/*.C; do \
	    n=$$(basename $$f | tr 'A-Z' 'a-z'); \
	    ln -s ../sdk/SOURCE/$$(basename $$f) $(SRC_DIR)/$$n; \
	done
	touch $@

# Just check that planespl.c parses
syntax: $(STAMP)
	$(CC) $(CFLAGS) -fsyntax-only planespl.c

# Debug.c is #included by planespl.c: it is not compiled on its own
planespl.o: planespl.c Debug.c lwpanel.h $(STAMP)
	$(CC) $(CFLAGS) -c planespl.c -o $@

$(SRC_DIR)/%.o: $(SRC_DIR)/%.c $(STAMP)
	$(CC) $(CFLAGS) -c $< -o $@

planespl.so: planespl.o $(SDK_OBJS)
	$(CC) -shared -o $@ planespl.o $(SDK_OBJS)

# dist/ holds the committed binary, so clean leaves it alone
clean:
	rm -rf $(BUILD) *.o *.so

distclean: clean
	rm -rf $(WIN_OUT_DIR)
