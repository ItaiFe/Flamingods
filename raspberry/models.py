"""
Data models for Sonoff WiFi Socket Server

This module defines all the data structures used by the server:
- Device information
- Control commands
- API responses
- WebSocket events
"""

from datetime import datetime
from enum import Enum
from typing import Dict, List, Optional, Any, Union
from pydantic import BaseModel, Field, validator


class DeviceStatus(str, Enum):
    """Device status enumeration"""
    ONLINE = "online"
    OFFLINE = "offline"
    UNKNOWN = "unknown"
    ERROR = "error"


class PowerState(str, Enum):
    """Power state enumeration"""
    ON = "on"
    OFF = "off"
    TOGGLE = "toggle"


class DeviceType(str, Enum):
    """Sonoff device type enumeration"""
    S26 = "S26"      # Basic WiFi Socket
    S31 = "S31"      # WiFi Socket with Power Monitoring
    S40 = "S40"      # WiFi Socket with USB
    S60 = "S60"      # WiFi Socket with Power Monitoring and USB
    S20 = "S20"      # WiFi Socket with Power Monitoring
    S10 = "S10"      # Basic WiFi Socket
    STAGE = "STAGE"  # Stage ESP32 LED Controller
    UNKNOWN = "unknown"


class StageLightingPlan(str, Enum):
    """Stage lighting plan enumeration"""
    IDLE = "idle"        # Ambient background lighting
    SKIP = "skip"        # Quick scene transitions
    SHOW = "show"        # Main performance lighting
    SPECIAL = "special"  # Special effects


class ConnectionType(str, Enum):
    """Device connection type"""
    LOCAL = "local"      # Local network connection
    CLOUD = "cloud"      # Cloud connection (if available)
    BOTH = "both"        # Both local and cloud


class DeviceInfo(BaseModel):
    """Device information model"""
    
    # Device identification
    id: str = Field(..., description="Unique device identifier")
    name: str = Field(..., description="Device name")
    type: DeviceType = Field(..., description="Device type")
    model: str = Field(..., description="Device model")
    
    # Network information
    ip_address: str = Field(..., description="Device IP address")
    mac_address: str = Field(..., description="Device MAC address")
    port: int = Field(default=80, description="Device port")
    
    # Status information
    status: DeviceStatus = Field(default=DeviceStatus.UNKNOWN, description="Device status")
    power_state: PowerState = Field(default=PowerState.OFF, description="Current power state")
    connection_type: ConnectionType = Field(default=ConnectionType.LOCAL, description="Connection type")
    
    # Device capabilities
    supports_power_monitoring: bool = Field(default=False, description="Supports power monitoring")
    supports_timer: bool = Field(default=False, description="Supports timer functionality")
    supports_schedule: bool = Field(default=False, description="Supports scheduling")
    
    # Additional information
    firmware_version: Optional[str] = Field(default=None, description="Firmware version")
    hardware_version: Optional[str] = Field(default=None, description="Hardware version")
    last_seen: Optional[datetime] = Field(default=None, description="Last time device was seen")
    
    # Power monitoring data (if available)
    voltage: Optional[float] = Field(default=None, description="Voltage in volts")
    current: Optional[float] = Field(default=None, description="Current in amperes")
    power: Optional[float] = Field(default=None, description="Power in watts")
    energy: Optional[float] = Field(default=None, description="Energy in kilowatt-hours")
    
    class Config:
        use_enum_values = True
        json_encoders = {
            datetime: lambda v: v.isoformat() if v else None
        }


class DeviceControl(BaseModel):
    """Device control command model"""
    
    # Control parameters
    power: PowerState = Field(..., description="Power state to set")
    
    # Optional parameters
    timer: Optional[int] = Field(default=None, description="Timer in seconds")
    schedule: Optional[Dict[str, Any]] = Field(default=None, description="Schedule configuration")
    
    # Validation
    @validator('timer')
    def validate_timer(cls, v):
        if v is not None and (v < 0 or v > 86400):  # 24 hours in seconds
            raise ValueError('Timer must be between 0 and 86400 seconds')
        return v


class StageControl(BaseModel):
    """Stage lighting control command model"""
    
    # Control parameters
    plan: StageLightingPlan = Field(..., description="Lighting plan to activate")
    
    # Optional parameters
    brightness: Optional[int] = Field(default=None, description="Brightness level (0-255)")
    duration: Optional[int] = Field(default=None, description="Duration in seconds (0 for infinite)")
    auto_return: Optional[bool] = Field(default=None, description="Auto-return to previous plan")
    
    # Validation
    @validator('brightness')
    def validate_brightness(cls, v):
        if v is not None and (v < 0 or v > 255):
            raise ValueError('Brightness must be between 0 and 255')
        return v
    
    @validator('duration')
    def validate_duration(cls, v):
        if v is not None and v < 0:
            raise ValueError('Duration must be non-negative')
        return v


