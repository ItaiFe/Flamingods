"""
Sonoff Device Manager

This module manages Sonoff WiFi devices including:
- Device discovery on local network
- Device control and monitoring
- Power management
- Device status tracking
"""

import asyncio
import aiohttp
import socket
import time
import json
import hashlib
from datetime import datetime, timedelta
from typing import Dict, List, Optional, Set, Tuple
from dataclasses import dataclass
import structlog

from config import get_config
from models import (
    DeviceInfo, DeviceStatus, PowerState, DeviceType, 
    ConnectionType, DeviceControl, DeviceResponse
)

logger = structlog.get_logger()


@dataclass
class SonoffDevice:
    """Internal Sonoff device representation"""
    
    # Device identification
    id: str
    name: str
    type: DeviceType
    model: str
    
    # Network information
    ip_address: str
    mac_address: str
    port: int
    
    # Status information
    status: DeviceStatus
    power_state: PowerState
    connection_type: ConnectionType
    
    # Device capabilities
    supports_power_monitoring: bool
    supports_timer: bool
    supports_schedule: bool
    
    # Additional information
    firmware_version: Optional[str] = None
    hardware_version: Optional[str] = None
    last_seen: Optional[datetime] = None
    
    # Power monitoring data
    voltage: Optional[float] = None
    current: Optional[float] = None
    power: Optional[float] = None
    energy: Optional[float] = None
    
    # Internal state
    _session: Optional[aiohttp.ClientSession] = None
    _last_control: Optional[datetime] = None
    _control_count: int = 0


