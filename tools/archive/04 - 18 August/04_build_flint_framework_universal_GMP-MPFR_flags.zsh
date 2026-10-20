#!/bin/zsh
set -e

# Universal build script for FLINT on macOS
# Builds arm64 and x86_64 and merges with lipo

FLINT_VERSION="2.9.0"   # change if needed
FLINT_DIR="flint-${FLINT_VERSION}"

if [[ ! -d $FLINT_DIR ]]; then
  echo "❌ FLINT source directory $FLINT_DIR not found."
  echo "➡️  Download it first: https://github.com/flintlib/flint/releases"
  exit 1
fi

# Output universal build directory
UNIVERSAL_DIR="flint-universal"
mkdir -p $UNIVERSAL_DIR

# Function to build for a specific architecture
build_flint_arch() {
  ARCH=$1
  HOST=$2
  PREFIX=$PWD/build/$ARCH

  echo "🔨 Building FLINT for $ARCH ($HOST)..."

  # Clean previous build for this arch
  rm -rf $PREFIX
  mkdir -p $PREFIX

  pushd $FLINT_DIR

  make distclean || true

  # Add Homebrew GMP + MPFR paths
  CPPFLAGS="-I/opt/homebrew/opt/gmp/include -I/opt/homebrew/opt/mpfr/include" \
  LDFLAGS="-L/opt/homebrew/opt/gmp/lib -L/opt/homebrew/opt/mpfr/lib" \
  ./configure \
    --prefix=$PREFIX \
    --host=$HOST \
    --with-gmp=/opt/homebrew/opt/gmp \
    --with-mpfr=/opt/homebrew/opt/mpfr

  make -j$(sysctl -n hw.logicalcpu)
  make install

  popd
}

# Build for arm64
build_flint_arch "arm64" "arm-apple-darwin"

# Build for x86_64
build_flint_arch "x86_64" "x86_64-apple-darwin"

# Merge libs into universal binaries
mkdir -p $UNIVERSAL_DIR/lib
mkdir -p $UNIVERSAL_DIR/include

echo "📦 Creating universal libraries..."

for lib in build/arm64/lib/*.a; do
  libname=$(basename $lib)
  lipo -create \
    build/arm64/lib/$libname \
    build/x86_64/lib/$libname \
    -output $UNIVERSAL_DIR/lib/$libname
done

# Copy headers (just take from arm64 build)
cp -R build/arm64/include/* $UNIVERSAL_DIR/include/

echo "✅ Universal build complete in $UNIVERSAL_DIR"
