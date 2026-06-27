# Makefile for Flamingo ESP32 LED Pattern Controller
# Provides convenient commands for building, uploading, and monitoring

.PHONY: help build upload monitor clean test

# Default target
help:
	@echo "Available commands:"
	@echo "  build    - Build the project"
	@echo "  upload   - Build and upload to ESP32"
	@echo "  monitor  - Monitor serial output"
	@echo "  clean    - Clean build files"
	@echo "  test     - Run tests (if available)"

# Build the project
build:
	pio run

# Upload to ESP32
upload:
	pio run --target upload

# Monitor serial output
monitor:
	pio device monitor

# Clean build files
clean:
	pio run --target clean

# Build and upload in one command
deploy: build upload

# Build, upload, and monitor
deploy-monitor: build upload monitor
