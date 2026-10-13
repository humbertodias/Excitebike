# Excitebike
#
#   make            build for the host operating system
#   make windows    Windows (MinGW; cross-compiler off Windows)
#   make linux      Linux
#   make macos      macOS (must be built on macOS)
#   make clean
#
# macOS and Linux use SDL2, SDL2_image and SDL2_mixer.
# Windows keeps the Win32 / XAudio2 / SOIL link.
#   make USE_SDL=1    Windows build that uses SDL instead of Win32
#
# Overrides:
#   make DEBUG=1
#   make windows CC=x86_64-w64-mingw32-gcc
#   make SOIL_LIB=-lSOIL

APP      := excitebike
BUILD    := build

GAME_SRCS := \
	src/game/background.c \
	src/game/collision.c \
	src/game/game.c \
	src/game/levelmgr.c \
	src/game/menu.c \
	src/game/object.c \
	src/game/objmgr.c \
	src/game/player.c \
	src/game/ramp.c \
	src/game/random.c \
	src/game/shape.c \
	src/game/sprite.c \
	src/game/surface.c \
	src/game/vector.c

FW_COMMON := \
	src/framework/application.c \
	src/framework/input.c

INCLUDES := \
	-Iinclude/game \
	-Iinclude/framework

SOIL_LIB ?= -lSOIL
PKG_CONFIG ?= pkg-config

ifeq ($(OS),Windows_NT)
  HOST_OS := windows
else
  UNAME_S := $(shell uname -s)
  ifeq ($(UNAME_S),Linux)
    HOST_OS := linux
  else ifeq ($(UNAME_S),Darwin)
    HOST_OS := macos
  else
    HOST_OS := unknown
  endif
endif

TARGET_OS ?= $(HOST_OS)

ifeq ($(USE_SDL),1)
  PLATFORM_SRCS := src/framework/framework_sdl.c src/framework/sound_sdl.c src/framework/soil_sdl.c
  SDL_CFLAGS := $(shell $(PKG_CONFIG) --cflags sdl2 SDL2_mixer SDL2_image 2>/dev/null)
  SDL_LIBS := $(shell $(PKG_CONFIG) --libs sdl2 SDL2_mixer SDL2_image 2>/dev/null)
  PLATFORM_CFLAGS := -DUSE_SDL $(SDL_CFLAGS)
  ifeq ($(TARGET_OS),windows)
    EXE := $(APP).exe
    ifeq ($(HOST_OS),windows)
      DEFAULT_CC := gcc
    else
      DEFAULT_CC := x86_64-w64-mingw32-gcc
    endif
    PLATFORM_LDFLAGS := -mwindows
    PLATFORM_LDLIBS := $(SDL_LIBS) -lopengl32 -lglu32
  else ifeq ($(TARGET_OS),macos)
    EXE := $(APP)
    DEFAULT_CC := clang
    PLATFORM_CFLAGS += -DGL_SILENCE_DEPRECATION -D_DARWIN_C_SOURCE
    PLATFORM_LDFLAGS := -Wl,-headerpad_max_install_names
    PLATFORM_LDLIBS := $(SDL_LIBS) -framework OpenGL
  else ifeq ($(TARGET_OS),linux)
    EXE := $(APP)
    DEFAULT_CC := gcc
    PLATFORM_CFLAGS += -D_GNU_SOURCE
    PLATFORM_LDFLAGS := -Wl,-rpath,'$$$$ORIGIN/libs'
    PLATFORM_LDLIBS := $(SDL_LIBS) -lGL -lGLU -lm
  else
    $(error Unsupported TARGET_OS '$(TARGET_OS)' for USE_SDL.)
  endif
else ifeq ($(TARGET_OS),windows)
  EXE := $(APP).exe
  ifeq ($(HOST_OS),windows)
    DEFAULT_CC := gcc
  else
    DEFAULT_CC := x86_64-w64-mingw32-gcc
  endif
  PLATFORM_SRCS := src/framework/framework.c src/framework/sound.c
  PLATFORM_CFLAGS := -DWIN32 -D_CRT_SECURE_NO_WARNINGS -D_USE_MATH_DEFINES
  PLATFORM_LDFLAGS := -mwindows
  PLATFORM_LDLIBS := -lopengl32 -lglu32 -lfreeglut $(SOIL_LIB) -lgdi32 -lwinmm -lole32 -luuid
else ifeq ($(TARGET_OS),linux)
  EXE := $(APP)
  DEFAULT_CC := gcc
  PLATFORM_SRCS := src/framework/framework_sdl.c src/framework/sound_sdl.c src/framework/soil_sdl.c
  SDL_CFLAGS := $(shell $(PKG_CONFIG) --cflags sdl2 SDL2_mixer SDL2_image gl glu 2>/dev/null)
  SDL_LIBS := $(shell $(PKG_CONFIG) --libs sdl2 SDL2_mixer SDL2_image gl glu 2>/dev/null)
  ifeq ($(strip $(SDL_LIBS)),)
    SDL_LIBS := -lGL -lGLU -lSDL2 -lSDL2_mixer -lSDL2_image
  endif
  PLATFORM_CFLAGS := -D_GNU_SOURCE $(SDL_CFLAGS)
  PLATFORM_LDFLAGS := -Wl,-rpath,'$$$$ORIGIN/libs'
  PLATFORM_LDLIBS := $(SDL_LIBS) -lm
else ifeq ($(TARGET_OS),macos)
  EXE := $(APP)
  DEFAULT_CC := clang
  PLATFORM_SRCS := src/framework/framework_sdl.c src/framework/sound_sdl.c src/framework/soil_sdl.c
  SDL_CFLAGS := $(shell $(PKG_CONFIG) --cflags sdl2 SDL2_mixer SDL2_image 2>/dev/null)
  SDL_LIBS := $(shell $(PKG_CONFIG) --libs sdl2 SDL2_mixer SDL2_image 2>/dev/null)
  PLATFORM_CFLAGS := -DGL_SILENCE_DEPRECATION -D_DARWIN_C_SOURCE $(SDL_CFLAGS)
  PLATFORM_LDFLAGS := -Wl,-headerpad_max_install_names
  PLATFORM_LDLIBS := $(SDL_LIBS) -framework OpenGL
else
  $(error Unsupported TARGET_OS '$(TARGET_OS)'. Use windows, linux, or macos.)
endif

SRCS := $(FW_COMMON) $(PLATFORM_SRCS) $(GAME_SRCS)

ifeq ($(origin CC),default)
  CC := $(DEFAULT_CC)
endif

CFLAGS  ?= -std=gnu11 -Wall -Wextra
CFLAGS  += $(INCLUDES) $(PLATFORM_CFLAGS) -MMD -MP
LDFLAGS += $(PLATFORM_LDFLAGS) -Llib
LDLIBS  += $(PLATFORM_LDLIBS)

ifeq ($(DEBUG),1)
  CFLAGS += -g -O0 -D_DEBUG
else
  CFLAGS += -O2 -DNDEBUG
endif

OUT_DIR := $(BUILD)/$(TARGET_OS)
OBJS := $(SRCS:%.c=$(OUT_DIR)/%.o)
DEPS := $(OBJS:.o=.d)
TARGET := $(OUT_DIR)/$(EXE)

.PHONY: all windows linux macos clean help

all: $(TARGET)

windows linux macos:
	@$(MAKE) --no-print-directory TARGET_OS=$@ all

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

$(OUT_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD)

help:
	@echo "Targets: all (host), windows, linux, macos, clean"
	@echo "Host: $(HOST_OS)    Target: $(TARGET_OS)    Compiler: $(CC)"

-include $(DEPS)
