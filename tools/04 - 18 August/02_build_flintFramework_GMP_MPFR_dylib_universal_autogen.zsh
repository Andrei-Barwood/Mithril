#!/bin/zsh
set -e

# Paths
BUILD_DIR="$HOME/flint_build"
SRC_DIR="$BUILD_DIR/flint2-src"
PREFIX_ARM="$BUILD_DIR/prefix-arm64"
PREFIX_X86="$BUILD_DIR/prefix-x86_64"
UNIVERSAL_PREFIX="$BUILD_DIR/prefix-universal"

# Detect Homebrew GMP/MPFR
GMP=$(brew --prefix gmp)
MPFR=$(brew --prefix mpfr)

echo "🔹 Using Homebrew GMP/MPFR:"
echo "  GMP:  $GMP/lib/libgmp.dylib"
echo "  MPFR: $MPFR/lib/libmpfr.dylib"

# Clean build dirs
rm -rf "$SRC_DIR" "$PREFIX_ARM" "$PREFIX_X86" "$UNIVERSAL_PREFIX"
mkdir -p "$BUILD_DIR"

# Clone FLINT source
echo "⚡ Cloning FLINT source..."
git clone https://github.com/wbhart/flint2.git "$SRC_DIR"

build_flint_arch() {
  local ARCH=$1
  local PREFIX=$2

  echo "🔹 Building FLINT for $ARCH..."

  cd "$SRC_DIR"

  # Ensure autotools bootstrap
  if [ ! -f configure ]; then
    echo "⚡ Running autogen.sh..."
    ./autogen.sh || autoreconf -i
  fi

  # Separate build dir
  mkdir -p "$BUILD_DIR/build-$ARCH"
  cd "$BUILD_DIR/build-$ARCH"

  # Configure for arch
  CFLAGS="-arch $ARCH -O3 -fPIC" \
  LDFLAGS="-arch $ARCH" \
  "$SRC_DIR/configure" \
    --prefix="$PREFIX" \
    --with-gmp="$GMP" \
    --with-mpfr="$MPFR"

  make -j$(sysctl -n hw.ncpu)
  make install
}

# Build arm64 + x86_64
build_flint_arch arm64 "$PREFIX_ARM"
build_flint_arch x86_64 "$PREFIX_X86"

# Merge into universal
echo "🔹 Creating universal binaries..."
mkdir -p "$UNIVERSAL_PREFIX/lib"
mkdir -p "$UNIVERSAL_PREFIX/include"

cp -r "$PREFIX_ARM/include/"* "$UNIVERSAL_PREFIX/include/"

for lib in "$PREFIX_ARM/lib/"*.dylib; do
  libname=$(basename "$lib")
  lipo -create \
    "$PREFIX_ARM/lib/$libname" \
    "$PREFIX_X86/lib/$libname" \
    -output "$UNIVERSAL_PREFIX/lib/$libname"
done

echo "✅ Universal FLINT built at: $UNIVERSAL_PREFIX"
