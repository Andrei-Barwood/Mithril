#!/bin/zsh
set -e

# ============ Paths ============
BUILD_DIR="$HOME/flint_build"
SRC_DIR="$BUILD_DIR/flint2-src"
PREFIX_ARM="$BUILD_DIR/prefix-arm64"
PREFIX_X86="$BUILD_DIR/prefix-x86_64"
UNIVERSAL_PREFIX="$BUILD_DIR/prefix-universal"
FRAMEWORK_DIR="$HOME/Desktop/FLINT.framework"

# ============ Homebrew deps ============
BREW_GMP_PREFIX=$(brew --prefix gmp)
BREW_MPFR_PREFIX=$(brew --prefix mpfr)
GMP_DYLIB="$BREW_GMP_PREFIX/lib/libgmp.dylib"
MPFR_DYLIB="$BREW_MPFR_PREFIX/lib/libmpfr.dylib"

echo "🔹 Using Homebrew GMP/MPFR:"
echo "  GMP:  $GMP_DYLIB"
echo "  MPFR: $MPFR_DYLIB"

# ============ Clean previous ============
rm -rf "$BUILD_DIR" "$FRAMEWORK_DIR"
mkdir -p "$BUILD_DIR" "$UNIVERSAL_PREFIX/lib" "$UNIVERSAL_PREFIX/include"

# ============ Clone FLINT ============
echo "⚡ Cloning FLINT source..."
git clone https://github.com/wbhart/flint2.git "$SRC_DIR"

# ============ Ensure configure exists ============
cd "$SRC_DIR"
if [ ! -f configure ]; then
  echo "⚙️  Bootstrapping autotools (autogen.sh / autoreconf)..."
  if [ -x ./autogen.sh ]; then
    ./autogen.sh
  else
    autoreconf -i
  fi
fi

# ============ Build function ============
build_flint_arch() {
  local ARCH=$1
  local PREFIX=$2
  local BLD="$BUILD_DIR/build-$ARCH"

  echo "🔹 Building FLINT for $ARCH..."
  rm -rf "$BLD" "$PREFIX"
  mkdir -p "$BLD"
  cd "$BLD"

  CFLAGS="-arch $ARCH -O3 -fPIC" \
  LDFLAGS="-arch $ARCH" \
  "$SRC_DIR/configure" \
    --prefix="$PREFIX" \
    --with-gmp="$BREW_GMP_PREFIX" \
    --with-mpfr="$BREW_MPFR_PREFIX" \
    --enable-shared

  make -j"$(sysctl -n hw.ncpu)"
  make install
}

# ============ Build both archs ============
build_flint_arch arm64  "$PREFIX_ARM"
build_flint_arch x86_64 "$PREFIX_X86"

# ============ Lipo universal libflint ============
echo "🔹 Creating universal libflint.dylib..."
FLINT_LIB_NAME=libflint.dylib
lipo -create \
  "$PREFIX_ARM/lib/$FLINT_LIB_NAME" \
  "$PREFIX_X86/lib/$FLINT_LIB_NAME" \
  -output "$UNIVERSAL_PREFIX/lib/$FLINT_LIB_NAME"

# Headers (same across archs)
cp -R "$PREFIX_ARM/include/" "$UNIVERSAL_PREFIX/include/"

# ============ Create Framework bundle ============
echo "📦 Building FLINT.framework bundle..."
# Structure
mkdir -p "$FRAMEWORK_DIR/Versions/A/Headers"
mkdir -p "$FRAMEWORK_DIR/Versions/A/Frameworks"
mkdir -p "$FRAMEWORK_DIR/Versions/A/Modules"

# Copy headers
cp -R "$UNIVERSAL_PREFIX/include/"* "$FRAMEWORK_DIR/Versions/A/Headers/"

# Copy libflint (as the framework binary; named 'FLINT' inside)
cp "$UNIVERSAL_PREFIX/lib/$FLINT_LIB_NAME" "$FRAMEWORK_DIR/Versions/A/FLINT"

