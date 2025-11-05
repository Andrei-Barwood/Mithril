# ============================================================================
# Mithril Network Layer - Makefile
# ============================================================================
# Compilación de ejemplos con libsodium + Boost.Asio
# ============================================================================

# Compiler configuration
CXX := clang++
CXXFLAGS := -std=c++20 -Wall -Wextra -O2
INCLUDES := -I/opt/homebrew/include -I/usr/local/include
LDFLAGS := -L/opt/homebrew/lib -L/usr/local/lib
LIBS := -lboost_system -lboost_context -lsodium -pthread

# Targets
SERVER := accepting_connection_sodium
CLIENT := secure_client

.PHONY: all clean run-server run-client test help

all: $(SERVER) $(CLIENT)

# ============================================================================
# Build Targets
# ============================================================================

$(SERVER): accepting_connection_sodium.cpp
	@echo "🔨 Compiling server..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(LDFLAGS) $(LIBS) -o $@
	@echo "✅ Server compiled: ./$(SERVER)"

$(CLIENT): secure_client.cpp
	@echo "🔨 Compiling client..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< $(LDFLAGS) $(LIBS) -o $@
	@echo "✅ Client compiled: ./$(CLIENT)"

# ============================================================================
# Run Targets
# ============================================================================

run-server: $(SERVER)
	@echo ""
	@echo "🚀 Starting Mithril secure server..."
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./$(SERVER)

run-client: $(CLIENT)
	@echo ""
	@echo "🚀 Starting Mithril secure client..."
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	./$(CLIENT) localhost 3333

# ============================================================================
# Testing
# ============================================================================

test: all
	@echo ""
	@echo "🧪 Running integration test..."
	@echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
	@echo ""
	@echo "Starting server in background..."
	@./$(SERVER) & SERVER_PID=$$!; \
	sleep 2; \
	echo "Running client..."; \
	./$(CLIENT) localhost 3333; \
	CLIENT_EXIT=$$?; \
	echo ""; \
	echo "Stopping server..."; \
	kill $$SERVER_PID 2>/dev/null || true; \
	if [ $$CLIENT_EXIT -eq 0 ]; then \
		echo "✅ Test passed!"; \
	else \
		echo "❌ Test failed!"; \
		exit 1; \
	fi

# ============================================================================
# Cleanup
# ============================================================================

clean:
	@echo "🧹 Cleaning build artifacts..."
	rm -f $(SERVER) $(CLIENT)
	rm -f *.o *.dSYM
	@echo "✅ Clean complete"

# ============================================================================
# Help
# ============================================================================

help:
	@echo ""
	@echo "╔═══════════════════════════════════════════════════════════╗"
	@echo "║          Mithril Network Layer - Build System             ║"
	@echo "╚═══════════════════════════════════════════════════════════╝"
	@echo ""
	@echo "Available targets:"
	@echo ""
	@echo "  make              Build both server and client"
	@echo "  make server       Build server only"
	@echo "  make client       Build client only"
	@echo "  make run-server   Build and run server"
	@echo "  make run-client   Build and run client (localhost:3333)"
	@echo "  make test         Run integration test"
	@echo "  make clean        Remove build artifacts"
	@echo "  make help         Show this help message"
	@echo ""
	@echo "Dependencies:"
	@echo "  - C++20 compiler (clang++ or g++)"
	@echo "  - Boost >= 1.82"
	@echo "  - libsodium"
	@echo ""
	@echo "Installation (macOS):"
	@echo "  brew install boost libsodium"
	@echo ""
	@echo "Installation (Ubuntu/Debian):"
	@echo "  sudo apt install libboost-all-dev libsodium-dev"
	@echo ""
	@echo "Usage example:"
	@echo "  Terminal 1: make run-server"
	@echo "  Terminal 2: make run-client"
	@echo ""

# ============================================================================
# Platform-specific adjustments
# ============================================================================

# Linux
ifeq ($(shell uname -s),Linux)
    INCLUDES := -I/usr/include
    LDFLAGS := -L/usr/lib
endif

# Check dependencies
check-deps:
	@echo "🔍 Checking dependencies..."
	@command -v $(CXX) >/dev/null 2>&1 || { echo "❌ $(CXX) not found"; exit 1; }
	@pkg-config --exists libsodium 2>/dev/null || { echo "⚠️  libsodium not found via pkg-config"; }
	@echo "✅ Basic dependencies OK"

.PHONY: check-deps

# ============================================================================
# Installation targets
# ============================================================================

install-deps-macos:
	@echo "📦 Installing dependencies on macOS..."
	brew install boost libsodium

install-deps-ubuntu:
	@echo "📦 Installing dependencies on Ubuntu/Debian..."
	sudo apt update
	sudo apt install -y build-essential libboost-all-dev libsodium-dev

.PHONY: install-deps-macos install-deps-ubuntu
