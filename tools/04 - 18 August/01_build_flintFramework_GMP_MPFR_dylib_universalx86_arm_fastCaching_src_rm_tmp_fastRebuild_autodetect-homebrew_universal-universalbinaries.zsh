#!/bin/zsh
set -e

echo "🔹 Using Homebrew GMP/MPFR:"
GMP=$(brew --prefix gmp)/lib/libgmp.dylib
MPFR=$(brew --prefix mpfr)/lib/libmpfr.dylib
echo "  GMP:  $GMP"
echo "  MPFR: $MPFR"

ROOTDIR="$HOME/flint_build"
SRCDIR="$ROOTDIR/flint2-src"
INSTALLDIR="$ROOTDIR/universal"
BUILD_ARM64="$ROOTDIR/build-arm64"
BUILD_X86_64="$ROOTDIR/build-x86_64"

rm -rf "$ROOTDIR"
mkdir -p "$BUILD_ARM64" "$BUILD_X86_64" "$INSTALLDIR/lib" "$INSTALLDIR/include"

echo "⚡ Cloning FLINT source..."
git clone https://github.com/wbhart/flint2.git "$SRCDIR"

# ---- Function to build per-arch ----
build_flint_arch() {
  ARCH=$1
  BUILDDIR=$2

  echo "🔹 Building FLINT for $ARCH..."
  cd "$BUILDDIR"
  "$SRCDIR/configure" \
      --prefix="$BUILDDIR/install" \
      --with-gmp=$(brew --prefix gmp) \
      --with-mpfr=$(brew --prefix mpfr) \
      CFLAGS="-arch $ARCH -O3 -fPIC" \
      LDFLAGS="-arch $ARCH"

  make -j$(sysctl -n hw.ncpu)
  make install
}

# ---- Build ARM64 ----
build_flint_arch arm64 "$BUILD_ARM64"

# ---- Build x86_64 ----
build_flint_arch x86_64 "$BUILD_X86_64"

# ---- Merge with lipo ----
echo "🔹 Creating universal binaries..."
for lib in $(ls $BUILD_ARM64/install/lib/libflint*.dylib); do
  LIBNAME=$(basename $lib)
  lipo -create \
    "$BUILD_ARM64/install/lib/$LIBNAME" \
    "$BUILD_X86_64/install/lib/$LIBNAME" \
    -output "$INSTALLDIR/lib/$LIBNAME"
done

# Copy headers (only need once, same for both)
cp -R "$BUILD_ARM64/install/include/" "$INSTALLDIR/include/"

echo "✅ Universal FLINT build complete."
echo "   Libraries: $INSTALLDIR/lib"
echo "   Headers:   $INSTALLDIR/include"
