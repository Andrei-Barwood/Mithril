# 📚 Mithril API Reference

Esta documentación cubre la especificación técnica completa de la **API C de Mithril API V2** y la **Capa de Abstracción C++20 (`mithril::sodium`)**.

---

## 1. API C de Bajo Nivel (`Mithril_API_V2`)

Archivo de cabecera: [`Mithril_API_V2/include/mithril/mithril_crypto.h`](file:///Users/andreibarwood/Documents/snocomm/Mithril/Mithril_API_V2/include/mithril/mithril_crypto.h)

### 1.1. Inicialización y Gestión de Proveedores

```c
mithril_status mithril_init(void);
mithril_status mithril_shutdown(void);
mithril_status mithril_provider_activate(const char* provider_name);
const char* mithril_status_string(mithril_status status);
```

* **`mithril_init()`**: Inicializa el runtime criptográfico, proveedores activos y el subsistema de números pseudoaleatorios seguros. Retorna `MITHRIL_SUCCESS` (0) si la operación fue exitosa.
* **`mithril_shutdown()`**: Libera recursos globales y sanitiza contextos de memoria internos.
* **`mithril_provider_activate(const char* name)`**: Activa dinámicamente un proveedor registrado (`"c23"`, `"flint"`, etc.).

### 1.2. Generación de Números Aleatorios Criptográficos (RNG)

```c
mithril_status mithril_rng_fill(void* buf, size_t len);
```
Llena el búfer especificado con `len` bytes criptográficamente seguros provenientes de la fuente de entropía del sistema operativo.

### 1.3. Funciones Hash Criptográficas

```c
mithril_status mithril_hash_compute(
    const void* in, size_t in_len,
    void* out, size_t out_len
);
```
Calcula el digest criptográfico (SHA-256 o BLAKE2b según la configuración). `out_len` típico: 32 bytes (`MITHRIL_HASH_256_BYTES`).

### 1.4. Intercambio de Claves (Diffie-Hellman / X25519)

```c
mithril_status mithril_kex_keypair(
    uint8_t pk[MITHRIL_KEX_PUBLICKEY_BYTES],
    uint8_t sk[MITHRIL_KEX_SECRETKEY_BYTES]
);

mithril_status mithril_kex_shared_secret(
    const uint8_t their_pk[MITHRIL_KEX_PUBLICKEY_BYTES],
    const uint8_t my_sk[MITHRIL_KEX_SECRETKEY_BYTES],
    uint8_t shared_secret[MITHRIL_KEX_SHAREDSECRET_BYTES]
);
```

### 1.5. Cifrado Autenticado con Datos Asociados (AEAD ChaCha20-Poly1305)

```c
mithril_status mithril_aead_encrypt(
    const uint8_t* msg, size_t msg_len,
    const uint8_t* ad, size_t ad_len,
    const uint8_t nonce[MITHRIL_AEAD_NONCE_BYTES],
    const uint8_t key[MITHRIL_AEAD_KEY_BYTES],
    uint8_t* out_ciphertext,
    size_t* out_ciphertext_len
);

mithril_status mithril_aead_decrypt(
    const uint8_t* ciphertext, size_t ciphertext_len,
    const uint8_t* ad, size_t ad_len,
    const uint8_t nonce[MITHRIL_AEAD_NONCE_BYTES],
    const uint8_t key[MITHRIL_AEAD_KEY_BYTES],
    uint8_t* out_msg,
    size_t* out_msg_len
);
```

### 1.6. Firmas Digitales Asimétricas (Ed25519)

```c
mithril_status mithril_sign_keypair(
    uint8_t pk[MITHRIL_SIGN_PUBLICKEY_BYTES],
    uint8_t sk[MITHRIL_SIGN_SECRETKEY_BYTES]
);

mithril_status mithril_sign_detached(
    const uint8_t* msg, size_t msg_len,
    const uint8_t sk[MITHRIL_SIGN_SECRETKEY_BYTES],
    uint8_t sig[MITHRIL_SIGN_SIGNATURE_BYTES]
);

mithril_status mithril_sign_verify_detached(
    const uint8_t* msg, size_t msg_len,
    const uint8_t sig[MITHRIL_SIGN_SIGNATURE_BYTES],
    const uint8_t pk[MITHRIL_SIGN_PUBLICKEY_BYTES]
);
```

---

## 2. API C++20 (`mithril::sodium`)

Cabecera principal: [`network/include/mithril/mithril_sodium.hpp`](file:///Users/andreibarwood/Documents/snocomm/Mithril/network/include/mithril/mithril_sodium.hpp)

### 2.1. Contenedor de Clave Segura (`SecureKey<N>`)
Plantilla para el almacenamiento de claves criptográficas de $N$ bytes en memoria protegida.
* **Constructor por defecto:** Llena el búfer con ceros.
* **`randomize()`**: Llena la clave con entropía criptográfica segura mediante el RNG.
* **Destructor:** Invoca `sodium_memzero` para eliminar de inmediato la clave de la memoria RAM.
* **`data()` / `size()` / `span()`**: Accesores de puntero y vistas de rango `std::span<const uint8_t>`.

### 2.2. Hashing (`CryptoHash`)
```cpp
#include "mithril/crypto_hash.hpp"

// Digest en un solo paso
auto digest = mithril::sodium::CryptoHash::hash(std::span<const uint8_t>(data));

// Digest por streaming
mithril::sodium::CryptoHash hasher;
hasher.update(chunk1);
hasher.update(chunk2);
auto final_hash = hasher.finalize();
```

### 2.3. Intercambio de Claves (`KeyExchange`)
```cpp
#include "mithril/key_exchange.hpp"

mithril::sodium::KeyExchange kex_alice;
auto alice_pubkey = kex_alice.public_key();

// En el lado de Bob:
mithril::sodium::KeyExchange kex_bob;
auto shared_secret_bob = kex_bob.derive_shared_secret(alice_pubkey);

// En el lado de Alice:
auto shared_secret_alice = kex_alice.derive_shared_secret(kex_bob.public_key());
// shared_secret_alice == shared_secret_bob
```

### 2.4. Cifrado de Flujo y AEAD (`StreamEncryption` y `AuthenticatedEncryption`)
```cpp
#include "mithril/stream_encryption.hpp"

mithril::sodium::SecureKey<32> key;
key.randomize();

mithril::sodium::AuthenticatedEncryption aead(key);
std::vector<uint8_t> ciphertext = aead.encrypt(plaintext_span);
std::vector<uint8_t> decrypted = aead.decrypt(ciphertext);
```

### 2.5. Firma Digital (`DigitalSignature`)
```cpp
#include "mithril/digital_signature.hpp"

mithril::sodium::DigitalSignature signer;
auto signature = signer.sign(message_span);
bool valid = signer.verify(message_span, signature, signer.public_key());
```

---

## 3. Capa de Red Asíncrona (`mithril::network`)

### 3.1. `SecureSession`
Representa una sesión TCP bidireccional cifrada y autenticada.
* **`start()`**: Inicia la negociación asíncrona del handshake ECDH.
* **`async_send(std::span<const uint8_t> data, Handler&& callback)`**: Cifra el mensaje con ChaCha20-Poly1305 y lo transmite.
* **`set_message_handler(MessageHandler handler)`**: Callback invocado al recibir y descifrar una trama válida.

### 3.2. `SecureAcceptor`
Escucha conexiones entrantes en un puerto TCP y despacha instancias de `SecureSession`.
* **`start_accept(uint16_t port, NewSessionHandler handler)`**: Inicia el bucle asíncrono de aceptación.
* **`stop()`**: Cierra el socket de escucha y cancela operaciones pendientes.

---

## 4. Protocolos Específicos

### 4.1. Protocolo Mareomotriz (`TidalProtocol`)
Cabecera: [`network/protocols/tidal_sensor_protocol.hpp`](file:///Users/andreibarwood/Documents/snocomm/Mithril/network/protocols/tidal_sensor_protocol.hpp)

```cpp
struct TidalMetrics {
    uint32_t device_id;
    uint64_t timestamp_ms;
    uint32_t sequence;
    float flow_velocity_mps;
    float water_pressure_kpa;
    float rotor_rpm;
    float power_output_kw;
    float water_temp_c;
    uint32_t status_flags;
};

// Empaquetar y cifrar
std::vector<uint8_t> frame = TidalProtocol::pack_telemetry(metrics, session_key, &optional_signing_key);

// Desempaquetar, verificar anti-replay y descifrar
TidalMetrics unpacked;
bool ok = TidalProtocol::unpack_telemetry(frame, session_key, replay_filter, unpacked, optional_peer_pubkey);
```

### 4.2. Protocolo IoT (`IoTProtocol`)
Cabecera: [`network/protocols/iot_protocol.hpp`](file:///Users/andreibarwood/Documents/snocomm/Mithril/network/protocols/iot_protocol.hpp)

```cpp
struct DeviceTelemetry {
    uint32_t device_id;
    uint64_t timestamp_ms;
    uint32_t sequence;
    float temperature;
    float humidity;
    float battery_level;
    uint32_t status_flags;
};
```
