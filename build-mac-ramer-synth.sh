#!/bin/bash

# --- Color definitions for premium terminal output ---
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}====================================================${NC}"
echo -e "${CYAN}   Ramer Synth Clean Full Build & AU Only Script    ${NC}"
echo -e "${BLUE}====================================================${NC}"

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: cmake is not installed or not in PATH.${NC}"
    echo -e "${YELLOW}Please install CMake (e.g., via 'brew install cmake') and try again.${NC}"
    exit 1
fi

# 1. Clean build directory by default (forces complete recompilation of all targets)
echo -e "${YELLOW}Purging previous build directory to ensure a clean build...${NC}"
if [ -d "build" ]; then
    rm -rf build
    echo -e "${GREEN}✓ Previous 'build' directory successfully deleted.${NC}"
else
    echo -e "${CYAN}No previous 'build' directory found.${NC}"
fi

# 2. Re-configure the project with CMake
echo -e "\n${YELLOW}Configuring the complete project with CMake (Release)...${NC}"
CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Release"

if [ "$ARCHS" = "universal" ]; then
    echo -e "${YELLOW}→ Targeting Universal Binary (arm64;x86_64) for distribution.${NC}"
    cmake -B build -S . $CMAKE_FLAGS -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
else
    echo -e "${CYAN}→ Targeting Native Host Architecture ($(uname -m)) for maximum local performance.${NC}"
    cmake -B build -S . $CMAKE_FLAGS
fi

if [ $? -ne 0 ]; then
    echo -e "${RED}Error: CMake configuration failed.${NC}"
    exit 1
fi
echo -e "${GREEN}✓ CMake configured successfully.${NC}"

# 3. Calculate safe number of parallel jobs based on RAM to prevent macOS freeze
MEM_BYTES=$(sysctl -n hw.memsize 2>/dev/null || echo 0)
CORES=$(sysctl -n hw.ncpu 2>/dev/null || echo 2)
if [ "$MEM_BYTES" -gt 0 ]; then
    MEM_GB=$(( MEM_BYTES / 1024 / 1024 / 1024 ))
    # Estimate ~3GB RAM per C++ compiler job to prevent swap thrashing
    SAFE_JOBS=$(( MEM_GB / 3 ))
    [ "$SAFE_JOBS" -lt 1 ] && SAFE_JOBS=1
    [ "$SAFE_JOBS" -gt "$CORES" ] && SAFE_JOBS=$CORES
else
    SAFE_JOBS=2
fi

# Force recompilation time stamp inside PluginEditor
touch src/ramer-synth/PluginEditor.cpp

# 4. Build the complete project cleanly (building all wrappers and dependencies)
echo -e "\n${YELLOW}Building the complete project cleanly (using $SAFE_JOBS of $CORES CPU cores to prevent system freeze)...${NC}"
cmake --build build --config Release --parallel "$SAFE_JOBS"

if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Build failed.${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Complete build finished successfully!${NC}"

# 5. Locate built AU plugin and copy to User plug-ins folder (VST3 / CLAP are built but NOT published/installed)
AU_SRC="build/src/ramer-synth/ramer-synth_artefacts/Release/AU/ramer-synth.component"
AU_DST="$HOME/Library/Audio/Plug-Ins/Components"

echo -e "\n${YELLOW}Publishing/Installing AU plugin only...${NC}"

if [ -d "$AU_SRC" ]; then
    mkdir -p "$AU_DST"
    echo -e "${CYAN}→ Installing AU to ${AU_DST}/ramer-synth.component...${NC}"
    rm -rf "${AU_DST}/ramer-synth.component"
    cp -R "$AU_SRC" "$AU_DST/"
    echo -e "${GREEN}✓ AU installed successfully.${NC}"
else
    echo -e "${RED}Error: AU plugin not found at expected path: $AU_SRC${NC}"
    exit 1
fi

# 6. Force macOS to reload AU plugin cache registrar and kill hosting processes
echo -e "\n${YELLOW}Clearing macOS AudioUnit cache registrar and restarting components...${NC}"
killall -9 AudioComponentRegistrar 2>/dev/null || true
killall -9 auvaltool 2>/dev/null || true
killall -9 AUHostingServiceXPC 2>/dev/null || true

echo -e "${GREEN}✓ AudioUnit cache cleared and hosting daemons killed.${NC}"

echo -e "${BLUE}====================================================${NC}"
echo -e "${GREEN}Clean build and AU installation complete!${NC}"
echo -e "${BLUE}====================================================${NC}"
