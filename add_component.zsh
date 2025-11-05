#!/usr/bin/env zsh

# ============================================================================
# Mithril - Add New Component
# ============================================================================
# Usage: ./add_component.zsh <type> <name>
# Example: ./add_component.zsh example "tcp_server"
# ============================================================================

set -euo pipefail

readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly CYAN='\033[0;36m'
readonly NC='\033[0m'

show_usage() {
    echo "Usage: $0 <type> <name>"
    echo ""
    echo "Types:"
    echo "  example     - Add new example (creates .cpp in examples/)"
    echo "  test        - Add new test (creates .cpp in tests/unit/)"
    echo "  header      - Add new header (creates .hpp in network/include/mithril/)"
    echo "  protocol    - Add new protocol (creates .hpp in network/protocols/)"
    echo "  doc         - Add new documentation (creates .md in docs/)"
    echo ""
    echo "Example:"
    echo "  $0 example tcp_streaming_server"
    exit 1
}

add_example() {
    local name=$1
    local file="examples/basic/${name}.cpp"
    
    mkdir -p "examples/basic"
    
    cat > "$file" << 'EOF'
#include "mithril/mithril_sodium.hpp"
#include <boost/asio.hpp>
#include <iostream>

using namespace mithril::sodium;
namespace asio = boost::asio;

// TODO: Implement your example here

int main() {
    try {
        asio::io_context ios;
        
        // Your code here
        
        ios.run();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}
EOF
    
    echo "${GREEN}✓${NC} Created example: ${file}"
    echo "${CYAN}Next:${NC} Add to examples/CMakeLists.txt"
}

add_test() {
    local name=$1
    local file="tests/unit/${name}.cpp"
    
    mkdir -p "tests/unit"
    
    cat > "$file" << 'EOF'
#include <catch2/catch_test_macros.hpp>
#include "mithril/mithril_sodium.hpp"

using namespace mithril::sodium;

TEST_CASE("Description of test", "[tag]") {
    // TODO: Implement test
    REQUIRE(true);
}
EOF
    
    echo "${GREEN}✓${NC} Created test: ${file}"
    echo "${CYAN}Next:${NC} Add to tests/CMakeLists.txt"
}

add_header() {
    local name=$1
    local file="network/include/mithril/${name}.hpp"
    
    mkdir -p "network/include/mithril"
    
    local guard=$(echo "${name}" | tr '[:lower:]' '[:upper:]' | tr '-' '_')
    
    cat > "$file" << EOF
#pragma once

#ifndef MITHRIL_${guard}_HPP
#define MITHRIL_${guard}_HPP

#include <boost/asio.hpp>
#include <sodium.h>

namespace mithril::sodium {

// TODO: Add your declarations here

} // namespace mithril::sodium

#endif // MITHRIL_${guard}_HPP
EOF
    
    echo "${GREEN}✓${NC} Created header: ${file}"
}

add_protocol() {
    local name=$1
    local file="network/protocols/${name}_protocol.hpp"
    
    mkdir -p "network/protocols"
    
    cat > "$file" << 'EOF'
#pragma once

#include <cstdint>
#include <vector>
#include <span>

namespace mithril::protocol {

// Protocol message types
enum class MessageType : uint16_t {
    // TODO: Define message types
};

// Protocol structures
struct Message {
    // TODO: Define message structure
};

} // namespace mithril::protocol
EOF
    
    echo "${GREEN}✓${NC} Created protocol: ${file}"
}

add_doc() {
    local name=$1
    local file="docs/${name}.md"
    
    mkdir -p "docs"
    
    cat > "$file" << EOF
# ${name}

## Overview

TODO: Add overview

## Usage

TODO: Add usage examples

## API Reference

TODO: Add API documentation
EOF
    
    echo "${GREEN}✓${NC} Created documentation: ${file}"
}

main() {
    if [[ $# -lt 2 ]]; then
        show_usage
    fi
    
    local type=$1
    local name=$2
    
    case "$type" in
        example)
            add_example "$name"
            ;;
        test)
            add_test "$name"
            ;;
        header)
            add_header "$name"
            ;;
        protocol)
            add_protocol "$name"
            ;;
        doc)
            add_doc "$name"
            ;;
        *)
            echo "${YELLOW}Unknown type: ${type}${NC}"
            show_usage
            ;;
    esac
    
    # Update state file
    if [[ -f ".mithril_project_state.json" ]]; then
        local timestamp=$(date +"%Y-%m-%d %H:%M:%S")
        # Update last_modified in state file (would need jq for proper JSON handling)
        echo "# Last modified: ${timestamp}" >> .mithril_project_state.json.log
    fi
}

main "$@"
