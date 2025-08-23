# Makefile for Sonoff WiFi Socket Server
# Common development and deployment commands

.PHONY: help install install-dev test lint format clean run discover build

# Default target
help:
	@echo "Sonoff WiFi Socket Server - Available Commands:"
	@echo ""
	@echo "Installation:"
	@echo "  install      Install production dependencies"
	@echo "  install-dev  Install development dependencies"
	@echo ""
	@echo "Development:"
	@echo "  run          Run the server"
	@echo "  discover     Run device discovery"
	@echo "  test         Run tests"
	@echo "  lint         Run linting checks"
	@echo "  format       Format code with black and isort"
	@echo ""
	@echo "Maintenance:"
	@echo "  clean        Clean up generated files"
	@echo "  build        Build package distribution"
	@echo ""

# Install production dependencies
install:
	@echo "Installing production dependencies..."
	uv pip install -e .

# Install development dependencies
install-dev:
	@echo "Installing development dependencies..."
	uv pip install -e ".[dev]"

# Run the server
run:
	@echo "Starting Sonoff WiFi Socket Server..."
	uv run python main.py

# Run device discovery
discover:
	@echo "Running device discovery..."
	uv run python -c "
import asyncio
from sonoff_manager import device_manager
async def main():
    await device_manager.start()
    devices = await device_manager.discover_devices(force_refresh=True)
    print(f'Discovered {len(devices)} devices:')
    for device in devices:
        print(f'  - {device.name} ({device.ip_address}) - {device.status}')
    await device_manager.stop()
asyncio.run(main())
"

# Run tests
test:
	@echo "Running tests..."
	uv run pytest

# Run tests with coverage
test-cov:
	@echo "Running tests with coverage..."
	uv run pytest --cov

# Run linting
lint:
	@echo "Running linting checks..."
	uv run ruff check .
	uv run mypy .

# Format code
format:
	@echo "Formatting code..."
	uv run black .
	uv run isort .

# Clean up generated files
clean:
	@echo "Cleaning up..."
	find . -type f -name "*.pyc" -delete
	find . -type d -name "__pycache__" -delete
	find . -type d -name "*.egg-info" -exec rm -rf {} +
	find . -type d -name ".pytest_cache" -exec rm -rf {} +
	find . -type d -name ".coverage" -delete
	find . -type d -name "htmlcov" -exec rm -rf {} +
	find . -type d -name ".mypy_cache" -exec rm -rf {} +

# Build package
build:
	@echo "Building package..."
	uv build

# Install in development mode
dev: install-dev
	@echo "Development environment ready!"

# Quick start (install and run)
start: install run

# Discovery and configuration
setup: install discover
	@echo "Setup complete! Check discovered devices and configure your setup."

# Production deployment
deploy: install
	@echo "Production deployment ready!"
	@echo "Use: sudo systemctl start sonoff-server"

# Quick server test
test-server:
	@echo "Testing server endpoints..."
	uv run python -c "
import requests
import time
try:
    # Wait for server to start
    time.sleep(2)
    response = requests.get('http://localhost:8000/health')
    print(f'Health check: {response.status_code}')
    if response.status_code == 200:
        print('✅ Server is running!')
    else:
        print('❌ Server health check failed')
except Exception as e:
    print(f'❌ Server test failed: {e}')
"
