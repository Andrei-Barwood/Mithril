#!/bin/zsh
set -e

# Paths
FLINT_SRC="$HOME/flint2-src"
BUILD_DIR="$HOME/flint2-build"
FRAMEWORK_DIR="$HOME/Desktop/FLINT.framework"

# Step 1: Clone FLINT if not already
if [ ! -d "$FLINT_SRC" ]; then
  git clone https://github.com/wbhart/flint2.git "$FLINT_SRC"
fi

# Step 2: Create build directory
mkdir -p "$BUILD_DIR"
cd "$FLINT_SRC"

# Step 3: Configure & build static library
./configure --with-gmp --with-mpfr --prefix="$BUILD_DIR"
make -j$(sysctl -n hw.ncpu)
make install

# Step 4: Prepare framework structure
rm -rf "$FRAMEWORK_DIR"
mkdir -p "$FRAMEWORK_DIR/Versions/A/Headers"
mkdir -p "$FRAMEWORK_DIR/Versions/A/Frameworks"

# Step 5: Copy library binary
# Using static library here
cp "$BUILD_DIR/lib/libflint.a" "$FRAMEWORK_DIR/Versions/A/FLINT"

# Step 6: Copy headers
cp "$BUILD_DIR/include/flint/"* "$FRAMEWORK_DIR/Versions/A/Headers/"

# Step 7: Symlinks for macOS framework structure
cd "$FRAMEWORK_DIR"
ln -sf Versions/A/FLINT FLINT
ln -sf Versions/A/Headers Headers
ln -s A Versions/Current

echo "✅ FLINT.framework is ready at $FRAMEWORK_DIR"
echo "💡 Drag into Xcode and set 'Embed & Sign'."
