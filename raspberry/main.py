"""
Main Server for Sonoff WiFi Socket Server with Audio System

This is the main entry point for the FastAPI server that provides:
- REST API for device control
- WebSocket for real-time updates
- Device discovery and management
- Health monitoring
- Audio playback and music management
"""

import asyncio
import json
import logging
import time
from contextlib import asynccontextmanager
from datetime import datetime, timezone
from typing import List, Optional, Dict, Any

import httpx
from fastapi import FastAPI, HTTPException, Depends, WebSocket, WebSocketDisconnect
from fastapi.responses import JSONResponse
from fastapi.middleware.cors import CORSMiddleware

from config import get_config
from models import (
    DeviceInfo, DeviceControl, DeviceResponse, PowerState,
    StageControl, StageResponse, ErrorResponse, WebSocketEvent,
    HealthCheck, DeviceDiscoveryRequest, DeviceDiscoveryResponse,
    BulkDeviceResponse, BulkDeviceControl
)
from sonoff_manager import device_manager
from websocket_manager import websocket_manager
from audio_manager import AudioManager
from audio_endpoints import router as audio_router, set_audio_manager

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)-8s] %(message)s',
    datefmt='%Y-%m-%d %H:%M:%S'
)
logger = logging.getLogger(__name__)

# Server startup time
startup_time = time.time()

# Global audio manager instance
audio_manager: Optional[AudioManager] = None

# Helper function to safely serialize error details
def safe_error_detail(error) -> str:
    """Convert any error object to a safe string representation"""
    try:
        if error is None:
            return "Unknown error"
        elif isinstance(error, str):
            return error
        elif isinstance(error, Exception):
            # Get the error message, avoiding any datetime objects
            error_msg = str(error)
            # Remove any datetime-like patterns that might cause issues
            if "datetime" in error_msg.lower():
                return f"Error: {type(error).__name__}"
            return error_msg
        else:
            return str(error)
    except Exception:
        return "Error serialization failed"


@asynccontextmanager
async def lifespan(app: FastAPI):
    """Manage application lifespan"""
    # Startup
    logger.info("Starting Sonoff WiFi Socket Server with Audio System")
    
    try:
        # Start websocket manager
        await websocket_manager.start()
        
        # Initialize and start audio manager
        global audio_manager
        config = get_config()
        audio_manager = AudioManager(config.audio)
        await audio_manager.start()
        
        # Set audio manager in endpoints
        set_audio_manager(audio_manager)
        
        # Add audio event callback to websocket manager
        audio_manager.add_event_callback(websocket_manager.broadcast_audio_event)
        
        logger.info("Server startup completed successfully")
        yield
        
    except Exception as e:
        logger.error(f"Server startup failed: {e}")
        raise
    finally:
        # Shutdown
        logger.info("Shutting down Sonoff WiFi Socket Server")
        
        try:
            # Stop audio manager
            if audio_manager:
                await audio_manager.stop()
            
            await websocket_manager.stop()
            # Stop device manager if it was started
            if device_manager.is_running():
                await device_manager.stop()
        except Exception as e:
            logger.error(f"Error during shutdown: {e}")


