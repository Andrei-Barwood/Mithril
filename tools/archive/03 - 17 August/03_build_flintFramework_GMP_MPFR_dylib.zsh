#!/bin/zsh
set -e

# Directories
BUILD_ROOT="$HOME/flint_build"
FLINT_SRC="$BUILD_ROOT/flint2-src"
GMP_SRC="$BUILD_ROOT/gmp-src"
MPFR_SRC="$BUILD_ROOT/mpfr-src"
FRAMEWORK_DIR="$HOME/Desktop/FLINT.framework"

# Create build directories
mkdir -p "$BUILD_ROOT"
mkdir -p "$FRAMEWORK_DIR/Versions/A/Headers"
mkdir -p "$FRAMEWORK_DIR/Versions/A/Frameworks"

# -------------------------
# 1. Build GMP (dynamic)
# -------------------------
if [ ! -d "$GMP_SRC" ]; then
    git clone https://gmplib.org/repo/gmp.git "$GMP_SRC"
fi
cd "$GMP_SRC"
./configure --prefix="$BUILD_ROOT/gmp-install" --enable-shared
make -j$(sysctl -n hw.ncpu)
make install

# -------------------------
# 2. Build MPFR (dynamic)
# -------------------------
if [ ! -d "$MPFR_SRC" ]; then
    git clone https://git.mpfr.org/mpfr.git "$MPFR_SRC"
fi
cd "$MPFR_SRC"
./configure --with-gmp="$BUILD_ROOT/gmp-install" --prefix="$BUILD_ROOT/mpfr-install" --enable-shared
make -j$(sysctl -n hw.ncpu)
make install

# -------------------------
# 3. Build FLINT (dynamic)
# -------------------------
if [ ! -d "$FLINT_SRC" ]; then
    git clone https://github.com/wbhart/flint2.git "$FLINT_SRC"
fi
cd "$FLINT_SRC"
./configure --with-gmp="$BUILD_ROOT/gmp-install" --with-mpfr="$BUILD_ROOT/mpfr-install" --prefix="$BUILD_ROOT/flint-install" --enable-shared
make -j$(sysctl -n hw.ncpu)
make install

# -------------------------
# 4. Copy FLINT dylib
# -------------------------
cp "$BUILD_ROOT/flint-install/lib/libflint.dylib" "$FRAMEWORK_DIR/Versions/A/FLINT"

# -------------------------
# 5. Copy headers
# -------------------------
cp "$BUILD_ROOT/flint-install/include/flint/"* "$FRAMEWORK_DIR/Versions/A/Headers/"

# -------------------------
# 6. Embed GMP & MPFR dylibs
# -------------------------
mkdir -p "$FRAMEWORK_DIR/Versions/A/Frameworks"
cp "$BUILD_ROOT/gmp-install/lib/libgmp.dylib" "$FRAMEWORK_DIR/Versions/A/Frameworks/"
cp "$BUILD_ROOT/mpfr-install/lib/libmpfr.dylib" "$FRAMEWORK_DIR/Versions/A/Frameworks/"

# -------------------------
# 7. Adjust install names
# -------------------------
install_name_tool -id @rpath/FLINT.framework/FLINT "$FRAMEWORK_DIR/Versions/A/FLINT"
install_name_tool -id @rpath/libgmp.dylib "$FRAMEWORK_DIR/Versions/A/Frameworks/libgmp.dylib"
install_name_tool -id @rpath/libmpfr.dylib "$FRAMEWORK_DIR/Versions/A/Frameworks/libmpfr.dylib"

# -------------------------
# 8. Create macOS framework symlinks
# -------------------------
cd "$FRAMEWORK_DIR"
ln -sf Versions/A/FLINT FLINT
ln -sf Versions/A/Headers Headers
ln -s A Versions/Current
ln -sf Versions/A/Frameworks Frameworks

echo "✅ Self-contained dynamic FLINT.framework ready at $FRAMEWORK_DIR"
echo "💡 Drag into Xcode, set 'Embed & Sign'. No external deps needed."
