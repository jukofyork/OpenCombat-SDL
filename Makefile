# OpenCombat SDL Makefile
# SDL2 port build configuration

# Compiler
CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -O2
CXXFLAGS_DEBUG := -std=c++17 -Wall -Wextra -g -O0 -DDEBUG

# SDL2 configuration
SDL_CFLAGS := $(shell pkg-config --cflags sdl2 SDL2_ttf SDL2_mixer 2>/dev/null || echo "-I/usr/include/SDL2 -D_REENTRANT")
SDL_LIBS := $(shell pkg-config --libs sdl2 SDL2_ttf SDL2_mixer 2>/dev/null || echo "-lSDL2 -lSDL2_ttf -lSDL2_mixer")

# TinyXML2 (included as source in src/misc/)
TINYXML_FLAGS :=
TINYXML_LIBS :=

# Include paths
INCLUDES := -I./src -I./src/graphics -I./src/misc -I./src/world -I./src/objects -I./src/application -I./src/states -I./src/ai -I./src/orders -I./src/sound

# All source files (excluding DirectX files for now, and tools)
SRCS := $(wildcard src/main.cpp)
SRCS += $(wildcard src/graphics/*.cpp)
SRCS += $(wildcard src/misc/*.cpp)
SRCS += $(wildcard src/world/*.cpp)
SRCS += $(wildcard src/objects/*.cpp)
SRCS += $(wildcard src/application/*.cpp)
SRCS += $(wildcard src/states/*.cpp)
SRCS += $(wildcard src/ai/*.cpp)
SRCS += $(wildcard src/orders/*.cpp)
SRCS += $(wildcard src/sound/*.cpp)
# Exclude DirectX files (legacy, not needed for SDL port)
# SRCS += $(wildcard src/directx/*.cpp)

# Object files
OBJS := $(SRCS:.cpp=.o)

# Target
TARGET := opencombat
TARGET_DEBUG := opencombat-debug

# Dependency tracking
DEPDIR := .deps
DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$*.d

# Build rules
.PHONY: all clean distclean debug test test-clean check-deps help

all: $(DEPDIR) $(TARGET)

# Create dependency directory
$(DEPDIR):
	@mkdir -p $(DEPDIR)/src/graphics
	@mkdir -p $(DEPDIR)/src/misc
	@mkdir -p $(DEPDIR)/src/world
	@mkdir -p $(DEPDIR)/src/objects
	@mkdir -p $(DEPDIR)/src/application
	@mkdir -p $(DEPDIR)/src/states
	@mkdir -p $(DEPDIR)/src/ai
	@mkdir -p $(DEPDIR)/src/orders
	@mkdir -p $(DEPDIR)/src/sound

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $@ $(SDL_LIBS) $(TINYXML_LIBS) -lpthread -ldl

# Debug build - separate object files to avoid mixing with release
DEBUG_OBJS := $(SRCS:.cpp=-debug.o)

debug: $(DEPDIR) $(TARGET_DEBUG)

$(TARGET_DEBUG): $(DEBUG_OBJS)
	$(CXX) $(DEBUG_OBJS) -o $@ $(SDL_LIBS) $(TINYXML_LIBS) -lpthread -ldl

%-debug.o: %.cpp
	@mkdir -p $(DEPDIR)/$(dir $<)
	$(CXX) $(CXXFLAGS_DEBUG) $(DEPFLAGS) $(INCLUDES) $(SDL_CFLAGS) $(TINYXML_FLAGS) -c $< -o $@

%.o: %.cpp
	@mkdir -p $(DEPDIR)/$(dir $<)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) $(INCLUDES) $(SDL_CFLAGS) $(TINYXML_FLAGS) -c $< -o $@

clean:
	@echo "Cleaning build artifacts..."
	@find src -name "*.o" -delete 2>/dev/null || true
	@find src -name "*-debug.o" -delete 2>/dev/null || true
	@rm -f $(TARGET) $(TARGET_DEBUG)
	@rm -f test_sdl test_main
	@rm -rf $(DEPDIR)
	@echo "Clean complete."

distclean: clean
	@echo "Full cleanup..."
	@find . -name "*.gch" -delete 2>/dev/null || true
	@find . -name "*.bak" -delete 2>/dev/null || true
	@find . -name "*~" -delete 2>/dev/null || true
	@find . -name "*.tmp" -delete 2>/dev/null || true
	@find . -name "core.*" -delete 2>/dev/null || true
	@find . -name "vgcore.*" -delete 2>/dev/null || true
	@echo "Distclean complete."

# Include generated dependencies
-include $(wildcard $(DEPDIR)/src/*.d)
-include $(wildcard $(DEPDIR)/src/*/*.d)

# Test build (minimal)
test: src/main.o src/graphics/Screen.o src/graphics/FontManager.o
	$(CXX) $^ -o test_sdl $(SDL_LIBS) $(TINYXML_LIBS)
	@echo "Test build complete. Run ./test_sdl"

test-clean: clean
	@rm -f test_sdl test_main test_*

# Check dependencies
check-deps:
	@echo "Checking SDL2 dependencies..."
	@pkg-config --exists sdl2 && echo "✓ SDL2 found" || echo "✗ SDL2 not found - install with: sudo apt-get install libsdl2-dev"
	@pkg-config --exists SDL2_ttf && echo "✓ SDL2_ttf found" || echo "✗ SDL2_ttf not found - install with: sudo apt-get install libsdl2-ttf-dev"
	@pkg-config --exists SDL2_mixer && echo "✓ SDL2_mixer found" || echo "✗ SDL2_mixer not found - install with: sudo apt-get install libsdl2-mixer-dev"
	@ldconfig -p | grep -q tinyxml2 && echo "✓ tinyxml2 found" || echo "✗ tinyxml2 not found - install with: sudo apt-get install libtinyxml2-dev"

# Help
help:
	@echo "OpenCombat SDL Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all         - Build release version (default)"
	@echo "  debug       - Build debug version"
	@echo "  clean       - Remove build artifacts"
	@echo "  distclean   - Remove all generated files including backups"
	@echo "  test        - Build minimal test executable"
	@echo "  test-clean  - Remove test executables"
	@echo "  check-deps  - Verify SDL2 dependencies are installed"
	@echo "  help        - Show this help message"
	@echo ""
	@echo "Build outputs:"
	@echo "  opencombat       - Release executable"
	@echo "  opencombat-debug - Debug executable"
