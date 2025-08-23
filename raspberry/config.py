"""
Configuration for Sonoff WiFi Socket Server

This module handles all configuration including:
- Server settings
- Sonoff device settings
- Network configuration
- Security settings
"""

import os
from typing import List, Optional
from pydantic_settings import BaseSettings
from pydantic import Field, validator


class ServerConfig(BaseSettings):
    """Server configuration settings"""
    
    # Server settings
    host: str = Field(default="0.0.0.0", description="Server host address")
    port: int = Field(default=8000, description="Server port")
    debug: bool = Field(default=False, description="Debug mode")
    log_level: str = Field(default="INFO", description="Logging level")
    
    # CORS settings
    cors_origins: List[str] = Field(
        default=["*"], 
        description="Allowed CORS origins"
    )
    
    # Security settings
    api_key: Optional[str] = Field(default=None, description="API key for authentication")
    enable_auth: bool = Field(default=False, description="Enable API authentication")
    
    class Config:
        env_file = ".env"
        extra = "allow"


class SonoffConfig(BaseSettings):
    """Sonoff device configuration settings"""
    
    # Device discovery
    discovery_timeout: int = Field(default=30, description="Device discovery timeout in seconds")
    max_devices: int = Field(default=100, description="Maximum number of devices to discover")
    
    # Communication settings
    request_timeout: int = Field(default=10, description="Request timeout in seconds")
    retry_attempts: int = Field(default=3, description="Number of retry attempts")
    retry_delay: float = Field(default=1.0, description="Delay between retries in seconds")
    
    # Device types supported
    supported_types: List[str] = Field(
        default=["S26", "S31", "S40", "S60", "S20", "S10"],
        description="Supported Sonoff device types"
    )
    
    class Config:
        env_file = ".env"
        extra = "allow"


class NetworkConfig(BaseSettings):
    """Network configuration settings"""
    
    # Local network settings
    local_network: str = Field(
        default="192.168.1.0/24", 
        description="Local network range for device discovery"
    )
    
    # Network scanning
    scan_ports: List[int] = Field(
        default=[80, 8080, 443, 8443],
        description="Ports to scan during discovery"
    )
    
    # Connection settings
    connection_timeout: int = Field(default=5, description="Connection timeout in seconds")
    max_concurrent_connections: int = Field(default=10, description="Maximum concurrent connections")
    
    class Config:
        env_file = ".env"
        extra = "allow"


class WebSocketConfig(BaseSettings):
    """WebSocket configuration settings"""
    
    # WebSocket settings
    max_connections: int = Field(default=100, description="Maximum WebSocket connections")
    ping_interval: float = Field(default=30.0, description="Ping interval in seconds")
    ping_timeout: float = Field(default=10.0, description="Ping timeout in seconds")
    
    # Event settings
    event_queue_size: int = Field(default=1000, description="Event queue size")
    broadcast_events: bool = Field(default=True, description="Broadcast events to all clients")
    
    class Config:
        env_file = ".env"
        extra = "allow"


class LoggingConfig(BaseSettings):
    """Logging configuration settings"""
    
    # Logging settings
    log_format: str = Field(
        default="%(asctime)s - %(name)s - %(levelname)s - %(message)s",
        description="Log format string"
    )
    log_file: Optional[str] = Field(default=None, description="Log file path")
    log_rotation: str = Field(default="1 day", description="Log rotation interval")
    log_retention: str = Field(default="30 days", description="Log retention period")
    
    # Structured logging
    enable_structured_logging: bool = Field(default=True, description="Enable structured logging")
    log_json: bool = Field(default=False, description="Output logs in JSON format")
    
    class Config:
        env_file = ".env"
        extra = "allow"


class Config:
    """Main configuration class that combines all settings"""
    
    def __init__(self):
        self.server = ServerConfig()
        self.sonoff = SonoffConfig()
        self.network = NetworkConfig()
        self.websocket = WebSocketConfig()
        self.logging = LoggingConfig()
    
    @validator('*', pre=True)
    def validate_config(cls, v):
        """Validate configuration values"""
        if isinstance(v, dict):
            for key, value in v.items():
                if isinstance(value, str) and value.lower() in ('true', 'false'):
                    v[key] = value.lower() == 'true'
        return v
    
    def get_device_config_path(self) -> str:
        """Get path to device configuration file"""
        return os.path.join(os.getcwd(), "config", "devices.yaml")
    
    def get_log_config_path(self) -> str:
        """Get path to logging configuration file"""
        return os.path.join(os.getcwd(), "config", "logging.yaml")


# Global configuration instance
config = Config()


def get_config() -> Config:
    """Get the global configuration instance"""
    return config


def reload_config() -> Config:
    """Reload configuration from environment and files"""
    global config
    config = Config()
    return config
