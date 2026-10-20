# 🌐 Capa de Red Asíncrona C++20 (Network Layer)

Este documento describe la arquitectura de la capa de comunicaciones asíncronas de Mithril, ubicada en [`network/`](file:///Users/andreibarwood/Documents/snocomm/Mithril/network).

---

## 1. Diseño Asíncrono no Bloqueante

La capa de red de Mithril se basa en el patrón arquitectónico **Reactor / Proactor** proporcionado por **Asio Standalone**:
* No requiere instalar o vincular el monolito de Boost (usa cabeceras autónomas con `-DASIO_STANDALONE`).
* Un único o múltiples hilos pueden gestionar concurrentemente miles de conexiones mediante una instancia de `asio::io_context`.
* Operaciones de E/S de red completamente asíncronas: `async_read`, `async_write`, `async_accept`.

---

## 2. Componentes Principales

```mermaid
flowchart TD
    IOContext[asio::io_context] --> SecureAcceptor[SecureAcceptor]
    SecureAcceptor -->|Acepta conexión TCP| SecureSession[SecureSession]
    
    subgraph SecureSessionInternals ["Mecanismo Interno de SecureSession"]
        Kex[KeyExchange (X25519)]
        AEAD[StreamEncryption (ChaCha20-Poly1305)]
        Socket[asio::ip::tcp::socket]
    end

    SecureSession --> Kex
    SecureSession --> AEAD
    SecureSession --> Socket
    
    SecureSession -->|Dispara Callback| MsgHandler[User Message Handler]
```

### 2.1. `SecureSession`
Representa una sesión TCP establecida entre dos nodos. Gestiona de forma autónoma:
1. **Fase de Negociación (Handshake):**
   - Transmite la clave pública efímera local.
   - Lee de forma asíncrona la clave pública efímera del par.
   - Deriva la clave de sesión compartida mediante X25519 y KDF.
   - Configura el contexto `AuthenticatedEncryption`.
2. **Fase de Comunicación Segura:**
   - Tramas con prefijo de longitud de 4 bytes (`uint32_t` en formato Big Endian).
   - Nonce de 12 bytes autoincremental por trama para evitar la reutilización de nonces en ChaCha20.
   - Tag de autenticación Poly1305 de 16 bytes.
   - Despacho de tramas verificadas al `MessageHandler` del usuario.

### 2.2. `SecureAcceptor`
Maneja el ciclo de vida del socket de escucha del servidor:
* Se enlaza a un puerto TCP (`asio::ip::tcp::endpoint`).
* Inicia un bucle recursivo de `async_accept`.
* Para cada conexión entrante, instancia un `std::shared_ptr<SecureSession>` e inicia inmediatamente su handshake.

---

## 3. Protocolo de Tramas (Wire Format)

Cada paquete cifrado transmitido a través del socket sigue la siguiente estructura binaria:

```text
+-----------------------+-------------------------+----------------------+-----------------------+
| Longitud (4 Bytes)    | Nonce (12 Bytes)        | Ciphertext (N Bytes) | Poly1305 Tag (16 B)   |
| Big-Endian uint32_t   | ChaCha20 IV             | Carga útil cifrada   | Autenticidad          |
+-----------------------+-------------------------+----------------------+-----------------------+
```

* **Longitud Total del Paquete:** $4 + 12 + N + 16 = N + 32$ bytes.
* Si la verificación del tag Poly1305 falla en el receptor, la sesión se aborta de inmediato para proteger contra manipulación activa.
