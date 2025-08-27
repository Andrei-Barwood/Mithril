#!/bin/zsh
set -e

# ==============================
# Settings
# ==============================
FLINT_VERSION="2.9.0"
FLINT_TARBALL="flint-${FLINT_VERSION}.tar.gz"
FLINT_URL="https://github.com/flintlib/flint/releases/download/v${FLINT_VERSION}/${FLINT_TARBALL}"

WORKDIR="$HOME/flint_build"
SRCDIR="$WORKDIR/flint-${FLINT_VERSION}"
PREFIX="$WORKDIR/flint-universal"

mkdir -p "$WORKDIR"
cd "$WORKDIR"

# ==============================
# Detect Homebrew GMP/MPFR
# ==============================
echo "🔹 Detecting Homebrew dependencies..."
GMP_PREFIX=$(brew --prefix gmp)
MPFR_PREFIX=$(brew --prefix mpfr)

echo "   GMP:  $GMP_PREFIX"
echo "   MPFR: $MPFR_PREFIX"

# ==============================
# Download FLINT tarball if missing
# ==============================
if [[ ! -d "$SRCDIR" ]]; then
    echo "⬇️  Downloading FLINT $FLINT_VERSION..."
    curl -L -o "$FLINT_TARBALL" "$FLINT_URL"

    echo "📦 Extracting..."
    tar -xzf "$FLINT_TARBALL"
fi

# ==============================
# Function to build FLINT per arch
# ==============================
build_flint_arch() {
    ARCH=$1
    BUILD_DIR="$WORKDIR/build-$ARCH"
    INSTALL_DIR="$WORKDIR/install-$ARCH"

    echo "⚡ Building FLINT for $ARCH..."
    rm -rf "$BUILD_DIR" "$INSTALL_DIR"
    mkdir -p "$BUILD_DIR"
    cd "$BUILD_DIR"

    export CC="clang -arch $ARCH"
    export CXX="clang++ -arch $ARCH"
    export CFLAGS="-O3 -arch $ARCH"
    export LDFLAGS="-arch $ARCH -L$GMP_PREFIX/lib -L$MPFR_PREFIX/lib"
    export CPPFLAGS="-I$GMP_PREFIX/include -I$MPFR_PREFIX/include"

    "$SRCDIR/configure" \
        --prefix="$INSTALL_DIR" \
        --with-gmp="$GMP_PREFIX" \
        --with-mpfr="$MPFR_PREFIX"

    make -j$(sysctl -n hw.logicalcpu)
    make install
}

# ==============================
# Build both archs
# ==============================
build_flint_arch arm64
build_flint_arch x86_64

# ==============================
# Merge into universal binaries
# ==============================
echo "🔄 Creating universal binaries..."
rm -rf "$PREFIX"
mkdir -p "$PREFIX/lib" "$PREFIX/include"

# Copy headers (just take from arm64 build)
cp -R "$WORKDIR/install-arm64/include/"* "$PREFIX/include/"

# Merge libraries
for lib in "$WORKDIR/install-arm64/lib/"*.dylib; do
    NAME=$(basename "$lib")
    lipo -create \
        "$WORKDIR/install-arm64/lib/$NAME" \
        "$WORKDIR/install-x86_64/lib/$NAME" \
        -output "$PREFIX/lib/$NAME"
    echo "   ✅ $NAME"
done

echo "🎉 Done!"
echo "Universal FLINT installed in: $PREFIX"
