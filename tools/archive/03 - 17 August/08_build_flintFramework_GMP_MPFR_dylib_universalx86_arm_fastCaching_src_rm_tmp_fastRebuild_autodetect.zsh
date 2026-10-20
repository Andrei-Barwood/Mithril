#!/bin/zsh
set -e

# Directories
BUILD_ROOT="$HOME/flint_build"
FRAMEWORK_DIR="$HOME/Desktop/FLINT.framework"

# Source directories
FLINT_SRC="$BUILD_ROOT/flint2-src"
GMP_SRC="$BUILD_ROOT/gmp-src"
MPFR_SRC="$BUILD_ROOT/mpfr-src"

ARCHS=("arm64" "x86_64")
MAKEFLAGS="-j$(sysctl -n hw.ncpu)"

# Check if binaries exist for fast rebuild
FAST_REBUILD=true
for ARCH in "${ARCHS[@]}"; do
    if [ ! -f "$BUILD_ROOT/flint-$ARCH/lib/libflint.dylib" ] || \
       [ ! -f "$BUILD_ROOT/gmp-$ARCH/lib/libgmp.dylib" ] || \
       [ ! -f "$BUILD_ROOT/mpfr-$ARCH/lib/libmpfr.dylib" ]; then
        FAST_REBUILD=false
        break
    fi
done

echo "=== FLINT.framework build script ==="
if [ "$FAST_REBUILD" = true ]; then
    echo "⚡ Fast rebuild mode detected (all binaries exist)."
else
    echo "⚡ Full build mode required (missing binaries)."
fi

mkdir -p "$FRAMEWORK_DIR/Versions/A/Headers"
mkdir -p "$FRAMEWORK_DIR/Versions/A/Frameworks"

# -------------------------
# Fast rebuild: use existing binaries
# -------------------------
if [ "$FAST_REBUILD" = true ]; then
    echo "🔹 Packaging existing binaries into framework..."
    lipo -create "$BUILD_ROOT/flint-arm64/lib/libflint.dylib" "$BUILD_ROOT/flint-x86_64/lib/libflint.dylib" -output "$FRAMEWORK_DIR/Versions/A/FLINT"
    lipo -create "$BUILD_ROOT/gmp-arm64/lib/libgmp.dylib" "$BUILD_ROOT/gmp-x86_64/lib/libgmp.dylib" -output "$FRAMEWORK_DIR/Versions/A/Frameworks/libgmp.dylib"
    lipo -create "$BUILD_ROOT/mpfr-arm64/lib/libmpfr.dylib" "$BUILD_ROOT/mpfr-x86_64/lib/libmpfr.dylib" -output "$FRAMEWORK_DIR/Versions/A/Frameworks/libmpfr.dylib"

    cp "$BUILD_ROOT/flint-arm64/include/flint/"* "$FRAMEWORK_DIR/Versions/A/Headers/"

# -------------------------
# Full build: compile libraries for each arch
# -------------------------
else
    echo "🔹 Full build: compiling GMP, MPFR, FLINT for each architecture..."

    build_library () {
        local name=$1
        local url=$2
        local src_dir=$3
        local prefix=$4
        local extra_config=$5
        local dylib_path="$prefix/lib/lib$name.dylib"

        if [ -f "$dylib_path" ]; then
            echo "✅ $name already built for $ARCH, skipping."
            return
        fi

        if [ ! -d "$src_dir" ]; then
            git clone "$url" "$src_dir"
        fi

        cd "$src_dir"
        make clean || true
        echo "🔹 Building $name for $ARCH..."
        CFLAGS="-arch $ARCH" LDFLAGS="-arch $ARCH" ./configure --prefix="$prefix" $extra_config
        make $MAKEFLAGS
        make install
    }

    for ARCH in "${ARCHS[@]}"; do
        echo "=== Architecture: $ARCH ==="
        build_library "gmp" "https://gmplib.org/repo/gmp.git" "$GMP_SRC" "$BUILD_ROOT/gmp-$ARCH" "--enable-shared"
        build_library "mpfr" "https://git.mpfr.org/mpfr.git" "$MPFR_SRC" "$BUILD_ROOT/mpfr-$ARCH" "--with-gmp=$BUILD_ROOT/gmp-$ARCH --enable-shared"
        build_library "flint" "https://github.com/wbhart/flint2.git" "$FLINT_SRC" "$BUILD_ROOT/flint-$ARCH" "--with-gmp=$BUILD_ROOT/gmp-$ARCH --with-mpfr=$BUILD_ROOT/mpfr-$ARCH --enable-shared"
    done

    # Create universal dylibs
    echo "🔹 Creating universal (fat) libraries..."
    lipo -create "$BUILD_ROOT/flint-arm64/lib/libflint.dylib" "$BUILD_ROOT/flint-x86_64/lib/libflint.dylib" -output "$FRAMEWORK_DIR/Versions/A/FLINT"
    lipo -create "$BUILD_ROOT/gmp-arm64/lib/libgmp.dylib" "$BUILD_ROOT/gmp-x86_64/lib/libgmp.dylib" -output "$FRAMEWORK_DIR/Versions/A/Frameworks/libgmp.dylib"
    lipo -create "$BUILD_ROOT/mpfr-arm64/lib/libmpfr.dylib" "$BUILD_ROOT/mpfr-x86_64/lib/libmpfr.dylib" -output "$FRAMEWORK_DIR/Versions/A/Frameworks/libmpfr.dylib"

    # Copy headers
    cp "$BUILD_ROOT/flint-arm64/include/flint/"* "$FRAMEWORK_DIR/Versions/A/Headers/"

    # Optional cleanup
    echo "🧹 Cleaning up intermediate build directories..."
    rm -rf "$BUILD_ROOT"
fi

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

echo "✅ Universal self-contained FLINT.framework ready at $FRAMEWORK_DIR"
echo "💡 Drag into Xcode, set 'Embed & Sign'. Works for Intel & Apple Silicon!"
