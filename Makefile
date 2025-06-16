# Copyright (c) 2025 Zachary Geurts
# MIT License

# Overview:
# Builds 'lines' for multiple platforms with SDL (SDL2/SDL1.2) and OpenGL.
# Targets 320x200 or closest 4:3 resolution. Creates SDK and other directories.

# Usage:
# - `make`: Build for local platform.
# - `make cross-<platform>`: Cross-compile (e.g., `make cross-wii`).
# - `make cross-emscripten`: Build for web (also for itch.io), test with `python3 -m http.server`.
# - `make cross-itchio`: Package emscripten build for itch.io upload.
# - `make clean`: Remove artifacts.
# - `make help`: Show help and create directories (assets, objects, include, sdks).

# Requirements:
# - SDL2/SDL1.2 and OpenGL libraries.
# - Toolchains in system PATH or `sdks/`.
# - src/main.cpp provided.

.DEFAULT_GOAL := lines

# Compiler and flags
CC = g++
CFLAGS = -Wall -O2 -Iinclude
LDFLAGS = -lSDL2 -lGL -lSDL2main

# Directories
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = objects
ASSETS_DIR = assets
SDK_DIR = sdks
ITCHIO_DIR = itchio

# Platforms
PLATFORMS := 3ds aarch64 amiga68k amigappc android armv6 armv7 armv8 djgpp dreamcast emscripten ios linux macos ouya ps3 ps4 psp steamdeck steamlink switch vita wii wiiu windows itchio