class StageResponse(BaseModel):
    """Stage control response model"""
    
    # Response status
    success: bool = Field(..., description="Operation success status")
    message: str = Field(..., description="Response message")
    
    # Stage information
    device_id: str = Field(..., description="Stage device identifier")
    current_plan: StageLightingPlan = Field(..., description="Current active lighting plan")
    previous_plan: Optional[StageLightingPlan] = Field(default=None, description="Previous lighting plan")
    
    # Additional data
    data: Optional[Dict[str, Any]] = Field(default=None, description="Additional response data")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Response timestamp")
    
    class Config:
        use_enum_values = True
        json_encoders = {
            datetime: lambda v: v.isoformat()
        }


class StageStatus(BaseModel):
    """Stage device status model"""
    
    # Device identification
    device_id: str = Field(..., description="Stage device identifier")
    name: str = Field(..., description="Stage device name")
    
    # Current status
    current_plan: StageLightingPlan = Field(..., description="Current active lighting plan")
    wifi_connected: bool = Field(..., description="WiFi connection status")
    ip_address: str = Field(..., description="Device IP address")
    
    # System information
    uptime: int = Field(..., description="Device uptime in seconds")
    firmware_version: str = Field(..., description="Firmware version")
    rssi: Optional[int] = Field(default=None, description="WiFi signal strength")
    
    # OTA status
    ota_in_progress: bool = Field(default=False, description="OTA update in progress")
    ota_progress: Optional[int] = Field(default=None, description="OTA progress percentage")
    
    # Timestamp
    last_update: datetime = Field(default_factory=datetime.utcnow, description="Last status update")
    
    class Config:
        use_enum_values = True
        json_encoders = {
            datetime: lambda v: v.isoformat() if v else None
        }


class StagePlanInfo(BaseModel):
    """Stage lighting plan information model"""
    
    # Plan details
    plan: StageLightingPlan = Field(..., description="Lighting plan identifier")
    name: str = Field(..., description="Human-readable plan name")
    description: str = Field(..., description="Plan description and purpose")
    
    # Plan characteristics
    purpose: str = Field(..., description="Intended use case")
    pattern: str = Field(..., description="Lighting pattern description")
    speed: str = Field(..., description="Animation speed description")
    brightness: str = Field(..., description="Brightness level description")
    auto_return: bool = Field(..., description="Whether plan auto-returns to previous")
    
    # Duration information
    typical_duration: Optional[int] = Field(default=None, description="Typical duration in seconds")
    is_transitional: bool = Field(..., description="Whether this is a transition plan")


class BulkStageControl(BaseModel):
    """Bulk stage control model"""
    
    # Control parameters
    stages: List[str] = Field(..., description="List of stage device IDs to control")
    plan: StageLightingPlan = Field(..., description="Lighting plan to activate")
    
    # Optional parameters
    delay: Optional[float] = Field(default=None, description="Delay between commands in seconds")
    sequence: Optional[List[StageLightingPlan]] = Field(default=None, description="Sequence of plans to execute")
    timing: Optional[Dict[str, int]] = Field(default=None, description="Timing for sequence execution")
    
    # Validation
    @validator('stages')
    def validate_stages(cls, v):
        if not v:
            raise ValueError('Stage list cannot be empty')
        if len(v) > 10:
            raise ValueError('Cannot control more than 10 stages at once')
        return v
    
    @validator('delay')
    def validate_delay(cls, v):
        if v is not None and (v < 0 or v > 60):
            raise ValueError('Delay must be between 0 and 60 seconds')
        return v


class BulkStageResponse(BaseModel):
    """Bulk stage control response model"""
    
    # Overall results
    total_stages: int = Field(..., description="Total number of stages")
    successful: int = Field(..., description="Number of successful operations")
    failed: int = Field(..., description="Number of failed operations")
    
    # Detailed results
    results: List[StageResponse] = Field(..., description="Individual stage results")
    
    # Operation metadata
    operation_id: str = Field(..., description="Unique operation identifier")
    duration: float = Field(..., description="Operation duration in seconds")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Operation timestamp")
    
    class Config:
        use_enum_values = True
        json_encoders = {
            datetime: lambda v: v.isoformat()
        }


class DeviceResponse(BaseModel):
    """Device control response model"""
    
    # Response status
    success: bool = Field(..., description="Operation success status")
    message: str = Field(..., description="Response message")
    
    # Device information
    device_id: str = Field(..., description="Device identifier")
    power_state: PowerState = Field(..., description="New power state")
    
    # Additional data
    data: Optional[Dict[str, Any]] = Field(default=None, description="Additional response data")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Response timestamp")
    
    class Config:
        use_enum_values = True
        json_encoders = {
            datetime: lambda v: v.isoformat()
        }


class WebSocketEvent(BaseModel):
    """WebSocket event model"""
    
    # Event information
    event_type: str = Field(..., description="Type of event")
    device_id: str = Field(..., description="Device identifier")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Event timestamp")
    
    # Event data
    data: Dict[str, Any] = Field(..., description="Event data")
    
    # Event metadata
    source: str = Field(default="server", description="Event source")
    priority: str = Field(default="normal", description="Event priority")
    
    class Config:
        use_enum_values = True
        json_encoders = {
            datetime: lambda v: v.isoformat()
        }


