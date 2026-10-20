#!/bin/zsh
set -e

# -------------------------
# Directories
# -------------------------
BUILD_ROOT="$HOME/flint_build"
FRAMEWORK_DIR="$HOME/Desktop/FLINT.framework"
FLINT_SRC="$BUILD_ROOT/flint2-src"

ARCHS=("arm64" "x86_64")
MAKEFLAGS="-j$(sysctl -n hw.ncpu)"

# -------------------------
# Detect Homebrew prefix
# -------------------------
BREW_PREFIX=$(brew --prefix)
GMP_PREFIX="$BREW_PREFIX"
MPFR_PREFIX="$BREW_PREFIX"

echo "🔹 Using Homebrew GMP/MPFR:"
echo "  GMP:  $GMP_PREFIX/lib/libgmp.dylib"
echo "  MPFR: $MPFR_PREFIX/lib/libmpfr.dylib"

# -------------------------
# Prepare framework
# -------------------------
rm -rf "$FRAMEWORK_DIR"
mkdir -p "$FRAMEWORK_DIR/Versions/A/Headers"
mkdir -p "$FRAMEWORK_DIR/Versions/A/Frameworks"

# -------------------------
# Check if binaries exist for fast rebuild
# -------------------------
FAST_REBUILD=true
for ARCH in "${ARCHS[@]}"; do
    if [ ! -f "$BUILD_ROOT/flint-$ARCH/lib/libflint.dylib" ]; then
        FAST_REBUILD=false
        break
    fi
done

if [ "$FAST_REBUILD" = true ]; then
    echo "⚡ Fast rebuild mode detected (FLINT binaries exist)."
else
    echo "⚡ Full build mode (compiling FLINT)."
fi

# -------------------------
# Build FLINT
# -------------------------
build_flint () {
    local arch=$1
    local src_dir="$FLINT_SRC"
    local prefix="$BUILD_ROOT/flint-$arch"

    if [ -f "$prefix/lib/libflint.dylib" ]; then
        echo "✅ FLINT already built for $arch, skipping."
        return
    fi

    if [ ! -d "$src_dir" ]; then
        git clone https://github.com/wbhart/flint2.git "$src_dir"
    fi

    cd "$src_dir"
    make clean || true
    echo "🔹 Building FLINT for $arch using Homebrew GMP/MPFR..."
    CFLAGS="-arch $arch" LDFLAGS="-arch $arch" \
    ./configure \
        --prefix="$prefix" \
        --with-gmp="$GMP_PREFIX" \
        --with-mpfr="$MPFR_PREFIX" \
        --enable-shared
    make $MAKEFLAGS
    make install
}

if [ "$FAST_REBUILD" = false ]; then
    for ARCH in "${ARCHS[@]}"; do
        build_flint "$ARCH"
    done
fi

# -------------------------
# Create universal FLINT dylib
# -------------------------
echo "🔹 Creating universal (fat) FLINT library..."
lipo -create "$BUILD_ROOT/flint-arm64/lib/libflint.dylib" "$BUILD_ROOT/flint-x86_64/lib/libflint.dylib" \
    -output "$FRAMEWORK_DIR/Versions/A/FLINT"

# -------------------------
# Copy Homebrew GMP/MPFR dylibs
# -------------------------
echo "🔹 Copying Homebrew GMP/MPFR dylibs into framework..."
cp "$GMP_PREFIX/lib/libgmp.dylib" "$FRAMEWORK_DIR/Versions/A/Frameworks/"
cp "$MPFR_PREFIX/lib/libmpfr.dylib" "$FRAMEWORK_DIR/Versions/A/Frameworks/"

# -------------------------
# Copy headers from FLINT build
# -------------------------
echo "🔹 Copying FLINT headers..."
cp "$BUILD_ROOT/flint-arm64/include/flint/"* "$FRAMEWORK_DIR/Versions/A/Headers/"

# -------------------------
# Adjust install names
# -------------------------
install_name_tool -id @rpath/FLINT.framework/FLINT "$FRAMEWORK_DIR/Versions/A/FLINT"
install_name_tool -id @rpath/libgmp.dylib "$FRAMEWORK_DIR/Versions/A/Frameworks/libgmp.dylib"
install_name_tool -id @rpath/libmpfr.dylib "$FRAMEWORK_DIR/Versions/A/Frameworks/libmpfr.dylib"

# -------------------------
# Create framework symlinks
# -------------------------
cd "$FRAMEWORK_DIR"
ln -sf Versions/A/FLINT FLINT
ln -sf Versions/A/Headers Headers
ln -s A Versions/Current
ln -sf Versions/A/Frameworks Frameworks

# -------------------------
# Cleanup intermediate build files
# -------------------------
echo "🧹 Cleaning up intermediate build directories..."
rm -rf "$BUILD_ROOT"

echo "✅ FLINT.framework ready at $FRAMEWORK_DIR"
echo "💡 Drag into Xcode, set 'Embed & Sign'. Works for Intel & Apple Silicon!"
