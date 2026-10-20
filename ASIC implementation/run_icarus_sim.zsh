#!/usr/bin/env zsh
# ============================================================================
# Mithril ASIC - Icarus Verilog Simulation Runner
# ============================================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "${SCRIPT_DIR}"

readonly GREEN='\033[0;32m'
readonly RED='\033[0;31m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly BOLD='\033[1m'
readonly NC='\033[0m'

echo -e "${BOLD}${BLUE}==========================================================${NC}"
echo -e "${BOLD}${BLUE}   Mithril ASIC - Icarus Verilog Simulation Runner        ${NC}"
echo -e "${BOLD}${BLUE}==========================================================${NC}"

# Check for iverilog and vvp
if ! command -v iverilog >/dev/null 2>&1; then
    echo -e "${RED}[ERROR] Icarus Verilog ('iverilog') is not installed on this system.${NC}"
    echo ""
    echo "To install on macOS via Homebrew:"
    echo "    brew install icarus-verilog"
    echo ""
    echo "To install on Debian/Ubuntu Linux:"
    echo "    sudo apt-get update && sudo apt-get install -y iverilog"
    echo ""
    echo "To install on Fedora/RHEL:"
    echo "    sudo dnf install iverilog"
    exit 1
fi

if ! command -v vvp >/dev/null 2>&1; then
    echo -e "${RED}[ERROR] VVP runtime ('vvp') is not installed or not in PATH.${NC}"
    exit 1
fi

SIM_BUILD="sim_build"
mkdir -p "${SIM_BUILD}"

modules=(
    "mithril_add_secure:tb_mithril_add_secure"
    "mithril_sub_secure:tb_mithril_sub_secure"
    "mithril_add_mod:tb_mithril_add_mod"
    "mithril_karatsuba_mul:tb_mithril_karatsuba_mul"
    "mithril_div_rem:tb_mithril_div_rem"
)

passed=0
failed=0

for item in "${modules[@]}"; do
    rtl="${item%%:*}"
    tb="${item##*:}"

    echo -e "\n${BOLD}==> Testing [${rtl}] with testbench [${tb}]...${NC}"
    vvp_file="${SIM_BUILD}/${tb}.vvp"

    iverilog -Wall -g2012 -I rtl -o "${vvp_file}" "rtl/${rtl}.v" "tb/${tb}.v"

    if (cd "${SIM_BUILD}" && vvp "${tb}.vvp"); then
        echo -e "${GREEN}✓ [PASS] ${rtl} simulation completed successfully.${NC}"
        ((passed++))
    else
        echo -e "${RED}✗ [FAIL] ${rtl} simulation failed.${NC}"
        ((failed++))
    fi
done

# Top level ALU
echo -e "\n${BOLD}==> Testing Top-Level Cryptographic ALU [mithril_crypto_alu_top]...${NC}"
vvp_alu="${SIM_BUILD}/tb_mithril_crypto_alu_top.vvp"
iverilog -Wall -g2012 -I rtl -o "${vvp_alu}" \
    rtl/mithril_add_secure.v \
    rtl/mithril_sub_secure.v \
    rtl/mithril_add_mod.v \
    rtl/mithril_karatsuba_mul.v \
    rtl/mithril_div_rem.v \
    rtl/mithril_crypto_alu_top.v \
    tb/tb_mithril_crypto_alu_top.v

if (cd "${SIM_BUILD}" && vvp "tb_mithril_crypto_alu_top.vvp"); then
    echo -e "${GREEN}✓ [PASS] Top-Level ALU simulation completed successfully.${NC}"
    ((passed++))
else
    echo -e "${RED}✗ [FAIL] Top-Level ALU simulation failed.${NC}"
    ((failed++))
fi

echo -e "\n${BOLD}${BLUE}==========================================================${NC}"
echo -e "${BOLD}Simulation Summary: ${GREEN}${passed} Passed${NC}, ${RED}${failed} Failed${NC}"
echo -e "Waveforms (.vcd) generated in: ${SIM_BUILD}/"
echo -e "${BOLD}${BLUE}==========================================================${NC}"

if [ "${failed}" -eq 0 ]; then
    echo -e "${GREEN}${BOLD}ALL ASIC SIMULATIONS PASSED!${NC}"
    exit 0
else
    echo -e "${RED}${BOLD}SOME SIMULATIONS FAILED!${NC}"
    exit 1
fi
