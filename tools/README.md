# 🛠️ Mithril Tooling & Archive

Este directorio contiene herramientas auxiliares y de diagnóstico, así como el archivo histórico de scripts de desarrollo del proyecto Mithril.

---

## 📁 Estructura

- **`archive/`**: Colección de scripts y bitácoras históricas desarrolladas durante las fases de investigación e ingeniería de empaquetado de FLINT y Xcode (Agosto y Septiembre de 2024):
  - `01 - 14 August/`: Correcciones iniciales de rutas para la biblioteca FLINT.
  - `02 - 15 August/`: Detección dinámica y linking con bibliotecas Homebrew en macOS.
  - `03 - 17 August/`: Generación de dylibs universales (x86_64 y ARM64) combinando FLINT, GMP y MPFR.
  - `04 - 18 August/`: Empaquetado en formato Apple Framework (`FLINT.framework`).
  - `05 - 20 August/`: Pruebas de enlace estático/dinámico y scripts de validación.
  - `06 - September/`: Gestión de almacenamiento, simuladores y respaldos.

---

> [!NOTE]
> Para herramientas y utilidades de desarrollo activas (scaffolding de componentes, visualización de árbol, runners de pruebas), consulte el directorio [`scripts/`](../scripts).
