// ============================================================================
// Mithril Network Layer - Secure Client
// ============================================================================
// Complemento para accepting_connection_sodium.cpp
// Cliente con cifrado libsodium para testing
// ============================================================================

#include <boost/asio.hpp>
#include <boost/asio/awaitable.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/use_awaitable.hpp>
#include <sodium.h>
#include <iostream>
#include <array>
#include <vector>
#include <span>
#include <string>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

// ============================================================================
// RAII Wrapper for Keys (mismo que el servidor)
// ============================================================================

template<size_t N>
class SecureKey {
public:
    SecureKey() {
        sodium_mlock(data_.data(), data_.size());
    }
    
    ~SecureKey() {
        sodium_munlock(data_.data(), data_.size());
    }
    
    SecureKey(const SecureKey&) = delete;
    SecureKey& operator=(const SecureKey&) = delete;
    SecureKey(SecureKey&&) noexcept = default;
    SecureKey& operator=(SecureKey&&) noexcept = default;
    
    uint8_t* data() { return data_.data(); }
    const uint8_t* data() const { return data_.data(); }
    constexpr size_t size() const { return N; }

private:
    std::array<uint8_t, N> data_;
};

using PublicKey = SecureKey<crypto_box_PUBLICKEYBYTES>;
using SecretKey = SecureKey<crypto_box_SECRETKEYBYTES>;
using SessionKey = SecureKey<crypto_secretbox_KEYBYTES>;

// ============================================================================
// Key Exchange (igual que servidor)
// ============================================================================

class KeyExchange {
public:
    KeyExchange() {
        if (sodium_init() < 0) {
            throw std::runtime_error("libsodium initialization failed");
        }
        crypto_box_keypair(public_key_.data(), secret_key_.data());
    }
    
    const PublicKey& get_public_key() const { return public_key_; }
    
    SessionKey derive_shared_secret(const PublicKey& peer_public_key) const {
        SessionKey shared;
        
        if (crypto_box_beforenm(
                shared.data(),
                peer_public_key.data(),
                secret_key_.data()) != 0) {
            throw std::runtime_error("Key exchange failed");
        }
        
        return shared;
    }

private:
    PublicKey public_key_;
    SecretKey secret_key_;
};

// ============================================================================
// Crypto Handler (igual que servidor)
// ============================================================================

class CryptoHandler {
public:
    explicit CryptoHandler(const SessionKey& key) : key_(key) {}
    
    std::vector<uint8_t> encrypt(std::span<const uint8_t> plaintext) {
        std::array<uint8_t, crypto_secretbox_NONCEBYTES> nonce;
        randombytes_buf(nonce.data(), nonce.size());
        
        std::vector<uint8_t> result(
            nonce.size() + plaintext.size() + crypto_secretbox_MACBYTES
        );
        
        std::copy(nonce.begin(), nonce.end(), result.begin());
        
        if (crypto_secretbox_easy(
                result.data() + nonce.size(),
                plaintext.data(), plaintext.size(),
                nonce.data(), key_.data()) != 0) {
            throw std::runtime_error("Encryption failed");
        }
        
        return result;
    }
    
    std::vector<uint8_t> decrypt(std::span<const uint8_t> ciphertext) {
        if (ciphertext.size() < crypto_secretbox_NONCEBYTES + crypto_secretbox_MACBYTES) {
            throw std::invalid_argument("Ciphertext too short");
        }
        
        std::array<uint8_t, crypto_secretbox_NONCEBYTES> nonce;
        std::copy_n(ciphertext.begin(), nonce.size(), nonce.begin());
        
        auto cipher_data = ciphertext.subspan(nonce.size());
        std::vector<uint8_t> plaintext(cipher_data.size() - crypto_secretbox_MACBYTES);
        
        if (crypto_secretbox_open_easy(
                plaintext.data(),
                cipher_data.data(), cipher_data.size(),
                nonce.data(), key_.data()) != 0) {
            throw std::runtime_error("Decryption failed or message forged");
        }
        
        return plaintext;
    }

private:
    SessionKey key_;
};

// ============================================================================
// Secure Client
// ============================================================================

class SecureClient {
public:
    SecureClient(asio::io_context& io_ctx)
        : io_ctx_(io_ctx), socket_(io_ctx) {}
    
