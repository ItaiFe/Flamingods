"""
WebSocket Manager for Real-time Communication

This module handles WebSocket connections and real-time event broadcasting
to connected clients.
"""

import asyncio
import json
import time
from datetime import datetime, timezone
from typing import Dict, List, Optional, Set, Any
from dataclasses import dataclass
import structlog

from fastapi import WebSocket, WebSocketDisconnect
from models import WebSocketEvent, DeviceControl, PowerState, DeviceInfo
from config import get_config

logger = structlog.get_logger()


@dataclass
class WebSocketClient:
    """WebSocket client connection"""
    websocket: WebSocket
    client_id: str
    connected_at: datetime
    last_ping: datetime
    subscriptions: Set[str] = None
    
    def __post_init__(self):
        if self.subscriptions is None:
            self.subscriptions = set()


class WebSocketManager:
    """Manages WebSocket connections and events"""
    
    def __init__(self):
        self.config = get_config()
        self.clients: Dict[str, WebSocketClient] = {}
        self.event_queue: asyncio.Queue = asyncio.Queue(maxsize=self.config.websocket.event_queue_size)
        self.broadcast_task: Optional[asyncio.Task] = None
        self.cleanup_task: Optional[asyncio.Task] = None
        
        # Event statistics
        self.total_events_sent = 0
        self.total_clients_connected = 0
        self.total_clients_disconnected = 0
        
        # Start time for uptime calculation
        self._start_time = time.time()
    
    async def start(self):
        """Start the WebSocket manager"""
        logger.info("Starting WebSocket Manager")
        
        # Start background tasks
        self.broadcast_task = asyncio.create_task(self._broadcast_events())
        self.cleanup_task = asyncio.create_task(self._cleanup_inactive_clients())
        
        logger.info("WebSocket Manager started successfully")
    
    async def stop(self):
        """Stop the WebSocket manager"""
        logger.info("Stopping WebSocket Manager")
        
        # Stop background tasks
        if self.broadcast_task:
            self.broadcast_task.cancel()
            try:
                await self.broadcast_task
            except asyncio.CancelledError:
                pass
        
        if self.cleanup_task:
            self.cleanup_task.cancel()
            try:
                await self.cleanup_task
            except asyncio.CancelledError:
                pass
        
        # Close all client connections
        await self._close_all_clients()
        
        logger.info("WebSocket Manager stopped")
    
    async def add_client(self, websocket: WebSocket, client_info: Dict[str, Any] = None) -> str:
        """Add a new WebSocket client"""
        client_id = self._generate_client_id()
        
        client = WebSocketClient(
            websocket=websocket,
            client_id=client_id,
            connected_at=datetime.now(timezone.utc),
            last_ping=datetime.now(timezone.utc),
            subscriptions=set()  # Subscribe to all events by default
        )
        
        self.clients[client_id] = client
        self.total_clients_connected += 1
        
        logger.info(f"New WebSocket client connected: {client_id}")
        
        # Send welcome message
        welcome_event = WebSocketEvent(
            event_type="client_connected",
            device_id="system",
            data={
                "client_id": client_id,
                "message": "Welcome to Sonoff WiFi Socket Server",
                "server_time": datetime.now(timezone.utc).isoformat(),
                "total_clients": len(self.clients)
            }
        )
        
        await self._send_to_client(client_id, welcome_event)
        
        return client_id
    
    async def remove_client(self, client_id: str):
        """Remove a WebSocket client"""
        if client_id in self.clients:
            client = self.clients[client_id]
            
            # Close WebSocket connection
            try:
                await client.websocket.close()
            except Exception as e:
                logger.warning(f"Error closing WebSocket for {client_id}: {e}")
            
            # Remove from client list
            del self.clients[client_id]
            self.total_clients_disconnected += 1
            
            logger.info(f"WebSocket client disconnected: {client_id}")
    
    async def broadcast_event(self, event: WebSocketEvent):
        """Broadcast an event to all connected clients"""
        if self.config.websocket.broadcast_events:
            await self.event_queue.put(event)
            logger.debug(f"Event queued for broadcast: {event.event_type}")
    
    async def send_to_client(self, client_id: str, event: WebSocketEvent):
        """Send an event to a specific client"""
        if client_id in self.clients:
            await self._send_to_client(client_id, event)
    
    async def send_to_clients(self, client_ids: List[str], event: WebSocketEvent):
        """Send an event to multiple specific clients"""
        for client_id in client_ids:
            if client_id in self.clients:
                await self._send_to_client(client_id, event)
    
    async def _send_to_client(self, client_id: str, event: WebSocketEvent):
        """Internal method to send event to client"""
        try:
            client = self.clients[client_id]
            
            # Convert event to JSON
            event_data = event.dict()
            event_json = json.dumps(event_data)
            
            # Send to WebSocket
            await client.websocket.send_text(event_json)
            
            # Update last ping time
            client.last_ping = datetime.now(timezone.utc)
            
            self.total_events_sent += 1
            
        except Exception as e:
            logger.error(f"Error sending event to client {client_id}: {e}")
            # Mark client for removal
            await self._mark_client_for_removal(client_id)
    
    async def _broadcast_events(self):
        """Background task to broadcast events from queue"""
        logger.info("Starting event broadcast task")
        
        while True:
            try:
                # Wait for events
                event = await self.event_queue.get()
                
                # Broadcast to all clients
                if self.clients:
                    broadcast_tasks = []
                    for client_id in self.clients:
                        task = asyncio.create_task(self._send_to_client(client_id, event))
                        broadcast_tasks.append(task)
                    
                    # Wait for all broadcasts to complete
                    if broadcast_tasks:
                        await asyncio.gather(*broadcast_tasks, return_exceptions=True)
                
                # Mark task as done
                self.event_queue.task_done()
                
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Error in broadcast task: {e}")
                await asyncio.sleep(1)  # Wait before retrying
        
        logger.info("Event broadcast task stopped")
    
    async def _cleanup_inactive_clients(self):
        """Background task to cleanup inactive clients"""
        logger.info("Starting client cleanup task")
        
        while True:
            try:
                current_time = datetime.now(timezone.utc)
                clients_to_remove = []
                
                # Check for inactive clients
                for client_id, client in self.clients.items():
                    time_since_ping = (current_time - client.last_ping).total_seconds()
                    
                    if time_since_ping > self.config.websocket.ping_timeout:
                        clients_to_remove.append(client_id)
                
                # Remove inactive clients
                for client_id in clients_to_remove:
                    logger.info(f"Removing inactive client: {client_id}")
                    await self.remove_client(client_id)
                
                # Wait before next cleanup
                await asyncio.sleep(60)  # Check every minute
                
            except asyncio.CancelledError:
                break
            except Exception as e:
                logger.error(f"Error in cleanup task: {e}")
                await asyncio.sleep(60)  # Wait before retrying
        
        logger.info("Client cleanup task stopped")
    
    async def _close_all_clients(self):
        """Close all client connections"""
        client_ids = list(self.clients.keys())
        for client_id in client_ids:
            await self.remove_client(client_id)
    
    async def _mark_client_for_removal(self, client_id: str):
        """Mark a client for removal due to errors"""
        # This could be enhanced with a removal queue
        # For now, we'll remove immediately
        await self.remove_client(client_id)
    
    def _generate_client_id(self) -> str:
        """Generate a unique client ID"""
        timestamp = int(time.time() * 1000)
        random_suffix = hash(f"{timestamp}_{len(self.clients)}") % 10000
        return f"client_{timestamp}_{random_suffix:04d}"
    
    # Device-specific event methods
    
    async def broadcast_device_status_update(self, device_info: DeviceInfo):
        """Broadcast device status update to all clients"""
        event = WebSocketEvent(
            event_type="device_status_update",
            device_id=device_info.id,
            data={
                "status": device_info.status,
                "power_state": device_info.power_state,
                "last_seen": device_info.last_seen.isoformat() if device_info.last_seen else None,
                "supports_power_monitoring": device_info.supports_power_monitoring,
                "voltage": device_info.voltage,
                "current": device_info.current,
                "power": device_info.power,
                "energy": device_info.energy
            }
        )
        
        await self.broadcast_event(event)
    
    async def broadcast_device_control(self, device_id: str, power_state: PowerState, success: bool, message: str):
        """Broadcast device control event to all clients"""
        event = WebSocketEvent(
            event_type="device_control",
            device_id=device_id,
            data={
                "power_state": power_state,
                "success": success,
                "message": message,
                "timestamp": datetime.now(timezone.utc).isoformat()
            }
        )
        
        await self.broadcast_event(event)
    
    async def broadcast_device_discovery(self, discovered_devices: List[DeviceInfo]):
        """Broadcast device discovery results to all clients"""
        event = WebSocketEvent(
            event_type="device_discovery",
            device_id="system",
            data={
                "total_devices": len(discovered_devices),
                "devices": [
                    {
                        "id": device.id,
                        "name": device.name,
                        "type": device.type,
                        "model": device.model,
                        "ip_address": device.ip_address,
                        "status": device.status,
                        "power_state": device.power_state
                    }
                    for device in discovered_devices
                ],
                "discovery_time": datetime.now(timezone.utc).isoformat()
            }
        )
        
        await self.broadcast_event(event)
    
    async def broadcast_system_status(self, status_data: Dict[str, Any]):
        """Broadcast system status to all clients"""
        event = WebSocketEvent(
            event_type="system_status",
            device_id="system",
            data={
                **status_data,
                "timestamp": datetime.now(timezone.utc).isoformat(),
                "total_clients": len(self.clients),
                "total_events_sent": self.total_events_sent
            }
        )
        
        await self.broadcast_event(event)
    
    async def broadcast_audio_event(self, audio_event):
        """Broadcast audio event to all connected clients"""
        if not self.clients:
            return
        
        # Convert audio event to WebSocket event format
        websocket_event = WebSocketEvent(
            event_type="audio_event",
            device_id="audio_system",
            data={
                "event_type": audio_event.event_type,
                "timestamp": audio_event.timestamp.isoformat(),
                "track_id": audio_event.track_id,
                "playlist_id": audio_event.playlist_id,
                "event_data": audio_event.data
            }
        )
        
        # Broadcast to all clients
        await self.broadcast_event(websocket_event)
    
    # Client management methods
    
    def get_client_count(self) -> int:
        """Get current number of connected clients"""
        return len(self.clients)
    
    def get_client_info(self, client_id: str) -> Optional[Dict[str, Any]]:
        """Get information about a specific client"""
        if client_id in self.clients:
            client = self.clients[client_id]
            return {
                "id": client.client_id,  # Fixed: use client_id instead of id
                "connected_at": client.connected_at.isoformat(),
                "last_ping": client.last_ping.isoformat(),
                "subscriptions": list(client.subscriptions)
            }
        return None
    
    def get_all_clients_info(self) -> List[Dict[str, Any]]:
        """Get information about all connected clients"""
        return [
            self.get_client_info(client_id)
            for client_id in self.clients
        ]
    
    async def update_client_subscriptions(self, client_id: str, subscriptions: List[str]):
        """Update client event subscriptions"""
        if client_id in self.clients:
            client = self.clients[client_id]
            client.subscriptions = set(subscriptions)
            
            # Send confirmation
            event = WebSocketEvent(
                event_type="subscriptions_updated",
                device_id="system",
                data={
                    "client_id": client_id,
                    "subscriptions": list(client.subscriptions),
                    "timestamp": datetime.now(timezone.utc).isoformat()
                }
            )
            
            await self._send_to_client(client_id, event)
    
    # Statistics methods
    
    def get_statistics(self) -> Dict[str, Any]:
        """Get WebSocket manager statistics"""
        return {
            "total_clients_connected": self.total_clients_connected,
            "total_clients_disconnected": self.total_clients_disconnected,
            "current_clients": len(self.clients),
            "total_events_sent": self.total_events_sent,
            "event_queue_size": self.event_queue.qsize(),
            "uptime": time.time() - self._start_time if hasattr(self, '_start_time') else 0
        }


# Global WebSocket manager instance
websocket_manager = WebSocketManager()
