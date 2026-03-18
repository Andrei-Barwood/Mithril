# ============================================================================
# Mithril Network Layer - Makefile (self-contained)
# ============================================================================
# Builds network examples against Mithril API v2 + in-repo boost::asio shim
# without requiring a system Boost installation.
# ============================================================================

CXX ?= c++

ROOT_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))/../..)
MITHRIL_V2_DIR := $(ROOT_DIR)/Mithril_API_V2
MITHRIL_V2_BUILD_DIR ?= $(MITHRIL_V2_DIR)/build-cmake
MITHRIL_V2_LIB := $(MITHRIL_V2_BUILD_DIR)/libmithril_v2.a

ASIO_VENDOR_DIR := $(ROOT_DIR)/network/third_party/asio
ASIO_INCLUDE_DIR := $(ASIO_VENDOR_DIR)/asio/include
ASIO_VERSION := asio-1-30-2
ASIO_ARCHIVE_URL := https://github.com/chriskohlhoff/asio/archive/refs/tags/$(ASIO_VERSION).tar.gz

SODIUM_CFLAGS := $(shell pkg-config --cflags libsodium 2>/dev/null)
SODIUM_LIBS := $(shell pkg-config --libs libsodium 2>/dev/null)

ifeq ($(strip $(SODIUM_LIBS)),)
  SODIUM_LIBS := -lsodium
endif

CXXFLAGS := -std=c++20 -Wall -Wextra -O2 \
            -DASIO_STANDALONE \
            -DMITHRIL_CONSUMER_PATH_V2_DEFAULT=1 \
            -DMITHRIL_ENABLE_V1_COMPAT_PATH=0

INCLUDES := -I$(ROOT_DIR)/network/include \
            -I$(ROOT_DIR)/Mithril_API_V2/include \
            -I$(ASIO_INCLUDE_DIR) \
            $(SODIUM_CFLAGS)

LDFLAGS :=
LIBS := $(MITHRIL_V2_LIB) $(SODIUM_LIBS)

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
  LIBS += -framework Security
else
  LIBS += -pthread
endif

SERVER := accepting_connection_sodium
CLIENT := secure_client
ROLLOUT_TEST := test_rollout_toggles
TELEMETRY_TEST := test_crypto_telemetry
COMPAT_SRC := $(ROOT_DIR)/network/src/boost_asio_compat.cpp

.PHONY: all clean run-server run-client test test-sprint8 help check-deps ensure-asio ensure-v2 clean-third-party

all: check-deps ensure-asio ensure-v2 $(SERVER) $(CLIENT)

# ============================================================================
# Dependency/bootstrap targets
# ============================================================================

check-deps:
	@echo "[deps] checking toolchain..."
	@command -v $(CXX) >/dev/null 2>&1 || { echo "[error] $(CXX) not found"; exit 1; }
	@command -v cmake >/dev/null 2>&1 || { echo "[error] cmake not found"; exit 1; }
	@command -v pkg-config >/dev/null 2>&1 || { echo "[error] pkg-config not found"; exit 1; }
	@command -v curl >/dev/null 2>&1 || { echo "[error] curl not found"; exit 1; }
	@command -v tar >/dev/null 2>&1 || { echo "[error] tar not found"; exit 1; }
	@pkg-config --exists libsodium 2>/dev/null || { echo "[error] libsodium not found (pkg-config)"; exit 1; }
	@echo "[deps] OK"

