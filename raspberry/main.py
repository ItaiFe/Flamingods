"""
Main Server for Sonoff WiFi Socket Server

This is the main entry point for the FastAPI server that provides:
- REST API for device control
- WebSocket for real-time updates
- Device discovery and management
- Health monitoring
"""

import asyncio
import json
import time
from contextlib import asynccontextmanager
from typing import List, Optional

from fastapi import FastAPI, HTTPException, WebSocket, WebSocketDisconnect, Depends
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import JSONResponse
import structlog

from config import get_config
from models import (
    DeviceInfo, DeviceControl, DeviceResponse, DeviceDiscoveryRequest,
    DeviceDiscoveryResponse, HealthCheck, ErrorResponse, BulkDeviceControl,
    BulkDeviceResponse, PowerState, WebSocketEvent
)
from sonoff_manager import device_manager
from websocket_manager import websocket_manager

# Configure logging
logger = structlog.get_logger()

# Server startup time
startup_time = time.time()


@asynccontextmanager
async def lifespan(app: FastAPI):
    """Manage application lifespan"""
    # Startup
    logger.info("Starting Sonoff WiFi Socket Server")
    
    try:
        # Start managers
        await device_manager.start()
        await websocket_manager.start()
        
        logger.info("Server startup completed successfully")
        yield
        
    except Exception as e:
        logger.error(f"Server startup failed: {e}")
        raise
    finally:
        # Shutdown
        logger.info("Shutting down Sonoff WiFi Socket Server")
        
        try:
            await websocket_manager.stop()
            await device_manager.stop()
        except Exception as e:
            logger.error(f"Error during shutdown: {e}")


# Create FastAPI app
app = FastAPI(
    title="Sonoff WiFi Socket Server",
    description="Server for controlling Sonoff WiFi sockets in the Midburn project",
    version="0.1.0",
    lifespan=lifespan
)

# Add CORS middleware
config = get_config()
app.add_middleware(
    CORSMiddleware,
    allow_origins=config.server.cors_origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)


# Dependency functions
def get_device_manager():
    """Get device manager instance"""
    return device_manager


def get_websocket_manager():
    """Get WebSocket manager instance"""
    return websocket_manager


# Health check endpoint
@app.get("/health", response_model=HealthCheck)
async def health_check():
    """Get server health status"""
    uptime = time.time() - startup_time
    
    return HealthCheck(
        status="healthy",
        uptime=uptime,
        version="0.1.0",
        environment="production"
    )


# Device discovery endpoints
@app.post("/discover", response_model=DeviceDiscoveryResponse)
async def discover_devices(
    request: DeviceDiscoveryRequest,
    device_mgr=Depends(get_device_manager)
):
    """Discover Sonoff devices on the network"""
    try:
        logger.info("Device discovery requested")
        
        # Perform device discovery
        discovered_devices = await device_mgr.discover_devices(
            force_refresh=request.force_refresh
        )
        
        # Broadcast discovery results
        await websocket_manager.broadcast_device_discovery(discovered_devices)
        
        return DeviceDiscoveryResponse(
            devices=discovered_devices,
            total_count=len(discovered_devices),
            online_count=device_mgr.get_online_device_count(),
            network_range=config.network.local_network,
            duration=0.0  # Could be enhanced to track actual duration
        )
        
    except Exception as e:
        logger.error(f"Device discovery failed: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/devices", response_model=List[DeviceInfo])
async def get_devices(device_mgr=Depends(get_device_manager)):
    """Get list of all discovered devices"""
    try:
        devices = [device_mgr._convert_to_device_info(device) 
                  for device in device_mgr.devices.values()]
        return devices
        
    except Exception as e:
        logger.error(f"Failed to get devices: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/devices/{device_id}", response_model=DeviceInfo)
