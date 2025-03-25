# POSIX Makefile for GTest Project
# Converted from GTest.mak (Microsoft NMAKE file)

# Compiler settings
CC = gcc
CXX = g++
CFLAGS = -Wall -O2 -I../Src -I../include -I./DrvMenu -I./
LDFLAGS = 

# Build configurations
CONFIG ?= release

# Output directories
ifeq ($(CONFIG),debug)
    BUILDDIR = Debug
    CFLAGS += -g -DDEBUG
    LDFLAGS += 
else
    BUILDDIR = Release
    CFLAGS += -DNDEBUG
    LDFLAGS += -O2
endif

# Source directories
GAME_SRCS = \
	Game/_Bot.c \
	Game/Attacker.c \
	Game/Bot.c \
	Game/GenVSI.c \
	Game/GMain.c \
	Game/Items.c \
	Game/Level.c \
	Game/PathPt.c \
	Game/Track.c \
	Game/Weapons.c

FX_SRCS = FX/Corona.c FX/DynLight.c FX/Electric.c FX/FX.c FX/ModelCtl.c

PROCEDURAL_SRCS = \
	Procedurals/BumpMap.c \
	Procedurals/ElectricFX.c \
	Procedurals/Fire.c \
	Procedurals/GeBMUtil.c \
	Procedurals/Particles.c \
	Procedurals/Plasma.c \
	Procedurals/ProcEng.c \
	Procedurals/ProcEng_Tables.c \
	Procedurals/ProcUtil.c \
	Procedurals/SetFormat.c \
	Procedurals/Smoke.c \
	Procedurals/Water.c

DRVMENU_SRCS = DrvMenu/DrvList.c

ADDITIONAL_SRCS = \
	AutoSelect.c \
	Buffer.c \
	CD.c \
	Client.c \
	Console.c \
	GameMgr.c \
	Genvs.c \
	Gmenu.c \
	Host.c \
	IPAddr.c \
	Menu.c \
	ModeList.c \
	NetMgr.c \
	Server.c \
	Text.c \
	VidMode.c

# Object files
OBJS = $(GAME_SRCS:%.c=$(BUILDDIR)/%.o) \
       $(FX_SRCS:%.c=$(BUILDDIR)/%.o) \
       $(PROCEDURAL_SRCS:%.c=$(BUILDDIR)/%.o) \
       $(DRVMENU_SRCS:%.c=$(BUILDDIR)/%.o) \
       $(ADDITIONAL_SRCS:%.c=$(BUILDDIR)/%.o)

# Target executable
TARGET = ../GTest

# Libraries
LIBS = -lm -lwinmm -lws2_32

# Default target
all: $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

# Compilation rule for all source files
$(BUILDDIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -rf Debug Release
	rm -f $(TARGET)

# Create build directories
$(BUILDDIR):
	mkdir -p $(BUILDDIR)

# Phony targets
.PHONY: all clean debug release

# Debug configuration target
debug:
	$(MAKE) CONFIG=debug

# Release configuration target
release:
	$(MAKE) CONFIG=release
