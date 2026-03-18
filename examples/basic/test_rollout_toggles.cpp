// ============================================================================
// Sprint 8 Ops Test - Rollout Toggle Policy
// ============================================================================

#include <mithril/boost_asio_compat.hpp>

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

void set_env(const char *name, const char *value) {
    if (setenv(name, value, 1) != 0) {
        throw std::runtime_error(std::string("setenv failed for ") + name);
    }
}

void clear_env(const char *name) {
    (void)unsetenv(name);
}

void expect_blocked() {
    bool blocked = false;
    try {
        mithril::network::compat::enforce_rollout_policy();
    } catch (const std::exception &) {
        blocked = true;
    }

    if (!blocked) {
        throw std::runtime_error("expected rollout policy to block process");
    }
}

} // namespace

int main() {
    try {
        clear_env("MITHRIL_V2_ROLLOUT_MODE");
        clear_env("MITHRIL_V2_CANARY_PERCENT");
        clear_env("MITHRIL_V2_CANARY_KEY");

        mithril::network::compat::enforce_rollout_policy();
        const std::string default_summary = mithril::network::compat::rollout_summary();
        if (default_summary.find("mode=full") == std::string::npos) {
            throw std::runtime_error("default rollout mode must be full");
        }

        set_env("MITHRIL_V2_ROLLOUT_MODE", "canary");
        set_env("MITHRIL_V2_CANARY_PERCENT", "100");
        set_env("MITHRIL_V2_CANARY_KEY", "sprint8-ci");
        mithril::network::compat::enforce_rollout_policy();

        const std::string enabled_summary = mithril::network::compat::rollout_summary();
        if (enabled_summary.find("decision=enabled") == std::string::npos) {
            throw std::runtime_error("canary=100 must enable rollout");
        }

        set_env("MITHRIL_V2_CANARY_PERCENT", "0");
        expect_blocked();

        set_env("MITHRIL_V2_ROLLOUT_MODE", "rollback");
        clear_env("MITHRIL_V2_CANARY_PERCENT");
        expect_blocked();

        set_env("MITHRIL_V2_ROLLOUT_MODE", "canary");
        set_env("MITHRIL_V2_CANARY_PERCENT", "101");
        expect_blocked();

        std::cout << "[OK] test_rollout_toggles" << std::endl;
        return 0;
    } catch (const std::exception &ex) {
        std::cerr << "[FAIL] test_rollout_toggles: " << ex.what() << std::endl;
        return 1;
    }
}

