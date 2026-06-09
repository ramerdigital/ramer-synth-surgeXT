#!/bin/bash

# --- Color definitions for premium terminal output ---
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
CYAN='\033[0;36m'
NC='\033[0m' # No Color

echo -e "${BLUE}====================================================${NC}"
echo -e "${CYAN}             Surge XT macOS Build Script            ${NC}"
echo -e "${BLUE}====================================================${NC}"

# Check if CMake is installed
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}Error: cmake is not installed or not in PATH.${NC}"
    echo -e "${YELLOW}Please install CMake (e.g., via 'brew install cmake') and try again.${NC}"
    exit 1
fi

# 1. Purge previous build
echo -e "${YELLOW}Purging previous build directory...${NC}"
if [ -d "build" ]; then
    rm -rf build
    echo -e "${GREEN}✓ Previous 'build' directory successfully deleted.${NC}"
else
    echo -e "${CYAN}No previous 'build' directory found. Skipping purge.${NC}"
fi

# 2. Re-create and configure the project
echo -e "\n${YELLOW}Configuring the project with CMake (Optimized Release)...${NC}"
echo -e "${CYAN}Note: Release build automatically enables -O3 and LTO (Link-Time Optimization).${NC}"

CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Release"

# Build for both Apple Silicon and Intel if ARCHS=universal is specified
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

# 3. Build the project
# Calculate a safe number of parallel jobs based on RAM to prevent system freeze (C++ builds are memory-intensive)
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

echo -e "\n${YELLOW}Building the project in parallel (using $SAFE_JOBS of $CORES CPU cores to prevent memory saturation)...${NC}"
cmake --build build --config Release --parallel "$SAFE_JOBS"

if [ $? -ne 0 ]; then
    echo -e "${RED}Error: Build failed.${NC}"
    exit 1
fi
echo -e "${GREEN}✓ Build completed successfully!${NC}"

# 4. Show output location
PRODUCT_DIR="build/surge_xt_products"
echo -e "\n${BLUE}====================================================${NC}"
echo -e "${GREEN}Build succeeded! Here are the compiled products:${NC}"
echo -e "${BLUE}====================================================${NC}"
if [ -d "$PRODUCT_DIR" ]; then
    ls -la "$PRODUCT_DIR"
    echo -e "\n${CYAN}Products directory path:${NC}"
    echo -e "${NC}$(pwd)/$PRODUCT_DIR${NC}"
else
    echo -e "${YELLOW}Warning: Products directory not found where expected ($PRODUCT_DIR).${NC}"
fi
echo -e "${BLUE}====================================================${NC}"