class DeviceDiscoveryRequest(BaseModel):
    """Device discovery request model"""
    
    # Discovery parameters
    network_range: Optional[str] = Field(default=None, description="Network range to scan")
    timeout: Optional[int] = Field(default=None, description="Discovery timeout in seconds")
    force_refresh: bool = Field(default=False, description="Force refresh of device list")
    
    # Filtering options
    device_types: Optional[List[DeviceType]] = Field(default=None, description="Device types to discover")
    online_only: bool = Field(default=False, description="Only return online devices")


class DeviceDiscoveryResponse(BaseModel):
    """Device discovery response model"""
    
    # Discovery results
    devices: List[DeviceInfo] = Field(..., description="List of discovered devices")
    total_count: int = Field(..., description="Total number of devices found")
    online_count: int = Field(..., description="Number of online devices")
    
    # Discovery metadata
    discovery_time: datetime = Field(default_factory=datetime.utcnow, description="Discovery timestamp")
    network_range: str = Field(..., description="Network range scanned")
    duration: float = Field(..., description="Discovery duration in seconds")
    
    class Config:
        use_enum_values = True
        json_encoders = {
            datetime: lambda v: v.isoformat()
        }


class HealthCheck(BaseModel):
    """Health check response model"""
    
    # Health status
    status: str = Field(default="healthy", description="Overall health status")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Health check timestamp")
    
    # Component health
    server: str = Field(default="healthy", description="Server health status")
    database: str = Field(default="healthy", description="Database health status")
    network: str = Field(default="healthy", description="Network health status")
    
    # System information
    uptime: float = Field(..., description="Server uptime in seconds")
    version: str = Field(..., description="Server version")
    environment: str = Field(default="production", description="Environment")
    
    class Config:
        json_encoders = {
            datetime: lambda v: v.isoformat()
        }


class ErrorResponse(BaseModel):
    """Error response model"""
    
    # Error information
    error: str = Field(..., description="Error type")
    message: str = Field(..., description="Error message")
    details: Optional[str] = Field(default=None, description="Error details")
    
    # Error metadata
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Error timestamp")
    request_id: Optional[str] = Field(default=None, description="Request identifier")
    status_code: int = Field(..., description="HTTP status code")
    
    class Config:
        json_encoders = {
            datetime: lambda v: v.isoformat()
        }


class DeviceStatistics(BaseModel):
    """Device statistics model"""
    
    # Device identification
    device_id: str = Field(..., description="Device identifier")
    
    # Power statistics
    total_energy: float = Field(..., description="Total energy consumed in kWh")
    total_runtime: float = Field(..., description="Total runtime in hours")
    average_power: float = Field(..., description="Average power consumption in watts")
    
    # Usage patterns
    daily_usage: Dict[str, float] = Field(..., description="Daily energy usage")
    hourly_pattern: List[float] = Field(..., description="Hourly usage pattern")
    
    # Cost information
    energy_cost: float = Field(..., description="Total energy cost")
    cost_per_kwh: float = Field(..., description="Cost per kilowatt-hour")
    
    # Time period
    period_start: datetime = Field(..., description="Statistics period start")
    period_end: datetime = Field(..., description="Statistics period end")
    
    class Config:
        json_encoders = {
            datetime: lambda v: v.isoformat()
        }


class BulkDeviceControl(BaseModel):
    """Bulk device control model"""
    
    # Control parameters
    devices: List[str] = Field(..., description="List of device IDs to control")
    power: PowerState = Field(..., description="Power state to set")
    
    # Optional parameters
    delay: Optional[float] = Field(default=None, description="Delay between commands in seconds")
    timeout: Optional[int] = Field(default=None, description="Overall operation timeout")
    
    # Validation
    @validator('devices')
    def validate_devices(cls, v):
        if not v:
            raise ValueError('Device list cannot be empty')
        if len(v) > 100:
            raise ValueError('Cannot control more than 100 devices at once')
        return v
    
    @validator('delay')
    def validate_delay(cls, v):
        if v is not None and (v < 0 or v > 60):
            raise ValueError('Delay must be between 0 and 60 seconds')
        return v


class BulkDeviceResponse(BaseModel):
    """Bulk device control response model"""
    
    # Overall results
    total_devices: int = Field(..., description="Total number of devices")
    successful: int = Field(..., description="Number of successful operations")
    failed: int = Field(..., description="Number of failed operations")
    
    # Detailed results
    results: List[DeviceResponse] = Field(..., description="Individual device results")
    
    # Operation metadata
    operation_id: str = Field(..., description="Unique operation identifier")
    duration: float = Field(..., description="Operation duration in seconds")
    timestamp: datetime = Field(default_factory=datetime.utcnow, description="Operation timestamp")
    
    class Config:
        use_enum_values = True
        json_encoders = {
            datetime: lambda v: v.isoformat()
        }
