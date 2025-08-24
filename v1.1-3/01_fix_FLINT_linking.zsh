#!/bin/zsh

#=============================================================================
# FLINT Installation and Compilation Fix Script for macOS ARM64
# Created: August 20, 2025
# Purpose: Automatically fix FLINT linking errors with multiple fallback approaches
#=============================================================================

set -e  # Exit on error

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if command succeeded
check_success() {
    if [[ $? -ne 0 ]]; then
        print_error "$1 failed"
        return 1
    fi
    return 0
}

print_status "Starting FLINT installation and compilation fix..."

#=============================================================================
# STEP 1: System Prerequisites Check
#=============================================================================

print_status "Checking system prerequisites..."

# Check if we're on macOS ARM64
if [[ $(uname -m) != "arm64" ]]; then
    print_warning "This script is optimized for ARM64 (Apple Silicon). Your architecture: $(uname -m)"
fi

# Check if Xcode Command Line Tools are installed
if ! xcode-select -p &> /dev/null; then
    print_status "Installing Xcode Command Line Tools..."
    xcode-select --install
    print_status "Please complete the Xcode Command Line Tools installation and re-run this script."
    exit 0
fi

# Check if Homebrew is installed
if ! command -v brew &> /dev/null; then
    print_status "Homebrew not found. Installing Homebrew..."
    /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
    
    # Add Homebrew to PATH for ARM64 Macs
    if [[ -f /opt/homebrew/bin/brew ]]; then
        eval "$(/opt/homebrew/bin/brew shellenv)"
    fi
    
    if check_success "Homebrew installation"; then
        print_success "Homebrew installed successfully"
    else
        print_error "Failed to install Homebrew. Please install manually."
        exit 1
    fi
else
    print_success "Homebrew is already installed"
fi

#=============================================================================
# STEP 2: Install Dependencies via Homebrew (Primary Approach)
#=============================================================================

print_status "Installing FLINT and dependencies via Homebrew..."

# Update Homebrew first
brew update

# Install dependencies
if brew install flint gmp mpfr 2>/dev/null; then
    print_success "FLINT, GMP, and MPFR installed via Homebrew"
    HOMEBREW_SUCCESS=true
else
    print_warning "Homebrew installation failed, will try alternative approaches"
    HOMEBREW_SUCCESS=false
fi

#=============================================================================
# STEP 3: Set up compilation paths
#=============================================================================

print_status "Setting up compilation paths..."

# Try multiple path configurations
PATHS_TO_TRY=(
    "$(brew --prefix)/include:$(brew --prefix)/lib"
    "$(brew --prefix flint)/include:$(brew --prefix flint)/lib"
    "/opt/homebrew/include:/opt/homebrew/lib"
    "/usr/local/include:/usr/local/lib"
)

WORKING_INCLUDE=""
WORKING_LIB=""

for path_pair in "${PATHS_TO_TRY[@]}"; do
    IFS=':' read -r include_path lib_path <<< "$path_pair"
    
    if [[ -d "$include_path" && -d "$lib_path" ]]; then
        # Check if FLINT headers exist
        if [[ -f "$include_path/flint/flint.h" ]]; then
            WORKING_INCLUDE="$include_path"
            WORKING_LIB="$lib_path"
            print_success "Found FLINT at: $include_path"
            break
        fi
    fi
done

if [[ -z "$WORKING_INCLUDE" ]]; then
    print_warning "Could not find FLINT headers in standard locations"
fi

#=============================================================================
# STEP 4: Create Test Program
#=============================================================================

print_status "Creating test program..."

cat > test_flint_fix.c << 'EOF'
#include <flint/flint.h>
#include <flint/fmpz.h>
#include <stdio.h>

int fmpz_add_modern(fmpz_t result, const fmpz_t a, const fmpz_t b) {
    fmpz_add(result, a, b);
    return 0;
}

int main() {
    fmpz_t a, b, sum;
    
    fmpz_init(a);
    fmpz_init(b);
    fmpz_init(sum);
    
    fmpz_set_str(a, "123456789012345678901234567890", 10);
    fmpz_set_str(b, "987654321098765432109876543210", 10);
    
    int result = fmpz_add_modern(sum, a, b);
    
    printf("Addition test result: ");
    fmpz_print(sum);
    printf("\nStatus: %d\n", result);
    printf("Test completed successfully!\n");
    
    fmpz_clear(a);
    fmpz_clear(b);
    fmpz_clear(sum);
    
    return 0;
}
EOF

#=============================================================================
# STEP 5: Try Multiple Compilation Approaches
#=============================================================================

print_status "Attempting compilation with multiple approaches..."

COMPILATION_SUCCESS=false

# Approach 1: pkg-config (if available)
if command -v pkg-config &> /dev/null && pkg-config --exists flint 2>/dev/null; then
    print_status "Trying compilation with pkg-config..."
    PKG_CFLAGS=$(pkg-config --cflags flint)
    PKG_LIBS=$(pkg-config --libs flint)
    
    if gcc $PKG_CFLAGS $PKG_LIBS -o test_flint_fix test_flint_fix.c 2>/dev/null; then
        print_success "Compilation succeeded using pkg-config!"
        COMPILATION_SUCCESS=true
        SUCCESSFUL_METHOD="pkg-config"
        SUCCESSFUL_FLAGS="$PKG_CFLAGS $PKG_LIBS"
    fi