class SonoffDeviceManager:
    """Manages Sonoff WiFi devices"""
    
    def __init__(self):
        self.config = get_config()
        self.devices: Dict[str, SonoffDevice] = {}
        self.discovery_running = False
        self.last_discovery = None
        self.discovery_lock = asyncio.Lock()
        
        # Device monitoring
        self.monitoring_task: Optional[asyncio.Task] = None
        self.monitoring_interval = 30  # seconds
        
        # HTTP session management
        self.session: Optional[aiohttp.ClientSession] = None
        self.session_timeout = aiohttp.ClientTimeout(
            total=self.config.sonoff.request_timeout
        )
    
    async def start(self):
        """Start the device manager"""
        logger.info("Starting Sonoff Device Manager")
        
        # Create HTTP session
        self.session = aiohttp.ClientSession(timeout=self.session_timeout)
        
        # Start device monitoring
        self.monitoring_task = asyncio.create_task(self._monitor_devices())
        
        # Initial device discovery
        await self.discover_devices()
        
        logger.info("Sonoff Device Manager started successfully")
    
    async def stop(self):
        """Stop the device manager"""
        logger.info("Stopping Sonoff Device Manager")
        
        # Stop monitoring
        if self.monitoring_task:
            self.monitoring_task.cancel()
            try:
                await self.monitoring_task
            except asyncio.CancelledError:
                pass
        
        # Close HTTP session
        if self.session:
            await self.session.close()
        
        logger.info("Sonoff Device Manager stopped")
    
    async def discover_devices(self, force_refresh: bool = False) -> List[DeviceInfo]:
        """Discover Sonoff devices on the local network"""
        
        async with self.discovery_lock:
            if (not force_refresh and 
                self.last_discovery and 
                time.time() - self.last_discovery < 300):  # 5 minutes cache
                logger.info("Using cached device discovery results")
                return [self._convert_to_device_info(device) for device in self.devices.values()]
            
            logger.info("Starting device discovery")
            self.discovery_running = True
            
            try:
                # Scan network for Sonoff devices
                discovered_devices = await self._scan_network()
                
                # Update device list
                await self._update_device_list(discovered_devices)
                
                self.last_discovery = time.time()
                logger.info(f"Device discovery completed: {len(self.devices)} devices found")
                
                return [self._convert_to_device_info(device) for device in self.devices.values()]
                
            finally:
                self.discovery_running = False
    
    async def _scan_network(self) -> List[Dict]:
        """Scan network for Sonoff devices"""
        discovered_devices = []
        
        # Parse network range
        network_parts = self.config.network.local_network.split('.')
        base_ip = f"{network_parts[0]}.{network_parts[1]}.{network_parts[2]}"
        
        # Scan IP range
        scan_tasks = []
        for i in range(1, 255):
            ip = f"{base_ip}.{i}"
            task = asyncio.create_task(self._scan_ip_for_sonoff_device(ip))
            scan_tasks.append(task)
        
        # Execute scans with concurrency limit
        semaphore = asyncio.Semaphore(self.config.network.max_concurrent_connections)
        
        async def limited_scan(task):
            async with semaphore:
                return await task
        
        limited_tasks = [limited_scan(task) for task in scan_tasks]
        
        # Wait for all scans to complete
        results = await asyncio.gather(*limited_tasks, return_exceptions=True)
        
        # Collect successful results
        for result in results:
            if isinstance(result, dict) and result:
                discovered_devices.append(result)
        
        return discovered_devices
    
    async def _scan_ip_for_sonoff_device(self, ip: str) -> Optional[Dict]:
        """Scan a specific IP for Sonoff devices"""
        try:
            # Quick port check
            if not await self._check_port_open(ip, 80):
                return None
            
            # Try to identify Sonoff device
            device_info = await self._identify_sonoff_device(ip)
            if device_info:
                logger.info(f"Found Sonoff device at {ip}: {device_info['name']}")
                return device_info
            
        except Exception as e:
            logger.debug(f"Error scanning {ip}: {e}")
        
        return None
    
    async def _check_port_open(self, ip: str, port: int) -> bool:
        """Check if a port is open on an IP address"""
        try:
            sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            sock.settimeout(self.config.network.connection_timeout)
            result = sock.connect_ex((ip, port))
            sock.close()
            return result == 0
        except:
            return False
    
    async def _identify_sonoff_device(self, ip: str) -> Optional[Dict]:
        """Identify if an IP hosts a Sonoff device"""
        try:
            if not self.session:
                return None
            
            # Try to access device info
            url = f"http://{ip}/device"
            async with self.session.get(url, timeout=5) as response:
                if response.status == 200:
                    data = await response.text()
                    
                    # Check if response contains Sonoff indicators
                    if self._is_sonoff_response(data):
                        return await self._extract_device_info(ip, data)
            
            # Try alternative endpoints
            alternative_endpoints = ['/info', '/status', '/api/info']
            for endpoint in alternative_endpoints:
                try:
                    url = f"http://{ip}{endpoint}"
                    async with self.session.get(url, timeout=3) as response:
                        if response.status == 200:
                            data = await response.text()
                            if self._is_sonoff_response(data):
                                return await self._extract_device_info(ip, data)
                except:
                    continue
            
        except Exception as e:
            logger.debug(f"Error identifying device at {ip}: {e}")
        
        return None
    
    def _is_sonoff_response(self, data: str) -> bool:
        """Check if response indicates a Sonoff device"""
        sonoff_indicators = [
            'sonoff', 'ewelink', 'ewelink.com', 'sonoff.tech',
            'deviceid', 'apikey', 'model', 'brand'
        ]
        
        data_lower = data.lower()
        return any(indicator in data_lower for indicator in sonoff_indicators)
    
    async def _extract_device_info(self, ip: str, data: str) -> Dict:
        """Extract device information from response"""
        try:
            # Try to parse JSON response
            if data.strip().startswith('{'):
                json_data = json.loads(data)
                return self._parse_json_device_info(ip, json_data)
            else:
                # Parse text response
                return self._parse_text_device_info(ip, data)
        except:
            # Fallback to basic info
            return self._create_basic_device_info(ip)
    
    def _parse_json_device_info(self, ip: str, data: Dict) -> Dict:
        """Parse device info from JSON response"""
        device_id = data.get('deviceid', f"sonoff_{ip.replace('.', '_')}")
        name = data.get('name', f"Sonoff Device {device_id}")
        model = data.get('model', 'Unknown')
        
        # Determine device type
        device_type = self._determine_device_type(model)
        
        return {
            'id': device_id,
            'name': name,
            'type': device_type,
            'model': model,
            'ip_address': ip,
            'mac_address': data.get('mac', 'Unknown'),
            'port': 80,
            'firmware_version': data.get('fwVersion', None),
            'hardware_version': data.get('hwVersion', None),
            'supports_power_monitoring': device_type in [DeviceType.S31, DeviceType.S60, DeviceType.S20],
            'supports_timer': True,
            'supports_schedule': True
        }
    
    def _parse_text_device_info(self, ip: str, data: str) -> Dict:
        """Parse device info from text response"""
        device_id = f"sonoff_{ip.replace('.', '_')}"
        name = f"Sonoff Device {device_id}"
        
        return {
            'id': device_id,
            'name': name,
            'type': DeviceType.UNKNOWN,
            'model': 'Unknown',
            'ip_address': ip,
            'mac_address': 'Unknown',
            'port': 80,
            'supports_power_monitoring': False,
            'supports_timer': True,
            'supports_schedule': True
        }
    
    def _create_basic_device_info(self, ip: str) -> Dict:
        """Create basic device info when parsing fails"""
        device_id = f"sonoff_{ip.replace('.', '_')}"
        name = f"Sonoff Device {device_id}"
        
        return {
            'id': device_id,
            'name': name,
            'type': DeviceType.UNKNOWN,
            'model': 'Unknown',
            'ip_address': ip,
            'mac_address': 'Unknown',
            'port': 80,
            'supports_power_monitoring': False,
            'supports_timer': True,
            'supports_schedule': True
        }
    
    def _determine_device_type(self, model: str) -> DeviceType:
        """Determine device type from model string"""
        model_lower = model.lower()
        
        if 's26' in model_lower:
            return DeviceType.S26
        elif 's31' in model_lower:
            return DeviceType.S31
        elif 's40' in model_lower:
            return DeviceType.S40
        elif 's60' in model_lower:
            return DeviceType.S60
        elif 's20' in model_lower:
            return DeviceType.S20
        elif 's10' in model_lower:
            return DeviceType.S10
        else:
            return DeviceType.UNKNOWN
    
    async def _update_device_list(self, discovered_devices: List[Dict]):
        """Update the internal device list"""
        # Create new device set
        new_device_ids = {device['id'] for device in discovered_devices}
        current_device_ids = set(self.devices.keys())
        
        # Remove devices no longer present
        removed_devices = current_device_ids - new_device_ids
        for device_id in removed_devices:
            del self.devices[device_id]
            logger.info(f"Removed device: {device_id}")
        
        # Add or update devices
        for device_data in discovered_devices:
            device_id = device_data['id']
            
            if device_id in self.devices:
                # Update existing device
                await self._update_device(device_id, device_data)
            else:
                # Create new device
                await self._create_device(device_data)
    
    async def _create_device(self, device_data: Dict):
        """Create a new device"""
        device = SonoffDevice(
            id=device_data['id'],
            name=device_data['name'],
            type=device_data['type'],
            model=device_data['model'],
            ip_address=device_data['ip_address'],
            mac_address=device_data['mac_address'],
            port=device_data['port'],
            status=DeviceStatus.UNKNOWN,
            power_state=PowerState.OFF,
            connection_type=ConnectionType.LOCAL,
            supports_power_monitoring=device_data.get('supports_power_monitoring', False),
            supports_timer=device_data.get('supports_timer', False),
            supports_schedule=device_data.get('supports_schedule', False),
            firmware_version=device_data.get('firmware_version'),
            hardware_version=device_data.get('hardware_version'),
            last_seen=datetime.utcnow()
        )
        
        self.devices[device_id] = device
        logger.info(f"Created new device: {device_id} ({device.name})")
    
    async def _update_device(self, device_id: str, device_data: Dict):
        """Update an existing device"""
        device = self.devices[device_id]
        
        # Update basic info
        device.name = device_data.get('name', device.name)
        device.model = device_data.get('model', device.model)
        device.last_seen = datetime.utcnow()
        
        # Update capabilities if new info available
        if 'supports_power_monitoring' in device_data:
            device.supports_power_monitoring = device_data['supports_power_monitoring']
        if 'supports_timer' in device_data:
            device.supports_timer = device_data['supports_timer']
        if 'supports_schedule' in device_data:
            device.supports_schedule = device_data['supports_schedule']
    
    async def control_device(self, device_id: str, control: DeviceControl) -> DeviceResponse:
        """Control a Sonoff device"""
        if device_id not in self.devices:
            raise ValueError(f"Device {device_id} not found")
        
        device = self.devices[device_id]
        
        try:
            # Send control command
            success = await self._send_control_command(device, control)
            
            if success:
                # Update device state
                device.power_state = control.power
                device._last_control = datetime.utcnow()
                device._control_count += 1
                
                # Update status
                device.status = DeviceStatus.ONLINE
                
                return DeviceResponse(
                    success=True,
                    message=f"Device {device_id} {control.power} successfully",
                    device_id=device_id,
                    power_state=control.power,
                    data={'control_count': device._control_count}
                )
            else:
                return DeviceResponse(
                    success=False,
                    message=f"Failed to control device {device_id}",
                    device_id=device_id,
                    power_state=device.power_state
                )
                
        except Exception as e:
            logger.error(f"Error controlling device {device_id}: {e}")
            return DeviceResponse(
                success=False,
                message=f"Error controlling device: {str(e)}",
                device_id=device_id,
                power_state=device.power_state
            )
    
    async def _send_control_command(self, device: SonoffDevice, control: DeviceControl) -> bool:
        """Send control command to device"""
        try:
            if not self.session:
                return False
            
            # Construct control URL
            if control.power == PowerState.TOGGLE:
                url = f"http://{device.ip_address}:{device.port}/toggle"
            else:
                power_value = "on" if control.power == PowerState.ON else "off"
                url = f"http://{device.ip_address}:{device.port}/switch/{power_value}"
            
            # Send command
            async with self.session.get(url, timeout=5) as response:
                if response.status == 200:
                    # Check response content
                    data = await response.text()
                    return self._is_successful_response(data)
                else:
                    logger.warning(f"Control command failed with status {response.status}")
                    return False
                    
        except Exception as e:
            logger.error(f"Error sending control command: {e}")
            return False
    
    def _is_successful_response(self, data: str) -> bool:
        """Check if response indicates successful operation"""
        success_indicators = ['success', 'ok', 'true', '1']
        data_lower = data.lower()
        return any(indicator in data_lower for indicator in success_indicators)
    
    async def get_device_status(self, device_id: str) -> Optional[DeviceInfo]:
        """Get current status of a device"""
        if device_id not in self.devices:
            return None
        
        device = self.devices[device_id]
        
        # Try to get real-time status
        try:
            await self._update_device_status(device)
        except Exception as e:
            logger.warning(f"Failed to update status for {device_id}: {e}")
        
        return self._convert_to_device_info(device)
    
    async def _update_device_status(self, device: SonoffDevice):
        """Update device status from device"""
        try:
            if not self.session:
                return
            
            # Get device status
            url = f"http://{device.ip_address}:{device.port}/status"
            async with self.session.get(url, timeout=5) as response:
                if response.status == 200:
                    data = await response.text()
                    self._parse_status_response(device, data)
                    
        except Exception as e:
            logger.debug(f"Failed to update status for {device.id}: {e}")
    
    def _parse_status_response(self, device: SonoffDevice, data: str):
        """Parse status response from device"""
        try:
            if data.strip().startswith('{'):
                json_data = json.loads(data)
                
                # Update power state
                if 'switch' in json_data:
                    switch_state = json_data['switch'].lower()
                    if switch_state in ['on', '1', 'true']:
                        device.power_state = PowerState.ON
                    else:
                        device.power_state = PowerState.OFF
                
                # Update power monitoring data if available
                if device.supports_power_monitoring:
                    device.voltage = json_data.get('voltage')
                    device.current = json_data.get('current')
                    device.power = json_data.get('power')
                    device.energy = json_data.get('energy')
                
                # Update status
                device.status = DeviceStatus.ONLINE
                device.last_seen = datetime.utcnow()
                
        except Exception as e:
            logger.debug(f"Failed to parse status response: {e}")
    
    def _convert_to_device_info(self, device: SonoffDevice) -> DeviceInfo:
        """Convert internal device to public DeviceInfo"""
        return DeviceInfo(
            id=device.id,
            name=device.name,
            type=device.type,
            model=device.model,
            ip_address=device.ip_address,
            mac_address=device.mac_address,
            port=device.port,
            status=device.status,
            power_state=device.power_state,
            connection_type=device.connection_type,
            supports_power_monitoring=device.supports_power_monitoring,
            supports_timer=device.supports_timer,
            supports_schedule=device.supports_schedule,
            firmware_version=device.firmware_version,
            hardware_version=device.hardware_version,
            last_seen=device.last_seen,
            voltage=device.voltage,
            current=device.current,
            power=device.power,
            energy=device.energy
        )
    
    async def _monitor_devices(self):
        """Monitor devices for status changes"""
        logger.info("Starting device monitoring")
        
        while True:
            try:
                # Update status of all devices
                for device in self.devices.values():
                    try:
                        await self._update_device_status(device)
                    except Exception as e:
                        logger.debug(f"Failed to update status for {device.id}: {e}")
                
                # Wait for next monitoring cycle
                await asyncio.sleep(self.monitoring_interval)
                
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Error in device monitoring: {e}")
                await asyncio.sleep(5)  # Wait before retrying
        
        logger.info("Device monitoring stopped")
    
    def get_device_count(self) -> int:
        """Get total number of devices"""
        return len(self.devices)
    
    def get_online_device_count(self) -> int:
        """Get number of online devices"""
        return sum(1 for device in self.devices.values() 
                  if device.status == DeviceStatus.ONLINE)
    
    def get_device_by_ip(self, ip: str) -> Optional[SonoffDevice]:
        """Get device by IP address"""
        for device in self.devices.values():
            if device.ip_address == ip:
                return device
        return None


# Global device manager instance
device_manager = SonoffDeviceManager()