# Platform configurations: platform = compiler target ext sdk_path sdl_version
define PLATFORM_CONFIG
3ds = arm-none-eabi-g++ lines.3dsx .3dsx devkitpro/3ds SDL2
aarch64 = aarch64-linux-gnu-g++ lines "" aarch64-linux-gnu SDL2
amiga68k = m68k-amigaos-g++ lines "" amiga/m68k SDL1_2
amigappc = ppc-amigaos-g++ lines "" amiga/ppc SDL1_2
android = $(ANDROID_NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi21-clang++ lines.so .so android-ndk SDL2
armv6 = arm-linux-gnueabi-g++ lines "" arm-linux-gnueabi SDL2
armv7 = arm-linux-gnueabihf-g++ lines "" arm-linux-gnueabihf SDL2
armv8 = aarch64-linux-gnu-g++ lines "" aarch64-linux-gnu SDL2
djgpp = i586-pc-msdosdjgpp-g++ lines.exe .exe djgpp SDL1_2
dreamcast = sh-elf-g++ lines.elf .elf kallisti SDL1_2
emscripten = em++ lines.html .html emscripten SDL2
ios = xcrun -sdk iphoneos clang++ lines.app .app ios-sdk SDL2
linux = g++ lines "" linux SDL2
macos = clang++ lines.app .app macos SDL2
ouya = $(ANDROID_NDK)/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi21-clang++ lines.so .so ouya-sdk SDL2
ps3 = ppu-g++ lines.elf .elf ps3sdk SDL2
ps4 = orbis-clang++ lines.elf .elf openorbis SDL2
psp = psp-g++ lines.elf .elf pspsdk SDL1_2
steamdeck = g++ lines "" steam-deck SDL2
steamlink = arm-linux-gnueabihf-g++ lines "" steam-link-sdk SDL2
switch = aarch64-none-elf-g++ lines.nro .nro devkitpro/switch SDL2
vita = arm-vita-eabi-g++ lines.velf .velf vitasdk SDL2
wii = powerpc-eabi-g++ lines.dol .dol devkitpro/wii SDL1_2
wiiu = powerpc-eabi-g++ lines.rpx .rpx devkitpro/wiiu SDL2
windows = x86_64-w64-mingw32-g++ lines.exe .exe mingw32 SDL2
itchio = em++ lines.html .zip emscripten SDL2
endef

# Parse platform configurations
$(foreach platform,$(PLATFORMS),$(eval $(platform)_CC := $(word 2,$(subst =, ,$(filter $(platform) =%,$(PLATFORM_CONFIG))))))
$(foreach platform,$(PLATFORMS),$(eval $(platform)_TARGET := $(word 3,$(subst =, ,$(filter $(platform) =%,$(PLATFORM_CONFIG))))))
$(foreach platform,$(PLATFORMS),$(eval $(platform)_EXT := $(word 4,$(subst =, ,$(filter $(platform) =%,$(PLATFORM_CONFIG))))))
$(foreach platform,$(PLATFORMS),$(eval $(platform)_SDK := $(word 5,$(subst =, ,$(filter $(platform) =%,$(PLATFORM_CONFIG))))))
$(foreach platform,$(PLATFORMS),$(eval $(platform)_SDL := $(lastword $(subst =, ,$(filter $(platform) =%,$(PLATFORM_CONFIG))))))
$(foreach platform,$(PLATFORMS),$(eval $(platform)_LDFLAGS := -lSDL$($(platform)_SDL) $(if $(filter $(platform),amiga68k amigappc),-lwarp3d,-$(if $(filter $(platform),djgpp),minigl,-$(if $(filter $(platform),dreamcast),GLdc -lkos,-$(if $(filter $(platform),android ouya),GLESv1_CM -llog,-$(if $(filter $(platform),ios),-framework SDL2 -framework OpenGLES -framework UIKit -lSDL2main,-$(if $(filter $(platform),macos),-framework SDL2 -framework OpenGL -lSDL2main,-$(if $(filter $(platform),ps3),SDL2 -lGLESv1_CM -lps3 -lSDL2main,-$(if $(filter $(platform),ps4),SDL2 -lSceVideoOut -lScePigletv2VSH -lSDL2main,-$(if $(filter $(platform),psp),SDL -lGLESv1_CM -lpspgl -lpspvram -lpspsdk,-$(if $(filter $(platform),switch),SDL2 -lGLESv2 -lnx -lSDL2main,-$(if $(filter $(platform),vita),SDL2 -lSceGxm -lSceDisplay -lSDL2main,-$(if $(filter $(platform),wii),SDL -lGLESv1_CM -lwiiuse -lwiimote -logc -lSDLmain,-$(if $(filter $(platform),wiiu),SDL2 -lgx2 -lwiiu -lSDL2main,-$(if $(filter $(platform),windows),mingw32 -lSDL2main -lSDL2 -lopengl32,-$(if $(filter $(platform),emscripten itchio),-s USE_SDL=2 -s FULL_ES2=1 -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap'] --preload-file $(ASSETS_DIR)/@/,--$(if $(filter $(platform),linux steamdeck aarch64 armv6 armv7 armv8 steamlink),SDL2 -lGL -lSDL2main,))))))))))))))))))

# Source and object files
SOURCES = $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(SOURCES))
TARGET = lines

# Detect local platform
UNAME_S := $(shell uname -s 2>/dev/null || echo Unknown)
OS := $(OS)
LOCAL_PLATFORM := $(or $(filter Windows_NT,$(OS)),$(UNAME_S))
ifeq ($(LOCAL_PLATFORM),Darwin)
LOCAL_PLATFORM := macos
else ifeq ($(LOCAL_PLATFORM),Linux)
LOCAL_PLATFORM := linux
else ifeq ($(LOCAL_PLATFORM),Windows_NT)
LOCAL_PLATFORM := windows
endif

# Directory creation
$(INCLUDE_DIR) $(BUILD_DIR) $(ASSETS_DIR) $(SDK_DIR) $(ITCHIO_DIR):
	@mkdir -p $@ || true

# SDK directories
sdk-dirs: $(SDK_DIR)
	@echo "Creating SDK directories..."
	@for platform in $(PLATFORMS); do \
		eval sdk_path=\$$$platform"_SDK"; \
		eval sdl_ver=\$$$platform"_SDL"; \
		mkdir -p $(SDK_DIR)/$$sdk_path/include/SDL$$sdl_ver $(SDK_DIR)/$$sdk_path/lib || true; \
	done

# Create all directories (used by help)
init-dirs: $(INCLUDE_DIR) $(BUILD_DIR) $(ASSETS_DIR) $(ITCHIO_DIR) sdk-dirs
	@echo "All directories created: $(INCLUDE_DIR), $(BUILD_DIR), $(ASSETS_DIR), $(ITCHIO_DIR), $(SDK_DIR)/<platform>"

# Local build
$(TARGET): $(OBJECTS) | init-dirs
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

# Compile sources
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp | $(INCLUDE_DIR) $(BUILD_DIR)
	$(CC) $(CFLAGS) -DUSE_SDL$($(LOCAL_PLATFORM)_SDL) -I$(SDK_DIR)/$($(LOCAL_PLATFORM)_SDK)/include/SDL$($(LOCAL_PLATFORM)_SDL) -c $< -o $@ || { echo "Error: Compilation failed for $<."; exit 1; }

# Cross-compilation
define CROSS_RULE
cross-$(platform): init-dirs | $(BUILD_DIR)
	@echo "Building for $(platform)..."
	@which $$($(platform)_CC) >/dev/null 2>&1 || { echo "Build skipped for $(platform): $$($(platform)_CC) not found."; exit 0; }
	$$($(platform)_CC) $(CFLAGS) -I$(INCLUDE_DIR) -I$(SDK_DIR)/$$($(platform)_SDK)/include/SDL$$($(platform)_SDL) -DUSE_SDL$$($(platform)_SDL) -c $(SRC_DIR)/main.cpp -o $(BUILD_DIR)/main.o
	$$($(platform)_CC) $(BUILD_DIR)/main.o -o $$($(platform)_TARGET)$$($(platform)_EXT) -L$(SDK_DIR)/$$($(platform)_SDK)/lib $$($(platform)_LDFLAGS) || { echo "Error: Build failed for $(platform)."; exit 1; }
	$(if $(filter $(platform),itchio),zip -r $$($(platform)_TARGET)$$($(platform)_EXT) $$($(platform)_TARGET).html $$($(platform)_TARGET).js $$($(platform)_TARGET).wasm $$($(platform)_TARGET).data $(ASSETS_DIR) -d $(ITCHIO_DIR),)
endef
$(foreach platform,$(PLATFORMS),$(eval $(CROSS_RULE)))

# Help
help: init-dirs
	@echo "=== Lines Program Makefile ==="
	@echo "Builds 'lines' for 320x200 or closest 4:3 resolution."
	@echo "Usage:"
	@echo "  make: Build for local platform ($(LOCAL_PLATFORM))."
	@echo "  make cross-<platform>: Cross-compile (e.g., make cross-wii)."
	@echo "  make cross-emscripten: Build for web (also for itch.io), test with 'python3 -m http.server'."
	@echo "  make cross-itchio: Package emscripten build for itch.io upload."
	@echo "  make clean: Remove artifacts."
	@echo "  make help: Show this help and create directories."
	@echo "Platforms: $(subst $(space),$(comma),$(PLATFORMS))"
	@echo "Notes:"
	@echo "  - src/main.cpp is provided."
	@echo "  - Assets in $(ASSETS_DIR)/."
	@echo "  - Headers in $(INCLUDE_DIR)/."
	@echo "  - Objects in $(BUILD_DIR)/."
	@echo "  - Itch.io output in $(ITCHIO_DIR)/."
	@echo "  - SDKs in $(SDK_DIR)/<platform>/ (e.g., $(SDK_DIR)/devkitpro/3ds)."
	@echo "  - Ensure toolchains and libraries are installed."

# Clean
clean:
	@echo "Cleaning up..."
	@rm -rf $(BUILD_DIR)/*.o $(TARGET) *.exe *.app *.so *.nro *.dol *.elf *.velf *.rpx *.3dsx *.html *.js *.wasm *.data $(ITCHIO_DIR)

# Phony targets
.PHONY: help clean sdk-dirs init-dirs $(addprefix cross-,$(PLATFORMS))

# Prevent object deletion
.PRECIOUS: $(OBJECTS)

# Define comma and space
comma := ,
space :=
space +=