fi

# Approach 2: Direct paths (if pkg-config failed)
if [[ "$COMPILATION_SUCCESS" == "false" && -n "$WORKING_INCLUDE" ]]; then
    print_status "Trying compilation with direct paths..."
    DIRECT_FLAGS="-I$WORKING_INCLUDE -L$WORKING_LIB -lflint -lgmp -lmpfr"
    
    if gcc $DIRECT_FLAGS -o test_flint_fix test_flint_fix.c 2>/dev/null; then
        print_success "Compilation succeeded using direct paths!"
        COMPILATION_SUCCESS=true
        SUCCESSFUL_METHOD="direct paths"
        SUCCESSFUL_FLAGS="$DIRECT_FLAGS"
    fi
fi

# Approach 3: Homebrew prefix method
if [[ "$COMPILATION_SUCCESS" == "false" ]]; then
    print_status "Trying compilation with Homebrew prefix method..."
    if command -v brew &> /dev/null; then
        BREW_FLAGS="-I$(brew --prefix)/include -L$(brew --prefix)/lib -lflint -lgmp -lmpfr"
        
        if gcc $BREW_FLAGS -o test_flint_fix test_flint_fix.c 2>/dev/null; then
            print_success "Compilation succeeded using Homebrew prefix!"
            COMPILATION_SUCCESS=true
            SUCCESSFUL_METHOD="homebrew prefix"
            SUCCESSFUL_FLAGS="$BREW_FLAGS"
        fi
    fi
fi

# Approach 4: Build from source (last resort)
if [[ "$COMPILATION_SUCCESS" == "false" ]]; then
    print_warning "All standard approaches failed. Attempting to build FLINT from source..."
    
    FLINT_BUILD_DIR="/tmp/flint_build_$$"
    mkdir -p "$FLINT_BUILD_DIR"
    cd "$FLINT_BUILD_DIR"
    
    if git clone https://github.com/flintlib/flint.git 2>/dev/null; then
        cd flint
        if ./bootstrap.sh && ./configure --prefix=/usr/local && make -j$(sysctl -n hw.ncpu) && sudo make install; then
            print_success "FLINT built and installed from source!"
            
            # Try compilation again
            SOURCE_FLAGS="-I/usr/local/include -L/usr/local/lib -lflint -lgmp -lmpfr"
            cd - > /dev/null
            
            if gcc $SOURCE_FLAGS -o test_flint_fix test_flint_fix.c 2>/dev/null; then
                print_success "Compilation succeeded after building from source!"
                COMPILATION_SUCCESS=true
                SUCCESSFUL_METHOD="built from source"
                SUCCESSFUL_FLAGS="$SOURCE_FLAGS"
            fi
        fi
    fi
    
    # Cleanup build directory
    rm -rf "$FLINT_BUILD_DIR"
fi

#=============================================================================
# STEP 6: Test and Provide Results
#=============================================================================

if [[ "$COMPILATION_SUCCESS" == "true" ]]; then
    print_success "FLINT compilation fix completed!"
    print_status "Running test program..."
    
    if ./test_flint_fix; then
        print_success "Test program executed successfully!"
    else
        print_warning "Test program compiled but failed to run"
    fi
    
    echo ""
    print_success "=== SOLUTION FOUND ==="
    echo "Method: $SUCCESSFUL_METHOD"
    echo "Compilation flags: $SUCCESSFUL_FLAGS"
    echo ""
    print_status "To compile your programs in the future, use:"
    echo "gcc $SUCCESSFUL_FLAGS -o your_program your_program.c"
    
    # Create a helper script for future compilations
    cat > compile_with_flint.sh << EOF
#!/bin/zsh
# Auto-generated compilation helper for FLINT programs
# Usage: ./compile_with_flint.sh source_file.c output_name

if [[ \$# -lt 2 ]]; then
    echo "Usage: \$0 <source_file.c> <output_name>"
    exit 1
fi

gcc $SUCCESSFUL_FLAGS -o "\$2" "\$1"
EOF
    chmod +x compile_with_flint.sh
    print_success "Created compile_with_flint.sh helper script for future use"
    
else
    print_error "=== ALL APPROACHES FAILED ==="
    echo ""
    print_status "Manual troubleshooting steps:"
    echo "1. Verify your macOS version and architecture: $(sw_vers -productVersion) $(uname -m)"
    echo "2. Check if Apple Silicon Rosetta is needed: softwareupdate --install-rosetta"
    echo "3. Manually install dependencies: brew install gmp mpfr"
    echo "4. Check library paths: ls -la /opt/homebrew/lib/*flint*"
    echo "5. Verify compiler: which gcc && gcc --version"
    echo ""
    print_error "You may need to install FLINT manually or check for architecture compatibility issues."
fi

#=============================================================================
# STEP 7: Cleanup
#=============================================================================

print_status "Cleaning up temporary files..."
rm -f test_flint_fix.c test_flint_fix

print_status "Script completed."