# Copy and (if needed) universalize GMP/MPFR into Frameworks/
copy_or_universalize() {
  local LIBNAME=$1            # e.g. libgmp.dylib
  local DEST="$FRAMEWORK_DIR/Versions/A/Frameworks/$LIBNAME"
  local ARM="/opt/homebrew/lib/$LIBNAME"
  local X86="/usr/local/lib/$LIBNAME"

  if [ -f "$GMP_DYLIB" ] && [ "$LIBNAME" = "libgmp.dylib" ]; then
    ARM="$GMP_DYLIB"
  fi
  if [ -f "$MPFR_DYLIB" ] && [ "$LIBNAME" = "libmpfr.dylib" ]; then
    ARM="$MPFR_DYLIB"
  fi

  if [ -f "$ARM" ]; then
    local INFO=$(lipo -info "$ARM" 2>/dev/null || true)
    if [[ "$INFO" == *"arm64 x86_64"* || "$INFO" == *"x86_64 arm64"* ]]; then
      cp "$ARM" "$DEST"
      echo "✅ $LIBNAME already universal; copied."
      return
    fi
  fi

  # Try to combine known locations if not universal
  if [ -f "$ARM" ] && [ -f "$X86" ]; then
    echo "🔹 Creating universal $LIBNAME..."
    lipo -create "$ARM" "$X86" -output "$DEST"
  else
    echo "❌ Could not find both archs for $LIBNAME:"
    echo "   arm64 candidate: $ARM"
    echo "   x86_64 candidate: $X86"
    exit 1
  fi
}

copy_or_universalize "libgmp.dylib"
copy_or_universalize "libmpfr.dylib"

# ============ module.modulemap ============
cat > "$FRAMEWORK_DIR/Versions/A/Modules/module.modulemap" <<'EOF'
framework module FLINT {
  umbrella header "flint.h"
  export *
  module * { export * }
}
EOF

# ============ Symlinks ============
cd "$FRAMEWORK_DIR"
ln -sf "A" Versions/Current
ln -sf "Versions/Current/FLINT" FLINT
ln -sf "Versions/Current/Headers" Headers
ln -sf "Versions/Current/Modules" Modules
ln -sf "Versions/Current/Frameworks" Frameworks

# ============ Fix install names ============
echo "🛠  Rewriting install names (rpaths) to point inside the framework..."
# Set the framework binary's ID
install_name_tool -id "@rpath/FLINT.framework/FLINT" "Versions/A/FLINT"

# Point libflint’s references to embedded GMP/MPFR inside the framework
# First discover current paths:
CURRENT_GMP_REF=$(otool -L "Versions/A/FLINT" | awk '/libgmp\.dylib/ {print $1; exit}')
CURRENT_MPFR_REF=$(otool -L "Versions/A/FLINT" | awk '/libmpfr\.dylib/ {print $1; exit}')

if [ -n "$CURRENT_GMP_REF" ]; then
  install_name_tool -change "$CURRENT_GMP_REF" "@rpath/FLINT.framework/Frameworks/libgmp.dylib" "Versions/A/FLINT"
fi
if [ -n "$CURRENT_MPFR_REF" ]; then
  install_name_tool -change "$CURRENT_MPFR_REF" "@rpath/FLINT.framework/Frameworks/libmpfr.dylib" "Versions/A/FLINT"
fi

# Give IDs to embedded dylibs (so other consumers could link to them if needed)
install_name_tool -id "@rpath/FLINT.framework/Frameworks/libgmp.dylib" "Versions/A/Frameworks/libgmp.dylib"
install_name_tool -id "@rpath/FLINT.framework/Frameworks/libmpfr.dylib" "Versions/A/Frameworks/libmpfr.dylib"

# ============ Verify ============
echo "🔎 Verifying load commands (otool -L):"
echo "— FLINT.framework/FLINT —"
otool -L "Versions/A/FLINT" || true
echo
echo "— Embedded libgmp.dylib —"
otool -L "Versions/A/Frameworks/libgmp.dylib" || true
echo
echo "— Embedded libmpfr.dylib —"
otool -L "Versions/A/Frameworks/libmpfr.dylib" || true

echo
echo "✅ Universal FLINT.framework ready:"
echo "   $FRAMEWORK_DIR"
echo "💡 In Xcode: add to the target ➜ Frameworks, Libraries & Embedded Content ➜ set to “Embed & Sign”."
