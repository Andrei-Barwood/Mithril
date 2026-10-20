# 튜orial: Integración IoT y Telemetría de Energía Mareomotriz

Este tutorial demuestra cómo conectar nodos de sensores en el borde (*edge nodes*) y turbinas marinas de generación mareomotriz utilizando los protocolos binarios compactos de Mithril.

---

## 1. Modelo de Telemetría Marina (`TidalMetrics`)

El protocolo [`network/protocols/tidal_sensor_protocol.hpp`](file:///Users/andreibarwood/Documents/snocomm/Mithril/network/protocols/tidal_sensor_protocol.hpp) define un formato de datos altamente optimizado para ambientes marinos hostiles:

```cpp
#include "protocols/tidal_sensor_protocol.hpp"
#include <chrono>
#include <iostream>

using namespace mithril::protocol;

int main() {
    // 1. Clave simétrica de sesión previamente acordada mediante ECDH
    mithril::sodium::SecureKey<32> session_key;
    session_key.randomize();

    // 2. Par de claves de identidad Ed25519 del sensor para firma digital
    mithril::sodium::DigitalSignature sensor_id;

    // 3. Captura de métricas de la turbina submarina
    TidalMetrics metrics;
    metrics.device_id = 0x01; // ID de la turbina
    metrics.timestamp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    metrics.sequence = 1;
    metrics.flow_velocity_mps = 3.42f;    // 3.42 m/s de corriente
    metrics.water_pressure_kpa = 320.5f;  // 320.5 kPa de presión
    metrics.rotor_rpm = 45.2f;            // 45.2 RPM
    metrics.power_output_kw = 850.0f;     // 850 kW generados
    metrics.water_temp_c = 8.5f;          // 8.5 °C
    metrics.status_flags = 0x01;          // Flag de turbina en red normal

    // 4. Empaquetar y cifrar con ChaCha20-Poly1305 + firma Ed25519
    std::vector<uint8_t> frame = TidalProtocol::pack_telemetry(
        metrics, session_key, &sensor_id.secret_key()
    );

    std::cout << "Trama mareomotriz empaquetada (" << frame.size() << " bytes).\n";

    // 5. En el panel de control receptor:
    AntiReplayFilter replay_filter;
    TidalMetrics received_metrics;

    bool ok = TidalProtocol::unpack_telemetry(
        frame, session_key, replay_filter, received_metrics, &sensor_id.public_key()
    );

    if (ok) {
        std::cout << "Telemetria recibida y verificada con exito:\n";
        std::cout << "  Turbina: #" << received_metrics.device_id << "\n";
        std::cout << "  Potencia generada: " << received_metrics.power_output_kw << " kW\n";
        std::cout << "  Velocidad de marea: " << received_metrics.flow_velocity_mps << " m/s\n";
    } else {
        std::cerr << "Fallo en la verificacion de la trama (posible adulteracion o replay).\n";
    }

    return 0;
}
```

---

## 2. Red de Sensores IoT Ambientales (`DeviceTelemetry`)

Para nodos IoT genéricos (temperatura, humedad, estado de batería):

```cpp
#include "protocols/iot_protocol.hpp"
#include <iostream>

using namespace mithril::protocol;

void simulate_iot_node() {
    mithril::sodium::SecureKey<32> session_key;
    session_key.randomize();

    DeviceTelemetry data;
    data.device_id = 42;
    data.timestamp_ms = 1700000000000ULL;
    data.sequence = 100;
    data.temperature = 22.4f;
    data.humidity = 65.0f;
    data.battery_level = 98.5f;
    data.status_flags = 0;

    // Empaquetar y cifrar
    auto frame = IoTProtocol::pack_telemetry(data, session_key);

    // Desempaquetar y validar
    AntiReplayFilter filter;
    DeviceTelemetry out;
    if (IoTProtocol::unpack_telemetry(frame, session_key, filter, out)) {
        std::cout << "Sensor IoT #" << out.device_id << " reporta " << out.temperature << " °C.\n";
    }
}
```
