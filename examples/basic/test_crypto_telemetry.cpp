// ============================================================================
// Sprint 8 Ops Test - Crypto Failure Telemetry
// ============================================================================

#include <mithril/mithril_sodium.hpp>

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

namespace {

struct HookState {
    uint64_t calls = 0u;
    mithril_status last_status = MITHRIL_OK;
};

void on_failure(
    const char *,
    mithril_status status,
    const char *,
    void *user_data) {
    HookState *state = static_cast<HookState *>(user_data);
    if (state != nullptr) {
        state->calls += 1u;
        state->last_status = status;
    }
}

} // namespace

int main() {
    try {
        using namespace mithril::sodium;

        telemetry::reset();
        telemetry::clear_failure_hook();

        // Force one successful operation first.
        std::vector<uint8_t> msg{0xDEu, 0xADu, 0xBEu, 0xEFu};
        (void)CryptoHash::hash(msg);

        HookState hook_state{};
        telemetry::set_failure_hook(&on_failure, &hook_state);

        DigitalSignature signer;
        Signature sig = signer.sign(msg);
        msg[0] ^= 0x01u; // tamper message to trigger verify failure

        const bool verified = signer.verify(msg, sig, signer.get_verify_key());
        if (verified) {
            throw std::runtime_error("verify unexpectedly succeeded for tampered message");
        }

        const telemetry::Counters counters = telemetry::snapshot();
        if (counters.total_operations == 0u) {
            throw std::runtime_error("telemetry total_operations must be > 0");
        }
        if (counters.total_failures == 0u) {
            throw std::runtime_error("telemetry total_failures must be > 0");
        }
        if (counters.verification_failures == 0u) {
            throw std::runtime_error("verification_failures must be > 0");
        }
        if (hook_state.calls == 0u) {
            throw std::runtime_error("failure hook was not invoked");
        }
        if (hook_state.last_status != MITHRIL_ERR_SIGNATURE_INVALID) {
            throw std::runtime_error("expected last failure status to be MITHRIL_ERR_SIGNATURE_INVALID");
        }
        if (telemetry::failure_rate() <= 0.0) {
            throw std::runtime_error("failure_rate must be > 0 when failures exist");
        }

        telemetry::clear_failure_hook();
        std::cout << "[OK] test_crypto_telemetry" << std::endl;
        return 0;
    } catch (const std::exception &ex) {
        std::cerr << "[FAIL] test_crypto_telemetry: " << ex.what() << std::endl;
        return 1;
    }
}

