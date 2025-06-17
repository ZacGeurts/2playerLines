# Copyright (c) 2025 Zachary Geurts
# MIT License

# Overview:
# Builds 'lines', a cross-platform 2-player game using SDL (1.2 or 2.0) and OpenGL (or OpenGL ES).
# Run 'make help' to create directories and see platforms.
#
# Usage:
# - make: Build for local platform (src/main.cpp).
# - make cross-<platform>: Cross-compile for <platform>.
# - make clean: Remove build artifacts.
# - make help: Show platforms and create directories.
#
# Requirements:
# - src/main.cpp (compatible with SDL1.2/SDL2 and OpenGL/OpenGL ES).
# - Optional: assets/{images,sounds,music,fonts,shaders}/*.
# - Cross-compilation: SDKs in ./sdks/<platform>/{include,lib,bin} or system PATH.
# - SDL and OpenGL libraries for each platform (specified in platform configs).
#
# Adding Platforms:
# - Add platform to PLATFORMS.
# - Add a 'define <platform>_CONFIG' block with fields.
# - Run 'make help' to create SDK directories.

.DEFAULT_GOAL := lines

# Detect host OS
HOST_OS := $(shell uname -s 2>/dev/null || echo Windows)
ifeq ($(HOST_OS),Windows)
  MKDIR = mkdir
  RM = del /Q
  RMDIR = rmdir /S /Q
  CP = copy
  ZIP = tar -a -c -f
else
  MKDIR = mkdir -p
  RM = rm -f
  RMDIR = rm -rf
  CP = cp
  ZIP = zip
endif

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = objects
ASSETS_DIR = assets
ASSETS_SUBDIRS = $(ASSETS_DIR)/images $(ASSETS_DIR)/sounds $(ASSETS_DIR)/music $(ASSETS_DIR)/fonts $(ASSETS_DIR)/shaders
SDK_DIR = sdks
ITCHIO_DIR = itchio
OUTPUT_DIR = bin

# Platforms (add new platforms here)
PLATFORMS := 3ds aarch64 amiga68k amigappc android armv6 armv7 armv8 djgpp dreamcast emscripten ios linux macos ouya ps3 ps4 psp steamdeck steamlink switch vita wii wiiu windows itchio

# Platform configurations
# Format: define <platform>_CONFIG
# Fields: COMPILER, TARGET, EXT, SDK, SDL, CFLAGS, LDFLAGS, ASSET_DEST
define 3ds_CONFIG
# 3DS - Nintendo 3DS (2011-2020), uses SDL2 and OpenGL ES
COMPILER = arm-none-eabi-g++
TARGET = lines
EXT = .3dsx
SDK = $(DEVKITPRO)/portlibs/3ds
SDL = 2
CFLAGS = -march=armv6k -mtune=mpcore -mfloat-abi=hard -I$(DEVKITPRO)/libctru/include
LDFLAGS = -L$(DEVKITPRO)/libctru/lib -lSDL2 -lGLESv1_CM -lctru
ASSET_DEST = $(OUTPUT_DIR)/3ds/$(TARGET)/
endef

define aarch64_CONFIG
# aarch64 - 64-bit ARM (e.g., Raspberry Pi 4), uses SDL2 and OpenGL
COMPILER = aarch64-linux-gnu-g++
TARGET = lines
EXT =
SDK = aarch64-linux-gnu
SDL = 2
CFLAGS =
LDFLAGS = -lSDL2 -lGL -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/aarch64/$(TARGET)/
endef

define amiga68k_CONFIG
# amiga68k - AmigaOS Motorola 68000 (1985-1994), uses SDL1.2 and Warp3D
COMPILER = m68k-amigaos-g++
TARGET = lines
EXT =
SDK = amiga/m68k
SDL = 1_2
CFLAGS =
LDFLAGS = -lSDL -lwarp3d
ASSET_DEST = $(OUTPUT_DIR)/amiga68k/$(TARGET)/
endef

