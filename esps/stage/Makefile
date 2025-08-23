# Stage ESP32 Makefile
# Provides easy commands for building, uploading, and testing

.PHONY: help build upload monitor clean test install-deps

# Default target
help:
	@echo "Stage ESP32 - Available commands:"
	@echo "  build        - Build the project"
	@echo "  upload       - Upload to ESP32"
	@echo "  monitor      - Monitor serial output"
	@echo "  clean        - Clean build files"
	@echo "  test         - Run endpoint tests (requires ESP IP)"
	@echo "  install-deps - Install Python dependencies"
	@echo ""
	@echo "Usage examples:"
	@echo "  make build"
	@echo "  make upload"
	@echo "  make monitor"
	@echo "  make test ESP_IP=192.168.1.100"

# Build the project
build:
	@echo "🔨 Building Stage ESP32 project..."
	pio run

# Upload to ESP32
upload:
	@echo "📤 Uploading to ESP32..."
	pio run --target upload

# Monitor serial output
monitor:
	@echo "📺 Monitoring serial output..."
	pio device monitor

# Clean build files
clean:
	@echo "🧹 Cleaning build files..."
	pio run --target clean

# Install Python dependencies for testing
install-deps:
	@echo "📦 Installing Python dependencies..."
	pip install -r requirements.txt

# Test HTTP endpoints (requires ESP_IP variable)
test:
	@if [ -z "$(ESP_IP)" ]; then \
		echo "❌ Error: ESP_IP not specified"; \
		echo "Usage: make test ESP_IP=192.168.1.100"; \
		exit 1; \
	fi
	@echo "🧪 Testing endpoints at $(ESP_IP)..."
	python3 test_endpoints.py $(ESP_IP)

# Build and upload in one command
deploy: build upload
	@echo "✅ Deploy complete!"

# Full development cycle
dev: clean build upload monitor
