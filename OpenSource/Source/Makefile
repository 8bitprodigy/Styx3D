# Genesis POSIX Makefile
# Converted from Genesis.mak

# Configuration variables
CC = gcc
AR = ar
RANLIB = ranlib

# Default configuration
CFG ?= debug

# Directories
OUTDIR_RELEASE = ./Release
OUTDIR_DEBUG = ./Debug
OUTDIR = $(OUTDIR_$(shell echo $(CFG) | tr '[:lower:]' '[:upper:]'))

# Compiler flags
CFLAGS_COMMON = -Wall -I../SDK/DXSDK/Include -I../Source \
	-I./World -I./Engine -I./Engine/Drivers -I./Actor -I./BSP \
	-I./Math -I./Entities -I./Support -I./Physics -I./VFile \
	-I./Bitmap -I./Bitmap/Compression -I../../include

CFLAGS_RELEASE = $(CFLAGS_COMMON) -O2 -DNDEBUG
CFLAGS_DEBUG = $(CFLAGS_COMMON) -g -O0 -D_DEBUG

CFLAGS = $(CFLAGS_$(shell echo $(CFG) | tr '[:lower:]' '[:upper:]'))

# Output library
LIB_NAME = Genesis
LIB = $(OUTDIR)/lib$(LIB_NAME).a

# Source files by directory
WORLD_SRCS = \
	World/Fog.c \
	World/Frustum.c \
	World/Gbspfile.c \
	World/Light.c \
	World/Plane.c \
	World/Surface.c \
	World/Trace.c \
	World/User.c \
	World/Vis.c \
	World/WBitmap.c \
	World/World.c

LOGO_SRCS = \
	Engine/Logo/A_CORONA.c \
	Engine/Logo/A_STREAK.c \
	Engine/Logo/CORONA.c \
	Engine/Logo/electric.c \
	Engine/Logo/logo.c \
	Engine/Logo/LogoActor.c \
	Engine/Logo/streak.c \
	Engine/Logo/WebUrl.c

ENGINE_SRCS = \
	Engine/BitmapList.c \
	Engine/engine.c \
	Engine/fontbmp.c \
	Engine/System.c

ACTOR_SRCS = \
	Actor/actor.c \
	Actor/body.c \
	Actor/bodyinst.c \
	Actor/motion.c \
	Actor/path.c \
	Actor/pose.c \
	Actor/puppet.c \
	Actor/QKFrame.c \
	Actor/strblock.c \
	Actor/tkarray.c \
	Actor/tkevents.c \
	Actor/vkframe.c \
	Actor/XFArray.c

MATH_SRCS = \
	Math/Box.c \
	Math/crc32.c \
	Math/ExtBox.c \
	Math/quatern.c \
	Math/Vec3d.c \
	Math/Xform3d.c

ENTITIES_SRCS = \
	Entities/Entities.c

SUPPORT_SRCS = \
	Support/Errorlog.c \
	Support/geAssert.c \
	Support/log.c \
	Support/mempool.c \
	Support/Ram.c \
	Support/ramdll.c

PHYSICS_SRCS = \
	Physics/matrix33.c \
	Physics/PhysicsJoint.c \
	Physics/PhysicsObject.c \
	Physics/PhysicsSystem.c

VFILE_SRCS = \
	VFile/dirtree.c \
	VFile/fsdos.c \
	VFile/Fsmemory.c \
	VFile/fsvfs.c \
	VFile/vfile.c

BITMAP_COMPRESSION_SRCS = \
	Bitmap/Compression/palcreate.c \
	Bitmap/Compression/palettize.c \
	Bitmap/Compression/paloptimize.c \
	Bitmap/Compression/yuv.c

BITMAP_SRCS = \
	Bitmap/bitmap.c \
	Bitmap/bitmap_blitdata.c \
	Bitmap/bitmap_gamma.c \
	Bitmap/pixelformat.c

FONT_SRCS = \
	Font/font.c \
	Font/wgClip.c

ROOT_SRCS = \
	Camera.c \
	CSNetMgr.c \
	Ge.c \
	list.c \
	Netplay.c \
	Sound.c \
	Sound3d.c \
	Tclip.c \
	timer.c \
	tsc.c

# All source files
SRCS = $(WORLD_SRCS) $(LOGO_SRCS) $(ENGINE_SRCS) $(ACTOR_SRCS) $(MATH_SRCS) \
	$(ENTITIES_SRCS) $(SUPPORT_SRCS) $(PHYSICS_SRCS) $(VFILE_SRCS) \
	$(BITMAP_COMPRESSION_SRCS) $(BITMAP_SRCS) $(FONT_SRCS) $(ROOT_SRCS)

# Object files
OBJS = $(patsubst %.c,$(OUTDIR)/%.o,$(notdir $(SRCS)))

# Main targets
all: prepare $(LIB)

$(LIB): $(OBJS)
	$(AR) cru $@ $(OBJS)
	$(RANLIB) $@

# Prepare output directory
prepare:
	@mkdir -p $(OUTDIR)

# Compilation rules
$(OUTDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: World/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Engine/Logo/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Engine/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Actor/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Math/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Entities/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Support/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Physics/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: VFile/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Bitmap/Compression/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Bitmap/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Font/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJS) $(LIB)
	rm -rf $(OUTDIR_RELEASE) $(OUTDIR_DEBUG)

# Phony targets
.PHONY: all clean prepare