async def get_device(device_id: str, device_mgr=Depends(get_device_manager)):
    """Get information about a specific device"""
    try:
        device_info = await device_mgr.get_device_status(device_id)
        if not device_info:
            raise HTTPException(status_code=404, detail=f"Device {device_id} not found")
        
        return device_info
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Failed to get device {device_id}: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# Device control endpoints
@app.post("/devices/{device_id}/control", response_model=DeviceResponse)
async def control_device(
    device_id: str,
    control: DeviceControl,
    device_mgr=Depends(get_device_manager),
    ws_mgr=Depends(get_websocket_manager)
):
    """Control a specific device"""
    try:
        logger.info(f"Control request for device {device_id}: {control.power}")
        
        # Send control command
        response = await device_mgr.control_device(device_id, control)
        
        # Broadcast control event
        await ws_mgr.broadcast_device_control(
            device_id, control.power, response.success, response.message
        )
        
        return response
        
    except ValueError as e:
        raise HTTPException(status_code=404, detail=str(e))
    except Exception as e:
        logger.error(f"Failed to control device {device_id}: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.post("/devices/{device_id}/power/{power_state}")
async def set_power(
    device_id: str,
    power_state: PowerState,
    device_mgr=Depends(get_device_manager),
    ws_mgr=Depends(get_websocket_manager)
):
    """Set power state for a device (simplified control)"""
    try:
        control = DeviceControl(power=power_state)
        response = await device_mgr.control_device(device_id, control)
        
        # Broadcast control event
        await ws_mgr.broadcast_device_control(
            device_id, power_state, response.success, response.message
        )
        
        return response
        
    except ValueError as e:
        raise HTTPException(status_code=404, detail=str(e))
    except Exception as e:
        logger.error(f"Failed to set power for device {device_id}: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.post("/devices/{device_id}/toggle")
async def toggle_device(
    device_id: str,
    device_mgr=Depends(get_device_manager),
    ws_mgr=Depends(get_websocket_manager)
):
    """Toggle device power state"""
    try:
        control = DeviceControl(power=PowerState.TOGGLE)
        response = await device_mgr.control_device(device_id, control)
        
        # Broadcast control event
        await ws_mgr.broadcast_device_control(
            device_id, response.power_state, response.success, response.message
        )
        
        return response
        
    except ValueError as e:
        raise HTTPException(status_code=404, detail=str(e))
    except Exception as e:
        logger.error(f"Failed to toggle device {device_id}: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# Bulk control endpoints
@app.post("/devices/bulk/control", response_model=BulkDeviceResponse)
async def bulk_control_devices(
    control: BulkDeviceControl,
    device_mgr=Depends(get_device_manager),
    ws_mgr=Depends(get_websocket_manager)
):
    """Control multiple devices simultaneously"""
    try:
        logger.info(f"Bulk control request for {len(control.devices)} devices: {control.power}")
        
        start_time = time.time()
        results = []
        successful = 0
        failed = 0
        
        # Control each device
        for device_id in control.devices:
            try:
                device_control = DeviceControl(power=control.power)
                response = await device_mgr.control_device(device_id, device_control)
                results.append(response)
                
                if response.success:
                    successful += 1
                else:
                    failed += 1
                
                # Add delay if specified
                if control.delay and device_id != control.devices[-1]:
                    await asyncio.sleep(control.delay)
                    
            except Exception as e:
                logger.error(f"Failed to control device {device_id}: {e}")
                failed += 1
                results.append(DeviceResponse(
                    success=False,
                    message=f"Error: {str(e)}",
                    device_id=device_id,
                    power_state=PowerState.OFF
                ))
        
        duration = time.time() - start_time
        
        # Create bulk response
        bulk_response = BulkDeviceResponse(
            total_devices=len(control.devices),
            successful=successful,
            failed=failed,
            results=results,
            operation_id=f"bulk_{int(time.time())}",
            duration=duration
        )
        
        # Broadcast bulk control event
        await ws_mgr.broadcast_event(WebSocketEvent(
            event_type="bulk_device_control",
            device_id="system",
            data={
                "total_devices": len(control.devices),
                "successful": successful,
                "failed": failed,
                "power_state": control.power,
                "duration": duration
            }
        ))
        
        return bulk_response
        
    except Exception as e:
        logger.error(f"Bulk control failed: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# WebSocket endpoint
@app.websocket("/ws")
async def websocket_endpoint(
    websocket: WebSocket,
    ws_mgr=Depends(get_websocket_manager)
):
    """WebSocket endpoint for real-time communication"""
    await websocket.accept()
    
    client_id = None
    try:
        # Add client
        client_id = await ws_mgr.add_client(websocket)
        
        # Handle WebSocket messages
        while True:
            try:
                # Receive message
                data = await websocket.receive_text()
                
                # Parse message
                try:
                    message = json.loads(data)
                    await _handle_websocket_message(client_id, message, ws_mgr)
                except json.JSONDecodeError:
                    logger.warning(f"Invalid JSON from client {client_id}")
                    
            except WebSocketDisconnect:
                logger.info(f"WebSocket client {client_id} disconnected")
                break
            except Exception as e:
                logger.error(f"Error handling WebSocket message from {client_id}: {e}")
                break
                
    except Exception as e:
        logger.error(f"WebSocket error for client {client_id}: {e}")
    finally:
        # Remove client
        if client_id:
            await ws_mgr.remove_client(client_id)


async def _handle_websocket_message(client_id: str, message: dict, ws_mgr):
    """Handle WebSocket messages from clients"""
    try:
        message_type = message.get('type')
        
        if message_type == 'ping':
            # Handle ping
            await ws_mgr.send_to_client(client_id, WebSocketEvent(
                event_type="pong",
                device_id="system",
                data={"timestamp": time.time()}
            ))
            
        elif message_type == 'subscribe':
            # Handle subscription updates
            subscriptions = message.get('subscriptions', [])
            await ws_mgr.update_client_subscriptions(client_id, subscriptions)
            
        elif message_type == 'get_status':
            # Handle status request
            await ws_mgr.broadcast_system_status({
                "device_count": device_manager.get_device_count(),
                "online_device_count": device_manager.get_online_device_count(),
                "client_count": ws_mgr.get_client_count()
            })
            
        else:
            logger.warning(f"Unknown message type from client {client_id}: {message_type}")
            
    except Exception as e:
        logger.error(f"Error handling WebSocket message: {e}")


# System endpoints
@app.get("/system/status")
async def get_system_status(
    device_mgr=Depends(get_device_manager),
    ws_mgr=Depends(get_websocket_manager)
):
    """Get system status information"""
    try:
        return {
            "server": {
                "status": "running",
                "uptime": time.time() - startup_time,
                "version": "0.1.0"
            },
            "devices": {
                "total": device_mgr.get_device_count(),
                "online": device_mgr.get_online_device_count()
            },
            "websocket": {
                "clients": ws_mgr.get_client_count(),
                "total_connected": ws_mgr.total_clients_connected,
                "total_disconnected": ws_mgr.total_clients_disconnected,
                "events_sent": ws_mgr.total_events_sent
            },
            "timestamp": time.time()
        }
        
    except Exception as e:
        logger.error(f"Failed to get system status: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@app.get("/system/clients")
async def get_websocket_clients(ws_mgr=Depends(get_websocket_manager)):
    """Get information about WebSocket clients"""
    try:
        return ws_mgr.get_all_clients_info()
        
    except Exception as e:
        logger.error(f"Failed to get client info: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# Error handlers
@app.exception_handler(HTTPException)
async def http_exception_handler(request, exc):
    """Handle HTTP exceptions"""
    return JSONResponse(
        status_code=exc.status_code,
        content=ErrorResponse(
            error="http_error",
            message=exc.detail,
            status_code=exc.status_code
        ).dict()
    )


@app.exception_handler(Exception)
async def general_exception_handler(request, exc):
    """Handle general exceptions"""
    logger.error(f"Unhandled exception: {exc}")
    return JSONResponse(
        status_code=500,
        content=ErrorResponse(
            error="internal_error",
            message="Internal server error",
            status_code=500
        ).dict()
    )


# Main function for running the server
def main():
    """Main function for running the server"""
    import uvicorn
    
    config = get_config()
    
    uvicorn.run(
        "main:app",
        host=config.server.host,
        port=config.server.port,
        reload=config.server.debug,
        log_level=config.server.log_level.lower()
    )


if __name__ == "__main__":
    main()
