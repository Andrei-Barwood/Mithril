#!/usr/bin/env zsh

# ============================================================================
# Mithril - Project Structure Visualizer
# ============================================================================

set -euo pipefail

readonly CYAN='\033[0;36m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly NC='\033[0m'

print_tree() {
    echo "${CYAN}Mithril Project Structure:${NC}"
    echo ""
    
    if command -v tree &> /dev/null; then
        tree -L 3 -I 'build|.git|*.xcworkspace|xcuserdata' --dirsfirst
    else
        echo "${YELLOW}Install 'tree' for better visualization: brew install tree${NC}"
        find . -type d -not -path '*/\.*' -not -path '*/build/*' | \
            sed 's|[^/]*/| |g' | \
            head -50
    fi
}

print_stats() {
    echo ""
    echo "${GREEN}Statistics:${NC}"
    echo "  Directories: $(find . -type d -not -path '*/\.*' | wc -l | tr -d ' ')"
    echo "  C++ Files:   $(find . -name "*.cpp" -o -name "*.hpp" | wc -l | tr -d ' ')"
    echo "  C Files:     $(find . -name "*.c" -o -name "*.h" | wc -l | tr -d ' ')"
    echo "  Tests:       $(find tests -name "*.cpp" 2>/dev/null | wc -l | tr -d ' ')"
    echo "  Examples:    $(find examples -name "*.cpp" 2>/dev/null | wc -l | tr -d ' ')"
}

main() {
    print_tree
    print_stats
}

main "$@"
