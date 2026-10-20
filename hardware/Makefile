# ============================================================================
# Mithril ASIC Implementation - Icarus Verilog Simulation Makefile
# ============================================================================

IVERILOG ?= iverilog
VVP      ?= vvp
GTKWAVE  ?= gtkwave
VPI_CC   ?= iverilog-vpi

IVERILOG_FLAGS := -Wall -g2012 -I rtl

SIM_DIR := sim_build
RTL_DIR := rtl
TB_DIR  := tb
VPI_DIR := vpi

RTL_ALL := $(RTL_DIR)/mithril_add_secure.v \
           $(RTL_DIR)/mithril_sub_secure.v \
           $(RTL_DIR)/mithril_add_mod.v \
           $(RTL_DIR)/mithril_karatsuba_mul.v \
           $(RTL_DIR)/mithril_div_rem.v \
           $(RTL_DIR)/mithril_crypto_alu_top.v

.PHONY: all help sim-all sim-add sim-sub sim-mod sim-mul sim-div sim-alu vpi clean check-tools

all: sim-all

help:
	@echo "Mithril ASIC - Icarus Verilog Simulation Suite"
	@echo "--------------------------------------------------------"
	@echo "Targets:"
	@echo "  make sim-all      -> Run all ASIC simulations"
	@echo "  make sim-add      -> 256-bit Secure Adder simulation"
	@echo "  make sim-sub      -> 256-bit Secure Subtractor simulation"
	@echo "  make sim-mod      -> Curve25519 Modular Adder simulation"
	@echo "  make sim-mul      -> Karatsuba Multiplier simulation"
	@echo "  make sim-div      -> 64-bit Division/Remainder simulation"
	@echo "  make sim-alu      -> Top Cryptographic ALU simulation"
	@echo "  make wave-add     -> View Adder waveform in GTKWave"
	@echo "  make wave-alu     -> View ALU waveform in GTKWave"
	@echo "  make vpi          -> Build libsodium VPI shared module"
	@echo "  make clean        -> Remove simulation outputs"

check-tools:
	@command -v $(IVERILOG) >/dev/null 2>&1 || { \
		echo "ERROR: Icarus Verilog ('$(IVERILOG)') not found!"; \
		echo "Install with Homebrew: brew install icarus-verilog"; \
		echo "Or on Ubuntu/Debian:  sudo apt-get install iverilog"; \
		exit 1; \
	}
	@command -v $(VVP) >/dev/null 2>&1 || { \
		echo "ERROR: VVP runtime ('$(VVP)') not found!"; \
		exit 1; \
	}

$(SIM_DIR):
	@mkdir -p $(SIM_DIR)

# ----------------------------------------------------------------------------
# Individual Simulations
# ----------------------------------------------------------------------------

sim-add: check-tools $(SIM_DIR)
	@echo "==> Compiling 256-bit Secure Adder..."
	@$(IVERILOG) $(IVERILOG_FLAGS) -o $(SIM_DIR)/tb_add.vvp \
		$(RTL_DIR)/mithril_add_secure.v $(TB_DIR)/tb_mithril_add_secure.v
	@echo "==> Running simulation..."
	@cd $(SIM_DIR) && $(VVP) tb_add.vvp

sim-sub: check-tools $(SIM_DIR)
	@echo "==> Compiling 256-bit Secure Subtractor..."
	@$(IVERILOG) $(IVERILOG_FLAGS) -o $(SIM_DIR)/tb_sub.vvp \
		$(RTL_DIR)/mithril_sub_secure.v $(TB_DIR)/tb_mithril_sub_secure.v
	@echo "==> Running simulation..."
	@cd $(SIM_DIR) && $(VVP) tb_sub.vvp

sim-mod: check-tools $(SIM_DIR)
	@echo "==> Compiling Curve25519 Modular Adder..."
	@$(IVERILOG) $(IVERILOG_FLAGS) -o $(SIM_DIR)/tb_mod.vvp \
		$(RTL_DIR)/mithril_add_mod.v $(TB_DIR)/tb_mithril_add_mod.v
	@echo "==> Running simulation..."
	@cd $(SIM_DIR) && $(VVP) tb_mod.vvp

sim-mul: check-tools $(SIM_DIR)
	@echo "==> Compiling Karatsuba Multiplier..."
	@$(IVERILOG) $(IVERILOG_FLAGS) -o $(SIM_DIR)/tb_mul.vvp \
		$(RTL_DIR)/mithril_karatsuba_mul.v $(TB_DIR)/tb_mithril_karatsuba_mul.v
	@echo "==> Running simulation..."
	@cd $(SIM_DIR) && $(VVP) tb_mul.vvp

sim-div: check-tools $(SIM_DIR)
	@echo "==> Compiling Division & Remainder..."
	@$(IVERILOG) $(IVERILOG_FLAGS) -o $(SIM_DIR)/tb_div.vvp \
		$(RTL_DIR)/mithril_div_rem.v $(TB_DIR)/tb_mithril_div_rem.v
	@echo "==> Running simulation..."
	@cd $(SIM_DIR) && $(VVP) tb_div.vvp

sim-alu: check-tools $(SIM_DIR)
	@echo "==> Compiling Top-level Cryptographic ALU..."
	@$(IVERILOG) $(IVERILOG_FLAGS) -o $(SIM_DIR)/tb_alu.vvp \
		$(RTL_ALL) $(TB_DIR)/tb_mithril_crypto_alu_top.v
	@echo "==> Running simulation..."
	@cd $(SIM_DIR) && $(VVP) tb_alu.vvp

sim-all: sim-add sim-sub sim-mod sim-mul sim-div sim-alu
	@echo ""
	@echo "========================================================"
	@echo " All Mithril ASIC Icarus Simulations Finished!"
	@echo " Waveforms available in: $(SIM_DIR)/*.vcd"
	@echo "========================================================"

# ----------------------------------------------------------------------------
# Waveform Viewing
# ----------------------------------------------------------------------------

wave-add: sim-add
	@command -v $(GTKWAVE) >/dev/null 2>&1 && $(GTKWAVE) $(SIM_DIR)/mithril_add_secure.vcd & || \
		echo "Install gtkwave: brew install --cask gtkwave"

wave-alu: sim-alu
	@command -v $(GTKWAVE) >/dev/null 2>&1 && $(GTKWAVE) $(SIM_DIR)/mithril_crypto_alu_top.vcd & || \
		echo "Install gtkwave: brew install --cask gtkwave"

# ----------------------------------------------------------------------------
# Optional VPI Co-simulation module
# ----------------------------------------------------------------------------

vpi: $(SIM_DIR)
	@command -v $(VPI_CC) >/dev/null 2>&1 || { \
		echo "iverilog-vpi required for VPI build"; \
		exit 1; \
	}
	@echo "==> Building Mithril VPI co-simulation module..."
	@$(VPI_CC) --name=mithril_vpi $(VPI_DIR)/mithril_vpi.c -lsodium
	@mv mithril_vpi.vpi $(SIM_DIR)/

clean:
	@rm -rf $(SIM_DIR) *.vcd *.vvp *.vpi *.o
	@echo "Cleaned simulation artifacts."
