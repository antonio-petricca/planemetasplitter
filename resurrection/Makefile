# Makefile -- builds the resurrected Plane Meta Splitter plugin on Linux.
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
CFLAGS    = -std=gnu89 -D_XGL -fPIC -Wall -I. -I$(INC_DIR)

# SDK startup/library sources, compiled from the lowercase copies
SDK_OBJS  = $(SRC_DIR)/serv_w.o $(SRC_DIR)/slib1.o $(SRC_DIR)/slib2.o \
            $(SRC_DIR)/slib3.o $(SRC_DIR)/slib4.o

.PHONY: all syntax clean
.DEFAULT_GOAL := all

all: planespl.so

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

clean:
	rm -rf $(BUILD) *.o *.so
