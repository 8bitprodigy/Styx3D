# Genesis POSIX Makefile
# Converted from Genesis.mak

# Configuration variables
CC = gcc-14
AR = ar
RANLIB = ranlib

# Default configuration
CFG ?= debug

# Directories
TARGETDIR = ../../lib
OUTDIR_RELEASE = ./Release
OUTDIR_DEBUG = ./Debug
OUTDIR = $(OUTDIR_$(shell echo $(CFG) | tr '[:lower:]' '[:upper:]'))

# Compiler flags
CFLAGS_COMMON = -Wall -I../SDK/DXSDK/Include -I../Source \
	-I./World -I./Engine -I./Engine/Drivers -I./Actor -I./BSP \
	-I./Math -I./Entities -I./Support -I./Physics -I./VFile \
	-I./Bitmap -I./Bitmap/Compression -I../../include \
	-lSDL2 -lSDL2_ttf 

CFLAGS_RELEASE = $(CFLAGS_COMMON) -O2 -DNDEBUG
CFLAGS_DEBUG = $(CFLAGS_COMMON) -g -Og -D_DEBUG 

CFLAGS = $(CFLAGS_$(shell echo $(CFG) | tr '[:lower:]' '[:upper:]'))

# Output library
LIB_NAME = Genesis
LIB = $(TARGETDIR)/lib$(LIB_NAME).a

# Source files by directory
ACTOR_SRCS = \
	Actor/Actor.c \
	Actor/Body.c \
	Actor/BodyInst.c \
	Actor/Motion.c \
	Actor/Path.c \
	Actor/Pose.c \
	Actor/Puppet.c \
	Actor/QKFrame.c \
	Actor/StrBlock.c \
	Actor/TKArray.c \
	Actor/TKEvents.c \
	Actor/VKFrame.c \
	Actor/XFArray.c

BITMAP_COMPRESSION_SRCS = \
	Bitmap/Compression/PalCreate.c \
	Bitmap/Compression/Palettize.c \
	Bitmap/Compression/PalOptimize.c \
	Bitmap/Compression/YUV.c

BITMAP_SRCS = \
	Bitmap/Bitmap.c \
	Bitmap/Bitmap_BlitData.c \
	Bitmap/Bitmap_Gamma.c \
	Bitmap/PixelFormat.c

ENGINE_SRCS = \
	Engine/BitmapList.c \
	Engine/Engine.c \
	Engine/FontBMP.c \
	Engine/System.c

ENTITIES_SRCS = \
	Entities/Entities.c

FONT_SRCS = \
	Font/Font.c \
	Font/WgClip.c

LOGO_SRCS = \
	Engine/Logo/A_Corona.c \
	Engine/Logo/A_Streak.c \
	Engine/Logo/Corona.c \
	Engine/Logo/Electric.c \
	Engine/Logo/Logo.c \
	Engine/Logo/LogoActor.c \
	Engine/Logo/Streak.c \
	Engine/Logo/WebURL.c

MATH_SRCS = \
	Math/Box.c \
	Math/crc32.c \
	Math/ExtBox.c \
	Math/Quatern.c \
	Math/Vec3D.c \
	Math/XForm3D.c

PHYSICS_SRCS = \
	Physics/Matrix33.c \
	Physics/PhysicsJoint.c \
	Physics/PhysicsObject.c \
	Physics/PhysicsSystem.c

ROOT_SRCS = \
	Camera.c \
	CSNetMgr.c \
	Ge.c \
	List.c \
	NetPlay.c \
	Sound.c \
	Sound3D.c \
	TClip.c \
	Timer.c \
	TSC.c

SUPPORT_SRCS = \
	Support/ErrorLog.c \
	Support/GEAssert.c \
	Support/Log.c \
	Support/MemPool.c \
	Support/RAM.c \
	Support/RAMDLL.c \
	Support/XPlatUtils.c 

VFILE_SRCS = \
	VFile/DirTree.c \
	VFile/FSDisk.c \
	VFile/FSMemory.c \
	VFile/FSVFS.c \
	VFile/VFile.c
#	VFile/FSOps.c \

WORLD_SRCS = \
	World/Fog.c \
	World/Frustum.c \
	World/GBSPFile.c \
	World/Light.c \
	World/Plane.c \
	World/Surface.c \
	World/Trace.c \
	World/User.c \
	World/Vis.c \
	World/WBitmap.c \
	World/World.c


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

$(OUTDIR)/%.o: Actor/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Bitmap/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Bitmap/Compression/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Debug/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Engine/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Engine/Logo/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Entities/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Font/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Math/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Physics/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: Support/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: VFile/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OUTDIR)/%.o: World/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean target
clean:
	rm -f $(OBJS) $(LIB)
	rm -rf $(OUTDIR_RELEASE) $(OUTDIR_DEBUG)

# Phony targets
.PHONY: all clean prepare
