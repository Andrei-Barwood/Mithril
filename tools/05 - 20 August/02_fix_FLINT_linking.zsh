#!/bin/zsh

# =================== Automated FLINT/GMP/MPFR Linking Fix Script ===================
# Intended for macOS ARM64 (Apple Silicon)
# Detects and resolves GMP header/library issues for FLINT builds or user code.
# ================================================================================

set -e

# Colors for output
RED='\033[0;31m'
GRN='\033[0;32m'
YLW='\033[1;33m'
BLU='\033[0;34m'
NC='\033[0m'

msg()      { echo -e "${BLU}[INFO]${NC} $1"; }
ok()       { echo -e "${GRN}[SUCCESS]${NC} $1"; }
warn()     { echo -e "${YLW}[WARNING]${NC} $1"; }
err()      { echo -e "${RED}[ERROR]${NC} $1"; }
bail()     { err "$1"; exit 1; }

msg "Checking Homebrew, GMP, MPFR, FLINT..."

# 1. Ensure Homebrew is installed (never as root!)
if ! command -v brew &> /dev/null; then
    msg "Installing Homebrew..."
    NONINTERACTIVE=1 /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)" || bail "Failed to install Homebrew"
    eval "$(/opt/homebrew/bin/brew shellenv)"
else
    ok "Homebrew is already installed"
fi

# 2. Install GMP, MPFR & FLINT or ensure they're not broken
msg "Ensuring gmp, mpfr, and flint are installed and (re)linked..."
brew install gmp mpfr flint
brew link --overwrite --force gmp
brew link --overwrite --force mpfr
brew link --overwrite --force flint

# 3. Confirm headers and libraries exist
INCDIR="$(brew --prefix)/include"
LIBDIR="$(brew --prefix)/lib"
[[ -f "$INCDIR/gmp.h" ]] || bail "gmp.h not found in $INCDIR; GMP install/link failed."
[[ -f "$LIBDIR/libgmp.dylib" ]] || bail "libgmp.dylib not found in $LIBDIR; GMP install/link failed."
ok "GMP found: $INCDIR/gmp.h, $LIBDIR/libgmp.dylib"

[[ -f "$INCDIR/mpfr.h" ]] || warn "mpfr.h not found in $INCDIR"
[[ -f "$LIBDIR/libmpfr.dylib" ]] || warn "libmpfr.dylib not found in $LIBDIR"
[[ -d "$INCDIR/flint" && -f "$INCDIR/flint/flint.h" ]] || bail "FLINT header flint.h not found in $INCDIR/flint"
[[ -f "$LIBDIR/libflint.dylib" ]] || bail "FLINT library not found in $LIBDIR"

# 4. Export environment variables for the session and test them
export C_INCLUDE_PATH="$INCDIR:$C_INCLUDE_PATH"
export LIBRARY_PATH="$LIBDIR:$LIBRARY_PATH"
export LD_LIBRARY_PATH="$LIBDIR:$LD_LIBRARY_PATH"
ok "Exported C_INCLUDE_PATH, LIBRARY_PATH, and LD_LIBRARY_PATH for this session."

# 5. Test compilation of a one-line GMP program
msg "Testing simple GMP compile..."
cat > gmp_test.c <<EOF
#include <gmp.h>
#include <stdio.h>
int main() { mpz_t x; mpz_init_set_ui(x, 42); gmp_printf("%Zd\\n", x); mpz_clear(x); return 0; }
EOF
gcc -I"$INCDIR" -L"$LIBDIR" -lgmp -o gmp_test gmp_test.c || bail "GMP compilation failed"
./gmp_test | grep 42 >/dev/null && ok "GMP compilation and run succeeded" || bail "GMP test did not print 42"
rm -f gmp_test.c gmp_test

# 6. Test compilation of FLINT linkage
msg "Testing FLINT + GMP + MPFR linkage..."
cat > flint_test.c <<EOF
#include <flint/flint.h>
#include <flint/fmpz.h>
#include <stdio.h>
int main() {
    fmpz_t a, b, s;
    fmpz_init(a); fmpz_init(b); fmpz_init(s);
    fmpz_set_ui(a, 5); fmpz_set_ui(b, 7);
    fmpz_add(s, a, b);
    printf("Sum: "); fmpz_print(s); printf("\\n");
    fmpz_clear(a); fmpz_clear(b); fmpz_clear(s);
    return 0;
}
EOF

gcc -I"$INCDIR" -L"$LIBDIR" -lflint -lgmp -lmpfr -o flint_test flint_test.c || bail "FLINT compilation failed"
./flint_test | grep 12 >/dev/null && ok "FLINT test program compiled and ran: sum is 12." || warn "FLINT test ran, but did not find expected result."
rm -f flint_test.c flint_test

ok "AUTOMATION FINISHED. Your FLINT/GMP/MPFR installation is now fixed and ready."

# Show example for compiling your own code
echo; msg "To compile your own FLINT C99 programs, use:"
echo "gcc -I$INCDIR -L$LIBDIR -lflint -lgmp -lmpfr -o myprog myprog.c"
echo
