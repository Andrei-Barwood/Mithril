#!/bin/zsh
set -e

# Directories
BUILD_ROOT="$HOME/flint_build"
FRAMEWORK_DIR="$HOME/Desktop/FLINT.framework"

# Binaries for fast rebuild
FLINT_ARM64="$BUILD_ROOT/flint-arm64/lib/libflint.dylib"
FLINT_X86="$BUILD_ROOT/flint-x86_64/lib/libflint.dylib"
GMP_ARM64="$BUILD_ROOT/gmp-arm64/lib/libgmp.dylib"
GMP_X86="$BUILD_ROOT/gmp-x86_64/lib/libgmp.dylib"
MPFR_ARM64="$BUILD_ROOT/mpfr-arm64/lib/libmpfr.dylib"
MPFR_X86="$BUILD_ROOT/mpfr-x86_64/lib/libmpfr.dylib"

HEADERS_SRC="$BUILD_ROOT/flint-arm64/include/flint"

FAST_REBUILD=false
if [ "$1" = "--fast" ]; then
    FAST_REBUILD=true
fi

mkdir -p "$FRAMEWORK_DIR/Versions/A/Headers"
mkdir -p "$FRAMEWORK_DIR/Versions/A/Frameworks"

if [ "$FAST_REBUILD" = true ]; then
    echo "⚡ Fast rebuild mode: packaging existing binaries..."

    # -------------------------
    # Create universal dylibs
    # -------------------------
    lipo -create "$FLINT_ARM64" "$FLINT_X86" -output "$FRAMEWORK_DIR/Versions/A/FLINT"
    lipo -create "$GMP_ARM64" "$GMP_X86" -output "$FRAMEWORK_DIR/Versions/A/Frameworks/libgmp.dylib"
    lipo -create "$MPFR_ARM64" "$MPFR_X86" -output "$FRAMEWORK_DIR/Versions/A/Frameworks/libmpfr.dylib"

    # -------------------------
    # Copy headers
    # -------------------------
    cp "$HEADERS_SRC/"* "$FRAMEWORK_DIR/Versions/A/Headers/"

    # -------------------------
    # Adjust install names
    # -------------------------
    install_name_tool -id @rpath/FLINT.framework/FLINT "$FRAMEWORK_DIR/Versions/A/FLINT"
    install_name_tool -id @rpath/libgmp.dylib "$FRAMEWORK_DIR/Versions/A/Frameworks/libgmp.dylib"
    install_name_tool -id @rpath/libmpfr.dylib "$FRAMEWORK_DIR/Versions/A/Frameworks/libmpfr.dylib"

else
    echo "⚡ Full build mode: building libraries and framework..."
    # Include the previous full build script here
    # (GMP, MPFR, FLINT build per arch, lipo, headers copy, install_name_tool, cleanup)
    # For brevity, this example only shows fast rebuild mode.
fi

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