# Create FastAPI app
app = FastAPI(
    title="Sonoff WiFi Socket Server with Audio System",
    description="Server for controlling Sonoff WiFi sockets and audio playback in the Midburn project",
    version="0.2.0",
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


def get_audio_manager():
    """Get audio manager instance"""
    return audio_manager


# Include audio router
app.include_router(audio_router)


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
        
        # Start device manager if not already running
        if not device_mgr.is_running():
            logger.info("Starting device manager for discovery")
            await device_mgr.start()
        
        # Perform device discovery with timeout
        try:
            discovered_devices = await asyncio.wait_for(
                device_mgr.discover_devices(force_refresh=request.force_refresh),
                timeout=90.0  # 90 second timeout for the entire discovery process
            )
        except asyncio.TimeoutError:
            logger.warning("Device discovery timed out after 90 seconds, returning partial results")
            # Return any devices that were found before timeout
            discovered_devices = [device_mgr._convert_to_device_info(device) 
                                for device in device_mgr.devices.values()]
        
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
        raise HTTPException(status_code=500, detail=safe_error_detail(e))


@app.get("/devices", response_model=List[DeviceInfo])
async def get_devices(device_mgr=Depends(get_device_manager)):
    """Get list of all discovered devices"""
    try:
        # Start device manager if not already running
        if not device_mgr.is_running():
            logger.info("Starting device manager for device list")
            await device_mgr.start()
        
        devices = [device_mgr._convert_to_device_info(device) 
                  for device in device_mgr.devices.values()]
        return devices
        
    except Exception as e:
        logger.error(f"Failed to get devices: {e}")
        raise HTTPException(status_code=500, detail=safe_error_detail(e))


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
        raise HTTPException(status_code=500, detail=safe_error_detail(e))


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
        
        # Start device manager if not already running
        if not device_mgr.is_running():
            logger.info("Starting device manager for device control")
            await device_mgr.start()
        
        # Send control command
        response = await device_mgr.control_device(device_id, control)
        
        # Broadcast control event
        await ws_mgr.broadcast_device_control(
            device_id, control.power, response.success, response.message
        )
        
        return response
        
    except ValueError as e:
        raise HTTPException(status_code=404, detail=safe_error_detail(e))
    except Exception as e:
        logger.error(f"Failed to control device {device_id}: {e}")
        raise HTTPException(status_code=500, detail=safe_error_detail(e))


@app.post("/devices/{device_id}/power/{power_state}")
async def set_power(
    device_id: str,
    power_state: PowerState,
    device_mgr=Depends(get_device_manager),
    ws_mgr=Depends(get_websocket_manager)
):
    """Set power state for a device (simplified control)"""
    try:
        # Start device manager if not already running
        if not device_mgr.is_running():
            logger.info("Starting device manager for power control")
            await device_mgr.start()
        
        control = DeviceControl(power=power_state)
        response = await device_mgr.control_device(device_id, control)
        
        # Broadcast control event
        await ws_mgr.broadcast_device_control(
            device_id, power_state, response.success, response.message
        )
        
        return response
        
    except ValueError as e:
        raise HTTPException(status_code=404, detail=safe_error_detail(e))
    except Exception as e:
        logger.error(f"Failed to set power for device {device_id}: {e}")
        raise HTTPException(status_code=500, detail=safe_error_detail(e))


@app.post("/devices/{device_id}/toggle")
async def toggle_device(
    device_id: str,
    device_mgr=Depends(get_device_manager),
    ws_mgr=Depends(get_websocket_manager)
):
    """Toggle device power state"""
    try:
        # Start device manager if not already running
        if not device_mgr.is_running():
            logger.info("Starting device manager for device toggle")
            await device_mgr.start()
        
        control = DeviceControl(power=PowerState.TOGGLE)
        response = await device_mgr.control_device(device_id, control)
        
        # Broadcast control event
        await ws_mgr.broadcast_device_control(
            device_id, response.power_state, response.success, response.message
        )
        
        return response
        
    except ValueError as e:
        raise HTTPException(status_code=404, detail=safe_error_detail(e))
    except Exception as e:
        logger.error(f"Failed to toggle device {device_id}: {e}")
        raise HTTPException(status_code=500, detail=safe_error_detail(e))


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
        
        # Start device manager if not already running
        if not device_mgr.is_running():
            logger.info("Starting device manager for bulk control")
            await device_mgr.start()
        
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
        raise HTTPException(status_code=500, detail=safe_error_detail(e))


# Stage LED control endpoints
# Stage server configuration
STAGE_SERVER_BASE_URL = "http://192.168.1.209"  # Default stage server URL
STAGE_SERVER_TIMEOUT = 5.0  # Default timeout

@app.post("/stage/idle")
async def stage_idle():
    """Switch stage to IDLE lighting plan"""
    try:
        async with httpx.AsyncClient(timeout=STAGE_SERVER_TIMEOUT) as client:
            response = await client.post(f"{STAGE_SERVER_BASE_URL}/idle")
            
        if response.status_code == 200:
            return {"status": "success", "plan": "idle", "message": "Stage switched to IDLE plan"}
        else:
            raise HTTPException(status_code=response.status_code, detail=f"Stage server error: {response.text}")
            
    except httpx.TimeoutException:
        raise HTTPException(status_code=408, detail="Stage server timeout")
    except httpx.ConnectError:
        raise HTTPException(status_code=503, detail="Cannot connect to stage server")
    except Exception as e:
        logger.error(f"Failed to control stage IDLE: {e}")
        raise HTTPException(status_code=500, detail=safe_error_detail(e))


@app.post("/stage/skip")
async def stage_skip():
    """Switch stage to SKIP lighting plan"""
    try:
        async with httpx.AsyncClient(timeout=STAGE_SERVER_TIMEOUT) as client:
            response = await client.post(f"{STAGE_SERVER_BASE_URL}/skip")
            
        if response.status_code == 200:
            return {"status": "success", "plan": "skip", "message": "Stage switched to SKIP plan"}
        else:
            raise HTTPException(status_code=response.status_code, detail=f"Stage server error: {response.text}")
            
    except httpx.TimeoutException:
        raise HTTPException(status_code=408, detail="Stage server timeout")
    except httpx.ConnectError:
        raise HTTPException(status_code=503, detail="Cannot connect to stage server")
    except Exception as e:
        logger.error(f"Failed to control stage SKIP: {e}")
        raise HTTPException(status_code=500, detail=safe_error_detail(e))


@app.post("/stage/show")
async def stage_show():
    """Switch stage to SHOW lighting plan"""
    try:
        async with httpx.AsyncClient(timeout=STAGE_SERVER_TIMEOUT) as client:
            response = await client.post(f"{STAGE_SERVER_BASE_URL}/show")
            
        if response.status_code == 200:
            return {"status": "success", "plan": "show", "message": "Stage switched to SHOW plan"}
        else:
            raise HTTPException(status_code=response.status_code, detail=f"Stage server error: {response.text}")
            
    except httpx.TimeoutException:
        raise HTTPException(status_code=408, detail="Stage server timeout")
    except httpx.ConnectError:
        raise HTTPException(status_code=503, detail="Cannot connect to stage server")
    except Exception as e:
        logger.error(f"Failed to control stage SHOW: {e}")
        raise HTTPException(status_code=500, detail=safe_error_detail(e))


@app.post("/stage/special")
async def stage_special():
    """Switch stage to SPECIAL lighting plan"""
    try:
        async with httpx.AsyncClient(timeout=STAGE_SERVER_TIMEOUT) as client:
            response = await client.post(f"{STAGE_SERVER_BASE_URL}/special")
            
        if response.status_code == 200:
            return {"status": "success", "plan": "special", "message": "Stage switched to SPECIAL plan"}
        else:
            raise HTTPException(status_code=response.status_code, detail=f"Stage server error: {response.text}")
            
    except httpx.TimeoutException:
        raise HTTPException(status_code=408, detail="Stage server timeout")
    except httpx.ConnectError:
        raise HTTPException(status_code=503, detail="Cannot connect to stage server")
    except Exception as e:
        logger.error(f"Failed to control stage SPECIAL: {e}")
        raise HTTPException(status_code=500, detail=safe_error_detail(e))


@app.get("/stage/status")
async def stage_status():
    """Get stage device status"""
    try:
        async with httpx.AsyncClient(timeout=STAGE_SERVER_TIMEOUT) as client:
            response = await client.get(f"{STAGE_SERVER_BASE_URL}/status")
            
        if response.status_code == 200:
            return response.json()
        else:
            # Ensure error detail is serializable
            error_detail = str(response.text) if response.text else "Unknown stage server error"
            raise HTTPException(status_code=response.status_code, detail=error_detail)
            
    except httpx.TimeoutException:
        raise HTTPException(status_code=408, detail="Stage server timeout")
    except httpx.ConnectError:
        raise HTTPException(status_code=503, detail="Cannot connect to stage server")
    except Exception as e:
        logger.error(f"Failed to get stage status: {e}")
        # Ensure error detail is serializable
        error_detail = str(e) if e else "Unknown error"
        raise HTTPException(status_code=500, detail=error_detail)


@app.get("/stage/health")
async def stage_health():
    """Get stage device health"""
    try:
        async with httpx.AsyncClient(timeout=STAGE_SERVER_TIMEOUT) as client:
            response = await client.get(f"{STAGE_SERVER_BASE_URL}/health")
            
        if response.status_code == 200:
            return {"status": "healthy", "stage_server": "OK"}
        else:
            # Ensure error detail is serializable
            error_detail = str(response.text) if response.text else "Unknown stage server error"
            raise HTTPException(status_code=response.status_code, detail=error_detail)
            
    except httpx.TimeoutException:
        raise HTTPException(status_code=408, detail="Stage server timeout")
    except httpx.ConnectError:
        raise HTTPException(status_code=503, detail="Cannot connect to stage server")
    except Exception as e:
        logger.error(f"Failed to get stage health: {e}")
        # Ensure error detail is serializable
        error_detail = str(e) if e else "Unknown error"
        raise HTTPException(status_code=500, detail=error_detail)


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
        # Start device manager if not already running (for status info)
        if not device_mgr.is_running():
            logger.info("Starting device manager for system status")
            await device_mgr.start()
        
        return {
            "server": {
                "status": "running",
                "uptime": time.time() - startup_time,
                "version": "0.2.0"
            },
            "devices": {
                "total": device_mgr.get_device_count(),
                "online": device_mgr.get_online_device_count()
            },
            "stage": {
                "capabilities": ["idle", "skip", "show", "special"],
                "status": "available",
                "version": "1.0.0"
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
        raise HTTPException(status_code=500, detail=safe_error_detail(e))


@app.get("/system/clients")
async def get_websocket_clients(ws_mgr=Depends(get_websocket_manager)):
    """Get information about WebSocket clients"""
    try:
        return ws_mgr.get_all_clients_info()
        
    except Exception as e:
        logger.error(f"Failed to get client info: {e}")
        raise HTTPException(status_code=500, detail=safe_error_detail(e))


# Error handlers
@app.exception_handler(HTTPException)
async def http_exception_handler(request, exc):
    """Handle HTTP exceptions"""
    try:
        return JSONResponse(
            status_code=exc.status_code,
            content=ErrorResponse(
                error="http_error",
                message=safe_error_detail(exc.detail),  # Use safe error detail
                status_code=exc.status_code
            ).dict()
        )
    except Exception as e:
        # Fallback error response if ErrorResponse fails
        logger.error(f"Error in HTTP exception handler: {e}")
        return JSONResponse(
            status_code=exc.status_code,
            content={
                "error": "http_error",
                "message": safe_error_detail(exc.detail),  # Use safe error detail
                "status_code": exc.status_code
            }
        )


@app.exception_handler(Exception)
async def general_exception_handler(request, exc):
    """Handle general exceptions"""
    logger.error(f"Unhandled exception: {exc}")
    try:
        return JSONResponse(
            status_code=500,
            content=ErrorResponse(
                error="internal_error",
                message="Internal server error",
                status_code=500
            ).dict()
        )
    except Exception as e:
        # Fallback error response if ErrorResponse fails
        logger.error(f"Error in general exception handler: {e}")
        return JSONResponse(
            status_code=500,
            content={
                "error": "internal_error",
                "message": "Internal server error",
                "status_code": 500
            }
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
