#!/usr/bin/env bash
set -euo pipefail

# Cross-compile Windows exe (x86/x64) on Linux/macOS using mingw-w64
# Usage:
#   ./build_cross.sh                 # default x64
#   ./build_cross.sh -Arch x86       # 32-bit
#   ./build_cross.sh -Static         # link libgcc/libstdc++ statically
#   ./build_cross.sh -Winver 0x0601  # set _WIN32_WINNT (default Win7)
#   ./build_cross.sh -Clean          # remove intermediates and finder.exe

ARCH="x64"
STATIC="0"
WINVER="0x0601"   # Windows 7
CLEAN="0"

while [[ $# -gt 0 ]]; do
  case "$1" in
    -Arch)
      ARCH="$2"; shift 2;;
    -Static)
      STATIC="1"; shift;;
    -Winver)
      WINVER="$2"; shift 2;;
    -Clean)
      CLEAN="1"; shift;;
    *)
      echo "Unknown arg: $1"; exit 1;;
  esac
done

if [[ "$CLEAN" == "1" ]]; then
  rm -f finder.exe *.o
  echo "Cleaned finder.exe and object files."
  exit 0
fi

if [[ "$ARCH" == "x86" ]]; then
  CXX="i686-w64-mingw32-g++"
  MACHINE="X86"
else
  CXX="x86_64-w64-mingw32-g++"
  MACHINE="X64"
fi

if ! command -v "$CXX" >/dev/null 2>&1; then
  echo "Error: $CXX not found. Please install mingw-w64 toolchain." >&2
  echo "  Debian/Ubuntu: sudo apt-get install -y mingw-w64" >&2
  echo "  macOS (Homebrew): brew install mingw-w64" >&2
  exit 1
fi

SRC_FILES=$(printf ' %q' src/*.cpp)
EXTRA_FLAGS=""
if [[ "$STATIC" == "1" ]]; then
  EXTRA_FLAGS+=" -static -static-libgcc -static-libstdc++"
fi

set -x
"$CXX" -std=c++17 -O2 -DNDEBUG -municode \
  -DWINVER=${WINVER} -D_WIN32_WINNT=${WINVER} \
  -o finder.exe main.cpp ${SRC_FILES} \
  ${EXTRA_FLAGS}
set +x

echo "Build succeeded: finder.exe (${ARCH}, _WIN32_WINNT=${WINVER})"