define amigappc_CONFIG
# amigappc - AmigaOS 4.x PowerPC (2006-2011), uses SDL1.2 and Warp3D
COMPILER = ppc-amigaos-g++
TARGET = lines
EXT =
SDK = amiga/ppc
SDL = 1_2
CFLAGS =
LDFLAGS = -lSDL -lwarp3d
ASSET_DEST = $(OUTPUT_DIR)/amigappc/$(TARGET)/
endef

define android_CONFIG
# android - Android OS (2008-2025+), uses SDL2 and OpenGL ES
COMPILER = $(ANDROID_NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi21-clang++
TARGET = liblines
EXT = .so
SDK = android-ndk
SDL = 2
CFLAGS = -I$(ANDROID_NDK)/sysroot/usr/include
LDFLAGS = -lSDL2 -lGLESv1_CM -llog -shared
ASSET_DEST = $(OUTPUT_DIR)/android/assets/
endef

define armv6_CONFIG
# armv6 - 32-bit ARM (e.g., Raspberry Pi 1), uses SDL2 and OpenGL
COMPILER = arm-linux-gnueabi-g++
TARGET = lines
EXT =
SDK = arm-linux-gnueabi
SDL = 2
CFLAGS =
LDFLAGS = -lSDL2 -lgl -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/armv6/$(TARGET)/
endef

define armv7_CONFIG
# armv7 - 32-bit ARM (e.g., Raspberry Pi 2), uses SDL2 and OpenGL
COMPILER = arm-linux-gnueabihf-g++
TARGET = lines
EXT =
SDK = arm-linux-gnueabihf
SDL = 2
CFLAGS =
LDFLAGS = -lSDL2 -lGL -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/armv7/$(TARGET)/
endef

define armv8_CONFIG
# armv8 - 64-bit ARM (e.g., Raspberry Pi 3), uses SDL2 and OpenGL
COMPILER = aarch64-linux-gnu-g++
TARGET = lines
EXT =
SDK = aarch64-linux-gnu
SDL = 2
CFLAGS =
LDFLAGS = -lSDL2 -lGL -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/armv8/$(TARGET)/
endef

define djgpp_CONFIG
# djgpp - DOS environment (1989-2005), uses SDL1.2 and MiniGL
COMPILER = i586-pc-msdosdjgpp-g++
TARGET = lines
EXT = .exe
SDK = djgpp
SDL = 1_2
CFLAGS =
LDFLAGS = -lSDL -lminigl
ASSET_DEST = $(OUTPUT_DIR)/djgpp/
endef

define dreamcast_CONFIG
# dreamcast - Sega Dreamcast (1998-2001), uses SDL1.2 and OpenGL ES
COMPILER = sh-elf-g++
TARGET = lines
EXT = .elf
SDK = kallisti
SDL = 1_2
CFLAGS =
LDFLAGS = -lSDL -lgl -lkos
ASSET_DEST = $(OUTPUT_DIR)/dreamcast/$(TARGET)/
endef

define emscripten_CONFIG
# emscripten - WebAssembly for browsers (2011-2025+), uses SDL2 and WebGL
COMPILER = em++
TARGET = lines
EXT = .html
SDK = emscripten
SDL = 2
CFLAGS = -s USE_SDL=2
LDFLAGS = -s USE_SDL=2 -s FULL_ES2=1 -s EXPORTED_RUNTIME_METHODS=[ccall,cwrap] --preload-file $(ASSETS_DIR)/@/
ASSET_DEST = $(OUTPUT_DIR)/emscripten/
endef

# Platform configurations (replace only the relevant ones)
define ios_CONFIG
# ios - Apple iOS (2007-2025+), uses SDL2 and OpenGL ES
COMPILER = clang++  # Use xcrun in cross-ios rule to avoid parsing-time execution
TARGET = lines
EXT = .app
SDK = ios-sdk
SDL = 2
CFLAGS = -arch arm64
LDFLAGS = -framework SDL2 -framework OpenGLES -framework UIKit -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/ios/$(TARGET).app/
endef

define linux_CONFIG
# linux - Linux OS (1991-2025+), uses SDL2 and OpenGL
COMPILER = g++
TARGET = lines
EXT =
SDK = linux
SDL = 2
CFLAGS =
LDFLAGS = -lSDL2 -lGL -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/linux/$(TARGET)/
endef

define macos_CONFIG
# macos - Apple macOS (2001-2025+), uses SDL2 and OpenGL
COMPILER = clang++
TARGET = lines
EXT = .app
SDK = macos
SDL = 2
CFLAGS = -mmacosx-version-min=10.11
LDFLAGS = -framework SDL2 -framework OpenGL -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/macos/$(TARGET).app/Contents/Resources/
endef

define ouya_CONFIG
# ouya - Ouya Android microconsole (2013-2015), uses SDL2 and OpenGL ES
COMPILER = $(ANDROID_NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi21-clang++
TARGET = liblines
EXT = .so
SDK = ouya-sdk
SDL = 2
CFLAGS = -I$(ANDROID_NDK)/sysroot/usr/include
LDFLAGS = -lSDL2 -lGLESv1_CM -llog -shared
ASSET_DEST = $(OUTPUT_DIR)/ouya/assets/
endef

define ps3_CONFIG
# ps3 - Sony PlayStation 3 (2006-2017), uses SDL2 and OpenGL ES
COMPILER = ppu-g++
TARGET = lines
EXT = .elf
SDK = ps3sdk
SDL = 2
CFLAGS =
LDFLAGS = -lSDL2 -lGLESv1_CM -lps3 -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/ps3/$(TARGET)/
endef

define ps4_CONFIG
# ps4 - Sony PlayStation 4 (2013-2024), uses SDL2 and OpenGL
COMPILER = orbis-clang++
TARGET = lines
EXT = .elf
SDK = openorbis
SDL = 2
CFLAGS =
LDFLAGS = -lSDL2 -lSceVideoOut -lScePigletv2VSH -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/ps4/$(TARGET)/
endef

define psp_CONFIG
# psp - Sony PlayStation Portable (2004-2014), uses SDL1.2 and OpenGL ES
COMPILER = psp-g++
TARGET = lines
EXT = .elf
SDK = pspsdk
SDL = 1_2
CFLAGS =
LDFLAGS = -lSDL -lGLESv1_CM -lpspgl -lpspvram -lpspsdk
ASSET_DEST = $(OUTPUT_DIR)/psp/
endef

define steamdeck_CONFIG
# steamdeck - Valve Steam Deck (2022-2025+), uses SDL2 and OpenGL
COMPILER = g++
TARGET = lines
EXT =
SDK = steamdeck
SDL = 2
CFLAGS =
LDFLAGS = -lSDL2 -lGL -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/steamdeck/$(TARGET)/
endef

define steamlink_CONFIG
# steamlink - Valve Steam Link (2015-2018), uses SDL2 and OpenGL
COMPILER = arm-linux-gnueabihf-g++
TARGET = lines
EXT =
SDK = steam-link-sdk
SDL = 2
CFLAGS =
LDFLAGS = -lSDL2 -lGL -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/steamlink/$(TARGET)/
endef

define switch_CONFIG
# switch - Nintendo Switch (2017-2025+), uses SDL2 and OpenGL ES
COMPILER = aarch64-none-elf-g++
TARGET = lines
EXT = .nro
SDK = $(DEVKITPRO)/portlibs/switch
SDL = 2
CFLAGS = -I$(DEVKITPRO)/libnx/include
LDFLAGS = -L$(DEVKITPRO)/libnx/lib -lSDL2 -lGLESv2 -lnx -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/switch/$(TARGET)/
endef

define vita_CONFIG
# vita - Sony PlayStation Vita (2011-2019), uses SDL2 and OpenGL ES
COMPILER = arm-vita-eabi-g++
TARGET = lines
EXT = .velf
SDK = vitasdk
SDL = 2
CFLAGS =
LDFLAGS = -lSDL2 -lSceGxm -lSceDisplay -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/vita/$(TARGET)/
endef

define wii_CONFIG
# wii - Nintendo Wii (2006-2013), uses SDL1.2 and OpenGL ES
COMPILER = powerpc-eabi-g++
TARGET = lines
EXT = .dol
SDK = $(DEVKITPRO)/portlibs/wii
SDL = 1_2
CFLAGS = -I$(DEVKITPRO)/libogc/include
LDFLAGS = -L$(DEVKITPRO)/libogc/lib/wii -lSDL -lGLESv1_CM -lwiiuse -lwiimote -logc -lSDLmain
ASSET_DEST = $(OUTPUT_DIR)/wii/$(TARGET)/
endef

define wiiu_CONFIG
# wiiu - Nintendo Wii U (2012-2017), uses SDL2 and OpenGL
COMPILER = powerpc-eabi-g++
TARGET = lines
EXT = .rpx
SDK = $(DEVKITPRO)/portlibs/wiiu
SDL = 2
CFLAGS = -I$(DEVKITPRO)/wut/include
LDFLAGS = -L$(DEVKITPRO)/wut/lib -lSDL2 -lgx2 -lwiiu -lSDL2main
ASSET_DEST = $(OUTPUT_DIR)/wiiu/$(TARGET)/
endef

define windows_CONFIG
# windows - Microsoft Windows (1985-2025+), uses SDL2 and OpenGL
COMPILER = x86_64-w64-mingw32-g++
TARGET = lines
EXT = .exe
SDK = mingw32
SDL = 2
CFLAGS =
LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lopengl32
ASSET_DEST = $(OUTPUT_DIR)/windows/$(TARGET)/
endef

define itchio_CONFIG
# itchio - itch.io web platform (2013-2025+), packages Emscripten build
COMPILER = em++
TARGET = lines
EXT = .zip
SDK = emscripten
SDL = 2
CFLAGS = -s USE_SDL=2
LDFLAGS = -s USE_SDL=2 -s FULL_ES2=1 -s EXPORTED_RUNTIME_METHODS=[ccall,cwrap] --preload-file $(ASSETS_DIR)/@/
ASSET_DEST = $(ITCHIO_DIR)/
endef

# Local build defaults (host-dependent)
ifeq ($(HOST_OS),Linux)
  CC = g++
  CFLAGS = -Wall -O2 -I$(INCLUDE_DIR) -DUSE_SDL2
  LDFLAGS = -lSDL2 -lGL -lSDL2main
  TARGET_EXT =
  ASSET_DEST = $(OUTPUT_DIR)/linux/$(TARGET)/
else ifeq ($(HOST_OS),Darwin)
  CC = clang++
  CFLAGS = -Wall -O2 -I$(INCLUDE_DIR) -DUSE_SDL2 -mmacosx-version-min=10.11
  LDFLAGS = -framework SDL2 -framework OpenGL -lSDL2main
  TARGET_EXT = .app
  ASSET_DEST = $(OUTPUT_DIR)/macos/$(TARGET).app/Contents/Resources/
else
  CC = x86_64-w64-mingw32-g++
  CFLAGS = -Wall -O2 -I$(INCLUDE_DIR) -DUSE_SDL2
  LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lopengl32
  TARGET_EXT = .exe
  ASSET_DEST = $(OUTPUT_DIR)/windows/$(TARGET)/
endif
TARGET = lines

# Source and object files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
ASSETS = $(wildcard $(ASSETS_DIR)/images/* $(ASSETS_DIR)/sounds/* $(ASSETS_DIR)/music/* $(ASSETS_DIR)/fonts/* $(ASSETS_DIR)/shaders/*)

# Parse platform configurations
$(foreach platform,$(PLATFORMS),\
  $(eval $(platform)_CC := $(word 2,$($(platform)_CONFIG:COMPILER=%)))\
  $(eval $(platform)_TARGET := $(word 2,$($(platform)_CONFIG:TARGET=%)))\
  $(eval $(platform)_EXT := $(word 2,$($(platform)_CONFIG:EXT=%)))\
  $(eval $(platform)_SDK := $(word 2,$($(platform)_CONFIG:SDK=%)))\
  $(eval $(platform)_SDL := $(word 2,$($(platform)_CONFIG:SDL=%)))\
  $(eval $(platform)_CFLAGS := $(wordlist 2,999,$($(platform)_CONFIG:CFLAGS=%)))\
  $(eval $(platform)_LDFLAGS := $(wordlist 2,999,$($(platform)_CONFIG:LDFLAGS=%)))\
  $(eval $(platform)_ASSET_DEST := $(wordlist 2,999,$($(platform)_CONFIG:ASSET_DEST=%)))\
  $(eval $(platform)_SDK_PATH := $(SDK_DIR)/$($(platform)_SDK)))

# Debug target for parsing issues
debug-config:
	@$(foreach platform,$(PLATFORMS),\
		echo "$(platform):"; \
		echo "  CC: $($(platform)_CC)"; \
		echo "  TARGET: $($(platform)_TARGET)"; \
		echo "  EXT: $($(platform)_EXT)"; \
		echo "  SDK: $($(platform)_SDK)"; \
		echo "  SDL: $($(platform)_SDL)"; \
		echo "  CFLAGS: $($(platform)_CFLAGS)"; \
		echo "  LDFLAGS: $($(platform)_LDFLAGS)"; \
		echo "  ASSET_DEST: $($(platform)_ASSET_DEST)"; \
		echo "";)

# Directory creation
$(SRC_DIR) $(INCLUDE_DIR) $(BUILD_DIR) $(SDK_DIR) $(ITCHIO_DIR) $(OUTPUT_DIR):
	@echo "[CREATING] $@" && \
	$(MKDIR) $@ || { echo "[ERROR] Failed to create directory $@"; exit 1; }

$(ASSETS_DIR): $(ASSETS_SUBDIRS)
	@echo "[CREATING] $@" && \
	$(MKDIR) $@ || { echo "[ERROR] Failed to create directory $@"; exit 1; }

$(ASSETS_SUBDIRS):
	@echo "[CREATING] $@" && \
	$(MKDIR) $@ || { echo "[ERROR] Failed to create directory $@"; exit 1; }

# SDK directories for each platform
define SDK_DIR_RULE
sdk-dirs-$(platform): $(SDK_DIR)
	@echo "[CREATING] SDK directories for $(platform)..."
	@if test -n "$($(platform)_SDK)" && test -n "$($(platform)_SDL)"; then \
		$(MKDIR) "$(SDK_DIR)/$($(platform)_SDK)/include/SDL$($(platform)_SDL)" && \
		$(MKDIR) "$(SDK_DIR)/$($(platform)_SDK)/lib" && \
		$(MKDIR) "$(SDK_DIR)/$($(platform)_SDK)/bin" && \
		echo "[SUCCESS] Created SDK directories for $(platform)" || \
		{ echo "[ERROR] Failed to create SDK directories for $(platform)"; exit 1; }; \
	else \
		echo "[ERROR] Skipping $(platform): SDK or SDL undefined. Run 'make debug-config'."; \
		exit 1; \
	fi
endef
$(foreach platform,$(PLATFORMS),$(eval $(SDK_DIR_RULE)))

sdk-dirs: $(addprefix sdk-dirs-,$(PLATFORMS))
	@echo "[SUCCESS] All SDK directories created"

# Help directories
help-dirs: $(SRC_DIR) $(INCLUDE_DIR) $(ASSETS_DIR) $(SDK_DIR) $(ITCHIO_DIR) $(OUTPUT_DIR) sdk-dirs
	@echo "[SUCCESS] All directories created"

# Local build
$(TARGET)$(TARGET_EXT): $(OBJECTS) $(ASSETS) | $(BUILD_DIR) $(OUTPUT_DIR)
	@echo "[LINKING] $@..." && \
	$(CC) $(OBJECTS) -o $(OUTPUT_DIR)/$@ $(LDFLAGS) && \
	if [ -n "$(ASSETS)" ]; then \
		$(MKDIR) "$(ASSET_DEST)" && \
		$(CP) $(ASSETS) "$(ASSET_DEST)" && \
		echo "[SUCCESS] Copied assets to $(ASSET_DEST)"; \
	fi && \
	if [ "$(TARGET_EXT)" = ".app" ]; then \
		$(MKDIR) $(OUTPUT_DIR)/$@/Contents/MacOS && \
		$(CP) $(OUTPUT_DIR)/$@ $(OUTPUT_DIR)/$@/Contents/MacOS/$(TARGET) && \
		echo "[SUCCESS] Created macOS .app bundle"; \
	fi && \
	echo "[SUCCESS] Built $(OUTPUT_DIR)/$@" || \
	{ echo "[ERROR] Build failed. Ensure SDL2 and OpenGL are installed."; exit 1; }

# Compile sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(SRC_DIR) $(INCLUDE_DIR) $(BUILD_DIR)
	@echo "[COMPILING] $<..." && \
	$(CC) $(CFLAGS) -c $< -o $@ && \
	echo "[SUCCESS] Compiled $<" || \
	{ echo "[ERROR] Compilation failed for $<. Ensure src/main.cpp exists and SDL/OpenGL headers are available."; exit 1; }

# Modified CROSS_RULE for ios to handle xcrun
define CROSS_RULE
cross-$(platform): $(SOURCES) $(ASSETS) | $(BUILD_DIR) $(OUTPUT_DIR)
	@echo "[SYSTEM] Building for $(platform)..."
	@if [ -z "$($(platform)_CC)" ]; then \
		echo "[ERROR] Compiler undefined for $(platform). Run 'make debug-config'."; exit 1; \
	fi
	@if [ -z "$($(platform)_SDK)" ] || [ -z "$($(platform)_SDL)" ]; then \
		echo "[ERROR] SDK or SDL undefined for $(platform). Run 'make debug-config'."; exit 1; \
	fi
	@if [ "$(platform)" = "ios" ]; then \
		if command -v xcrun >/dev/null 2>&1; then \
			$(platform)_CC := xcrun -sdk iphoneos $$($(platform)_CC); \
			$(platform)_CFLAGS := $$($(platform)_CFLAGS) -isysroot $$(xcrun -sdk iphoneos --show-sdk-path); \
		else \
			echo "[ERROR] xcrun not found. Install Xcode for iOS builds."; exit 1; \
		fi \
	fi
	@if command -v $$($(platform)_CC) >/dev/null 2>&1; then \
		$(MKDIR) "$$($(platform)_ASSET_DEST)"; \
		for src in $(SOURCES); do \
			echo "[COMPILING] $$$$src for $(platform)..."; \
			$$($(platform)_CC) -DUSE_SDL$$($(platform)_SDL) -I$$($(platform)_SDK_PATH)/include/SDL$$($(platform)_SDL) \
			$(CFLAGS) $$($(platform)_CFLAGS) -c $$src -o $(BUILD_DIR)/$$(basename $$src .cpp).o && \
			echo "[SUCCESS] Compiled $$$$src" || \
			{ echo "[ERROR] Compilation failed for $$$$src. Ensure SDK is in $$($(platform)_SDK_PATH)."; exit 1; }; \
		done; \
		echo "[LINKING] $$($(platform)_TARGET)$$($(platform)_EXT)..."; \
		$$($(platform)_CC) $(OBJECTS) -o $(OUTPUT_DIR)/$$($(platform)_TARGET)$$($(platform)_EXT) \
		-L$$($(platform)_SDK_PATH)/lib $$($(platform)_LDFLAGS) && \
		echo "[SUCCESS] Linked $(OUTPUT_DIR)/$$($(platform)_TARGET)$$($(platform)_EXT)" || \
		{ echo "[ERROR] Linking failed. Ensure SDK is in $$($(platform)_SDK_PATH)."; exit 1; }; \
		if [ -n "$(ASSETS)" ]; then \
			$(MKDIR) "$$($(platform)_ASSET_DEST)" && \
			$(CP) $(ASSETS) "$$($(platform)_ASSET_DEST)" && \
			echo "[SUCCESS] Copied assets to $$($(platform)_ASSET_DEST)"; \
		fi; \
		if [ "$(platform)" = "itchio" ]; then \
			if command -v $(ZIP) >/dev/null 2>&1; then \
				echo "[PACKAGING] Itch.io build..."; \
				cd $(OUTPUT_DIR)/emscripten && \
				$(ZIP) ../../$(ITCHIO_DIR)/$$($(platform)_TARGET)$$($(platform)_EXT) $$($(platform)_TARGET).html \
				$$($(platform)_TARGET).js $$($(platform)_TARGET).wasm $$($(platform)_TARGET).data && \
				echo "[SUCCESS] Created $(ITCHIO_DIR)/$$($(platform)_TARGET)$$($(platform)_EXT)" || \
				{ echo "[ERROR] Failed to create itch.io zip."; exit 1; }; \
			else \
				echo "[ERROR] zip/tar not found. Install zip for itch.io builds."; exit 1; \
			fi; \
		elif [ "$(platform)" = "ios" ] || [ "$(platform)" = "macos" ]; then \
			$(MKDIR) $(OUTPUT_DIR)/$$($(platform)_TARGET)$$($(platform)_EXT)/Contents/MacOS && \
			$(CP) $(OUTPUT_DIR)/$$($(platform)_TARGET)$$($(platform)_EXT) \
			$(OUTPUT_DIR)/$$($(platform)_TARGET)$$($(platform)_EXT)/Contents/MacOS/$$($(platform)_TARGET) && \
			echo "[SUCCESS] Created $$($(platform)_EXT) bundle"; \
		fi; \
	else \
		echo "[ERROR] Compiler $$($(platform)_CC) not found. Place in $$($(platform)_SDK_PATH)/bin or PATH."; exit 1; \
	fi
endef
$(foreach platform,$(PLATFORMS),$(eval $(CROSS_RULE)))

# Update help to mention xcrun requirement
help: help-dirs
	@echo "Builds 'lines', a 2-player game using SDL (1.2 or 2.0) and OpenGL (or OpenGL ES)."
	@echo ""
	@echo "Usage:"
	@echo "  make - Build for local platform ($(HOST_OS))"
	@echo "  make cross-<platform> - Cross-compile (e.g., make cross-wii)"
	@echo "  make clean - Remove build artifacts"
	@echo "  make help - Show this help and create directories"
	@echo ""
	@echo "Setup:"
	@echo "  - Local build: Install SDL2, OpenGL, and a C++ compiler"
	@echo "  - Cross-compilation: Place SDKs in ./sdks/<platform>/{include,lib,bin} or system PATH"
	@echo "  - iOS builds require Xcode and xcrun (macOS only)"
	@echo "  - Source: $(SRC_DIR)/main.cpp"
	@echo "  - Assets (optional): $(ASSETS_DIR)/{images,sounds,music,fonts,shaders}/*"
	@echo ""
	@echo "Add a platform:"
	@echo "  1. Add platform to PLATFORMS"
	@echo "  2. Add 'define <platform>_CONFIG' with COMPILER, TARGET, EXT, SDK, SDL, CFLAGS, LDFLAGS, ASSET_DEST"
	@echo "  3. Run 'make help' to create directories"
	@echo ""
	@echo "Platforms:"
	@echo "$(PLATFORMS)"
	@echo ""
	@echo "Notes:"
	@echo "  - Simply run make to build the lines program for your computer."
	@echo "  - Or, put your files in the created sdk folders now and then run make cross-<platform>"
	@echo "  - Outputs are in $(OUTPUT_DIR)/<platform>/"
	@echo "  - If SDK issues occur, run 'make debug-config'."
	@echo "  - For web builds (emscripten), test with 'python3 -m http.server'"
	@echo "  - Itch.io builds create $(ITCHIO_DIR)/lines.zip"

# Clean
clean:
	@echo "[CLEANING] Build artifacts..." && \
	$(RMDIR) $(BUILD_DIR) $(ITCHIO_DIR) $(OUTPUT_DIR) && \
	$(RM) *.o *.exe *.app *.so *.nro *.dol *.elf *.velf *.rpx *.3dsx *.html *.js *.wasm *.zip && \
	echo "[SUCCESS] Clean completed" || \
	{ echo "[ERROR] Failed to clean some artifacts."; exit 1; }

# Phony targets
.PHONY: help clean sdk-dirs debug-config $(addprefix cross-,$(PLATFORMS))

# Prevent object deletion
.PRECIOUS: $(OBJECTS)