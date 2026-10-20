# ⚡ Mithril ASIC Implementation — Icarus Verilog Simulation Suite

> Especificación de módulos de hardware sintetizables (RTL) y banco de pruebas para simulación de circuitos integrados específicos de aplicación (ASIC) y FPGA mediante **Icarus Verilog** (`iverilog` + `vvp`) y visualización de ondas en **GTKWave**.

---

## 🧭 Estructura de Directorios

```text
ASIC implementation/
├── Makefile                     # Driver de compilación y simulación para Icarus Verilog
├── run_icarus_sim.zsh           # Script ejecutable de simulación automatizada
├── rtl/                         # Módulos Verilog sintetizables (RTL)
│   ├── mithril_add_secure.v     # Sumador seguro de 256 bits (tiempo constante)
│   ├── mithril_sub_secure.v     # Restador seguro de 256 bits con flag de underflow
│   ├── mithril_add_mod.v        # Sumador modular Curve25519 ((a + b) mod (2^255 - 19))
│   ├── mithril_karatsuba_mul.v  # Multiplicador Karatsuba 32x32 -> 64 bits segmentado
│   ├── mithril_div_rem.v        # Divisor y residuo de 64 bits con trampa por cero
│   └── mithril_crypto_alu_top.v # ALU criptográfica unificada de nivel superior
├── tb/                          # Bancos de pruebas (Testbenches) para Icarus Verilog
│   ├── tb_mithril_add_secure.v  # Testbench del sumador con volcado .vcd y aserciones
│   ├── tb_mithril_sub_secure.v  # Testbench del restador con casos de underflow
│   ├── tb_mithril_add_mod.v     # Testbench de reducción modular Curve25519
│   ├── tb_mithril_karatsuba_mul.v # Testbench de multiplicación Karatsuba
│   ├── tb_mithril_div_rem.v     # Testbench de división y residuo
│   └── tb_mithril_crypto_alu_top.v # Testbench de integración completa de la ALU
└── vpi/                         # Módulo VPI C para co-simulación con libsodium
    └── mithril_vpi.c            # Enlace PLI 2.0 / VPI para Icarus Verilog
```

---

## 🛠 Requisitos de Simulación

Para ejecutar las simulaciones se requiere **Icarus Verilog** y opcionalmente **GTKWave** para ver formas de onda:

### macOS (Homebrew)
```zsh
brew install icarus-verilog
brew install --cask gtkwave
```

### Ubuntu / Debian Linux
```bash
sudo apt-get update
sudo apt-get install -y iverilog gtkwave
```

---

## 🚀 Cómo Ejecutar las Simulaciones

### 1. Ejecutar Todas las Simulaciones con Makefile
```zsh
cd "ASIC implementation"
make sim-all
```

O usando el script Zsh:
```zsh
./run_icarus_sim.zsh
```

### 2. Ejecutar Módulos Individuales
```zsh
make sim-add   # Simula el sumador de 256 bits en tiempo constante
make sim-sub   # Simula el restador de 256 bits
make sim-mod   # Simula la adición modular Curve25519
make sim-mul   # Simula el multiplicador Karatsuba
make sim-div   # Simula la división con residuo
make sim-alu   # Simula la ALU criptográfica completa
```

### 3. Visualizar Formas de Onda (GTKWave)
Cada testbench genera automáticamente un archivo `.vcd` en `sim_build/`:
```zsh
make wave-add  # Abre la forma de onda de mithril_add_secure.vcd en GTKWave
make wave-alu  # Abre la forma de onda de mithril_crypto_alu_top.vcd en GTKWave
```

---

## 📐 Tabla de Opcodes de la ALU Criptográfica (`mithril_crypto_alu_top`)

| Opcode (`[2:0]`) | Operación | Ancho Operando | Latencia (Ciclos) | Salida Primaria | Flags |
| :---: | :--- | :---: | :---: | :---: | :---: |
| `3'b000` | `OP_ADD_SECURE` | 256 bits | 16 ciclos | `result_256` | `flag_carry_underflow` |
| `3'b001` | `OP_SUB_SECURE` | 256 bits | 16 ciclos | `result_256` | `flag_carry_underflow` |
| `3'b010` | `OP_ADD_MOD` (Curve25519) | 256 bits | 16 ciclos | `result_256` | `error` |
| `3'b011` | `OP_KARATSUBA` | 32 bits | 9 ciclos | `result_64` | `error` |
| `3'b100` | `OP_DIV_REM` | 64 bits | 65 ciclos | `result_64`, `remainder_64` | `flag_div_by_zero` |

---

## 🔒 Propiedades de Seguridad de Hardware para ASIC

1. **Tiempo Constante (Constant-Time Execution):**  
   Todas las operaciones aritméticas clave ejecutan una máquina de estados con número fijo de ciclos de reloj, eliminando variaciones de tiempo dependientes de los datos para mitigar ataques de canal lateral (*timing attacks*).
2. **Sanitización de Registros (Cleanup Phase):**  
   Tras completar el cálculo y antes de emitir la señal `done`, una fase de limpieza (`CLEANUP`) sobreescribe con ceros todos los registros temporales internos que contenían datos criptográficos sensibles.
3. **Detección de Subdesbordamiento y División por Cero:**  
   Señales explícitas de estado para control seguro en sistemas embebidos de energía crítica sin bloqueos de bus.