    asio::awaitable<void> connect_and_communicate(
        const std::string& host,
        const std::string& port) {
        
        try {
            std::cout << "╔═══════════════════════════════════════════════╗\n";
            std::cout << "║     Mithril Secure Client - libsodium        ║\n";
            std::cout << "╚═══════════════════════════════════════════════╝\n\n";
            
            // STEP 1: Resolver y conectar
            std::cout << "🔍 Resolving " << host << ":" << port << "...\n";
            tcp::resolver resolver(io_ctx_);
            auto endpoints = co_await resolver.async_resolve(
                host, port, asio::use_awaitable
            );
            
            std::cout << "🌐 Connecting to server...\n";
            co_await asio::async_connect(
                socket_, endpoints, asio::use_awaitable
            );
            
            std::cout << "✓ Connected to " 
                      << socket_.remote_endpoint() << "\n\n";
            
            // STEP 2: Key Exchange
            co_await perform_key_exchange();
            
            // STEP 3: Enviar mensajes cifrados
            co_await send_encrypted_messages();
            
            std::cout << "\n✓ Session completed successfully\n";
            
        } catch (const std::exception& e) {
            std::cerr << "❌ Error: " << e.what() << "\n";
        }
    }

private:
    asio::awaitable<void> perform_key_exchange() {
        std::cout << "🔐 Starting key exchange (Curve25519 ECDH)...\n";
        
        KeyExchange kex;
        
        // Recibir clave pública del servidor
        PublicKey server_pubkey;
        co_await asio::async_read(
            socket_,
            asio::buffer(server_pubkey.data(), server_pubkey.size()),
            asio::use_awaitable
        );
        
        std::cout << "   ← Received server public key\n";
        
        // Enviar nuestra clave pública
        const auto& our_pubkey = kex.get_public_key();
        co_await asio::async_write(
            socket_,
            asio::buffer(our_pubkey.data(), our_pubkey.size()),
            asio::use_awaitable
        );
        
        std::cout << "   → Sent our public key\n";
        
        // Derivar clave compartida
        auto shared_secret = kex.derive_shared_secret(server_pubkey);
        crypto_handler_ = std::make_unique<CryptoHandler>(shared_secret);
        
        std::cout << "✓ Secure channel established\n\n";
    }
    
    asio::awaitable<void> send_encrypted_messages() {
        // Mensajes de prueba
        std::vector<std::string> messages = {
            "Hello from Mithril secure client!",
            "Testing ChaCha20-Poly1305 encryption",
            "IoT sensor data: temp=22.5°C, humidity=65%",
            "This is end-to-end encrypted"
        };
        
        for (const auto& message : messages) {
            std::cout << "📤 Sending: \"" << message << "\"\n";
            
            // Cifrar
            std::vector<uint8_t> plaintext(message.begin(), message.end());
            auto encrypted = crypto_handler_->encrypt(plaintext);
            
            // Enviar tamaño + datos cifrados
            uint32_t length = htonl(encrypted.size());
            std::array<asio::const_buffer, 2> buffers{
                asio::buffer(&length, sizeof(length)),
                asio::buffer(encrypted)
            };
            
            co_await asio::async_write(socket_, buffers, asio::use_awaitable);
            
            std::cout << "   → Sent " << encrypted.size() 
                      << " bytes (encrypted)\n";
            
            // Recibir respuesta
            uint32_t response_length;
            co_await asio::async_read(
                socket_,
                asio::buffer(&response_length, sizeof(response_length)),
                asio::use_awaitable
            );
            
            response_length = ntohl(response_length);
            
            std::vector<uint8_t> encrypted_response(response_length);
            co_await asio::async_read(
                socket_,
                asio::buffer(encrypted_response),
                asio::use_awaitable
            );
            
            // Descifrar
            auto decrypted = crypto_handler_->decrypt(encrypted_response);
            std::string response(decrypted.begin(), decrypted.end());
            
            std::cout << "📥 Received: \"" << response << "\"\n";
            std::cout << "   ← " << encrypted_response.size() 
                      << " bytes (decrypted)\n\n";
            
            // Pequeña pausa entre mensajes
            asio::steady_timer timer(io_ctx_);
            timer.expires_after(std::chrono::milliseconds(500));
            co_await timer.async_wait(asio::use_awaitable);
        }
    }

    asio::io_context& io_ctx_;
    tcp::socket socket_;
    std::unique_ptr<CryptoHandler> crypto_handler_;
};

// ============================================================================
// Main Function
// ============================================================================

int main(int argc, char* argv[]) {
    try {
        if (argc != 3) {
            std::cerr << "Usage: " << argv[0] << " <host> <port>\n";
            std::cerr << "Example: " << argv[0] << " localhost 3333\n";
            return 1;
        }
        
        std::string host = argv[1];
        std::string port = argv[2];
        
        asio::io_context io_ctx;
        
        SecureClient client(io_ctx);
        
        asio::co_spawn(
            io_ctx,
            client.connect_and_communicate(host, port),
            [](std::exception_ptr e) {
                if (e) {
                    try {
                        std::rethrow_exception(e);
                    } catch (const std::exception& ex) {
                        std::cerr << "Fatal: " << ex.what() << "\n";
                    }
                }
            }
        );
        
        io_ctx.run();
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}

// ============================================================================
// COMPILATION & USAGE
// ============================================================================
//
// Compile:
//   clang++ -std=c++20 -I/opt/homebrew/include \
//           -L/opt/homebrew/lib \
//           secure_client.cpp \
//           -lboost_system -lboost_context -lsodium -o client
//
// Run:
//   # Terminal 1 (server):
//   ./server
//
//   # Terminal 2 (client):
//   ./client localhost 3333
//
// Expected Output:
//   - Key exchange completes
//   - Messages encrypted with ChaCha20-Poly1305
//   - Server echoes back encrypted responses
//   - All communication authenticated (prevents tampering)
//
// ============================================================================
