# 🚀 Guía de Inicio Rápido (Getting Started)

Bienvenido a **Mithril**. Esta guía detalla cómo compilar, probar y poner en marcha las comunicaciones seguras y simulaciones del proyecto.

---

## 1. Requisitos del Sistema

### 1.1. Herramientas de Compilación
* **Compilador C++20:** Clang 15+ (Apple Clang 14+) o GCC 11+.
* **Compilador C:** Soporte para estándar C23 / C17.
* **CMake:** Versión 3.20 o superior.
* **Ninja** o **Make**: Sistema de construcción preferido.
* **Zsh** o **Bash**: Para la ejecución de scripts de pruebas y automatización.

### 1.2. Bibliotecas del Sistema
* **libsodium** (Requerida): Biblioteca de operaciones criptográficas de alto rendimiento.
* **FLINT & GMP** (Opcional): Para habilitar el acelerador aritmético `provider_flint`.
* **Icarus Verilog (`iverilog`, `vvp`) y GTKWave** (Opcional): Para simulación de los módulos ASIC/hardware.

---

## 2. Instalación de Dependencias

### En macOS (Homebrew)
```zsh
brew update
brew install cmake pkg-config ccache libsodium flint gmp
# Opcional para simulación de hardware:
brew install icarus-verilog
brew install --cask gtkwave
```

### En Ubuntu / Debian Linux
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake pkg-config ccache \
    libsodium-dev libflint-dev libgmp-dev zsh
# Opcional para simulación de hardware:
sudo apt-get install -y iverilog gtkwave
```

---

## 3. Clonación y Compilación con CMake Raíz

Mithril utiliza un único sistema CMake raíz unificado:

```zsh
# 1. Clonar el repositorio
git clone https://github.com/Andrei-Barwood/Mithril.git
cd Mithril

# 2. Configurar el directorio de construcción
cmake -S . -B build \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DMITHRIL_ENABLE_TESTS=ON \
    -DMITHRIL_USE_LIBSODIUM=ON

# 3. Compilar todas las bibliotecas, pruebas y ejemplos
cmake --build build --parallel
```

Los artefactos resultantes quedarán organizados en:
* `build/Mithril_API_V2/libmithril_v2.a`: Biblioteca estática de la API V2.
* `build/crypto/libmithril_crypto.a`: Biblioteca de transición criptográfica.
* `build/network/libmithril_network.a`: Biblioteca de red C++20.
* `build/tests/`: Ejecutables de pruebas unitarias, integración, seguridad y benchmarks.
* `build/examples/`: Ejecutables de ejemplos básicos, IoT y energía mareomotriz.

---

## 4. Ejecución de la Suite de Pruebas

Para validar el 100% de los componentes del repositorio:

```zsh
# Opción A: Runner integral de pruebas
./scripts/testing/run_all_tests.zsh

# Opción B: Ejecutar solo CTest
ctest --test-dir build --output-on-failure
```

Las 24 pruebas automatizadas cubren:
* Aritmética de enteros grandes y operaciones modulares en C23.
* Generación de números aleatorios, hashing y firmas con libsodium.
* Handshake cliente-servidor asíncrono sobre loopback.
* Resiliencia ante desconexiones forzadas y ataques de repetición.
* Fuzzing mutacional y benchmarks de latencia y caudal de datos.

---

## 5. Ejecución de Ejemplos Prácticos

### 5.1. Servidor y Cliente de Eco Seguro (Básico)
En una terminal:
```zsh
./build/examples/basic/echo_server_sodium
```
En otra terminal:
```zsh
./build/examples/basic/connecting_client_sodium
```

### 5.2. Telemetría Mareomotriz (SCADA Industrial)
En una terminal, inicia el nodo concentrador del panel de control:
```zsh
./build/examples/tidal_energy/control_panel_client
```
En otra terminal, inicia la turbina marina generadora:
```zsh
./build/examples/tidal_energy/tidal_sensor_server
```
Observarás el handshake de clave efímera X25519, la firma digital Ed25519 de cada paquete y el streaming de métricas en tiempo real (velocidad de marea, presión submarina, potencia generada en kW).

### 5.3. Red de Sensores IoT
En una terminal:
```zsh
./build/examples/iot_devices/gateway_server
```
En otra terminal:
```zsh
./build/examples/iot_devices/device_simulator
```

---

## 6. Simulación de Módulos ASIC de Hardware

Para ejecutar la simulación de los módulos de hardware en Verilog:

```zsh
cd hardware
./run_icarus_sim.zsh
```
O usando el Makefile de simulación:
```zsh
make sim-all
```
Para visualizar las ondas de temporización y señales de reloj de la ALU en GTKWave:
```zsh
make wave-alu
```