ensure-asio:
	@if [ -f "$(ASIO_INCLUDE_DIR)/asio.hpp" ]; then \
		echo "[asio] using vendored Asio at $(ASIO_INCLUDE_DIR)"; \
	else \
		echo "[asio] vendored Asio not found, downloading $(ASIO_VERSION)..."; \
		tmp_dir=$$(mktemp -d); \
		archive_path="$$tmp_dir/asio.tar.gz"; \
		curl -fsSL "$(ASIO_ARCHIVE_URL)" -o "$$archive_path"; \
		tar -xzf "$$archive_path" -C "$$tmp_dir"; \
		extract_dir=$$(find "$$tmp_dir" -maxdepth 1 -type d -name "asio-*" | head -n 1); \
		if [ -z "$$extract_dir" ]; then \
			echo "[error] could not locate extracted Asio directory"; \
			rm -rf "$$tmp_dir"; \
			exit 1; \
		fi; \
		rm -rf "$(ASIO_VENDOR_DIR)"; \
		mkdir -p "$(ASIO_VENDOR_DIR)"; \
		cp -R "$$extract_dir"/* "$(ASIO_VENDOR_DIR)/"; \
		rm -rf "$$tmp_dir"; \
		echo "[asio] vendored Asio ready"; \
	fi

ensure-v2: $(MITHRIL_V2_LIB)

$(MITHRIL_V2_LIB):
	@echo "[v2] configuring Mithril_API_V2..."
	@cmake -S "$(MITHRIL_V2_DIR)" -B "$(MITHRIL_V2_BUILD_DIR)" \
		-DCMAKE_BUILD_TYPE=RelWithDebInfo \
		-DMITHRIL_ENABLE_TESTS=OFF \
		-DMITHRIL_USE_LIBSODIUM=ON \
		-DMITHRIL_ENABLE_PROVIDER_FLINT=OFF \
		-DMITHRIL_USE_FLINT=OFF
	@echo "[v2] building library..."
	@cmake --build "$(MITHRIL_V2_BUILD_DIR)" --parallel

# ============================================================================
# Build targets
# ============================================================================

$(SERVER): accepting_connection_sodium.cpp $(COMPAT_SRC) ensure-asio ensure-v2
	@echo "[build] compiling $(SERVER)..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(COMPAT_SRC) $< $(LDFLAGS) $(LIBS) -o $@
	@echo "[build] done: ./$(SERVER)"

$(CLIENT): secure_client.cpp $(COMPAT_SRC) ensure-asio ensure-v2
	@echo "[build] compiling $(CLIENT)..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(COMPAT_SRC) $< $(LDFLAGS) $(LIBS) -o $@
	@echo "[build] done: ./$(CLIENT)"

$(ROLLOUT_TEST): test_rollout_toggles.cpp $(COMPAT_SRC) ensure-asio
	@echo "[build] compiling $(ROLLOUT_TEST)..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(COMPAT_SRC) $< $(LDFLAGS) -o $@
	@echo "[build] done: ./$(ROLLOUT_TEST)"

$(TELEMETRY_TEST): test_crypto_telemetry.cpp $(COMPAT_SRC) ensure-asio ensure-v2
	@echo "[build] compiling $(TELEMETRY_TEST)..."
	@$(CXX) $(CXXFLAGS) $(INCLUDES) $(COMPAT_SRC) $< $(LDFLAGS) $(LIBS) -o $@
	@echo "[build] done: ./$(TELEMETRY_TEST)"

# ============================================================================
# Run targets
# ============================================================================

run-server: $(SERVER)
	@./$(SERVER)

run-client: $(CLIENT)
	@./$(CLIENT) localhost 3333

# ============================================================================
# Testing
# ============================================================================

test-sprint8: $(ROLLOUT_TEST) $(TELEMETRY_TEST)
	@echo "[test] running sprint 8 rollout toggles"
	@./$(ROLLOUT_TEST)
	@echo "[test] running sprint 8 telemetry hooks"
	@./$(TELEMETRY_TEST)

test: all test-sprint8
	@echo "[test] starting integration test"
	@./$(SERVER) > ./server_test.log 2>&1 & SERVER_PID=$$!; \
	sleep 1; \
	./$(CLIENT) localhost 3333 > ./client_test.log 2>&1; \
	CLIENT_EXIT=$$?; \
	kill $$SERVER_PID 2>/dev/null || true; \
	wait $$SERVER_PID 2>/dev/null || true; \
	if [ $$CLIENT_EXIT -eq 0 ]; then \
		echo "[test] OK"; \
		rm -f ./server_test.log ./client_test.log; \
	else \
		echo "[test] FAILED"; \
		echo "--- server_test.log ---"; \
		cat ./server_test.log; \
		echo "--- client_test.log ---"; \
		cat ./client_test.log; \
		exit 1; \
	fi

# ============================================================================
# Cleanup
# ============================================================================

clean:
	@echo "[clean] removing local binaries"
	@rm -f $(SERVER) $(CLIENT) $(ROLLOUT_TEST) $(TELEMETRY_TEST)
	@rm -rf *.dSYM

clean-third-party:
	@echo "[clean] removing vendored Asio"
	@rm -rf "$(ASIO_VENDOR_DIR)"

# ============================================================================
# Help
# ============================================================================

help:
	@echo "Mithril examples (local)"
	@echo "  make                 -> build server + client"
	@echo "  make run-server      -> run secure server"
	@echo "  make run-client      -> run secure client"
	@echo "  make test-sprint8    -> run rollout and telemetry ops tests"
	@echo "  make test            -> integration test"
	@echo "  make clean           -> remove built binaries"
	@echo "  make clean-third-party -> remove vendored Asio"
