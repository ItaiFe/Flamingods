"""
Audio API endpoints for music playback system

This module provides REST API endpoints for controlling the audio system including:
- Playback control (play, pause, stop, next, previous)
- Volume control and mute
- Playlist management
- Library information and statistics
- File upload for new songs
"""

from typing import List, Optional
from fastapi import APIRouter, HTTPException, Depends, Query, UploadFile, File, Form
from fastapi.responses import JSONResponse
import structlog
import os
import shutil
from pathlib import Path
from datetime import datetime

from audio_models import (
    AudioControl, AudioResponse, PlaybackStatus, TrackInfo, 
    PlaylistInfo, AudioStats, AudioConfig
)
from audio_manager import AudioManager

logger = structlog.get_logger()

# Create router
router = APIRouter(prefix="/audio", tags=["audio"])

# Global audio manager instance
audio_manager: Optional[AudioManager] = None


def get_audio_manager() -> AudioManager:
    """Get audio manager instance"""
    if not audio_manager:
        raise HTTPException(status_code=503, detail="Audio system not initialized")
    return audio_manager


def set_audio_manager(manager: AudioManager):
    """Set the global audio manager instance"""
    global audio_manager
    audio_manager = manager


# File upload endpoints
@router.post("/upload", response_model=AudioResponse)
async def upload_song(
    file: UploadFile = File(..., description="Audio file to upload"),
    category: Optional[str] = Form(None, description="Category/folder for the song"),
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Upload a new song to the music library"""
    try:
        # Validate file type
        if not file.filename:
            raise HTTPException(status_code=400, detail="No filename provided")
        
        # Check file extension
        file_ext = Path(file.filename).suffix.lower().lstrip('.')
        supported_formats = [fmt.value for fmt in audio_mgr.config.supported_formats]
        
        if file_ext not in supported_formats:
            raise HTTPException(
                status_code=400, 
                detail=f"Unsupported file format: {file_ext}. Supported formats: {', '.join(supported_formats)}"
            )
        
        # Determine upload path
        config = audio_mgr.config
        if category:
            # Upload to specific category folder
            upload_dir = Path(config.music_folder) / category
        else:
            # Upload to main music folder
            upload_dir = Path(config.music_folder)
        
        # Create directory if it doesn't exist
        upload_dir.mkdir(parents=True, exist_ok=True)
        
        # Generate safe filename
        safe_filename = _generate_safe_filename(file.filename)
        upload_path = upload_dir / safe_filename
        
        # Check if file already exists
        if upload_path.exists():
            # Generate unique filename
            counter = 1
            while upload_path.exists():
                name_without_ext = Path(safe_filename).stem
                ext = Path(safe_filename).suffix
                safe_filename = f"{name_without_ext}_{counter}{ext}"
                upload_path = upload_dir / safe_filename
                counter += 1
        
        # Save uploaded file
        try:
            with open(upload_path, "wb") as buffer:
                shutil.copyfileobj(file.file, buffer)
        except Exception as e:
            logger.error(f"Failed to save uploaded file: {e}")
            raise HTTPException(status_code=500, detail="Failed to save uploaded file")
        
        # Scan the new file to add it to the library
        try:
            # Extract track info from the uploaded file
            track_info = await audio_mgr._extract_track_info(upload_path)
            if track_info:
                # Add to tracks dictionary
                audio_mgr.tracks[track_info.id] = track_info
                logger.info(f"Added new track to library: {track_info.title}")
                
                # Update library statistics
                audio_mgr.last_scan_time = datetime.utcnow()
                
                return AudioResponse(
                    success=True,
                    message=f"Song uploaded successfully: {track_info.title}",
                    data={
                        "track_id": track_info.id,
                        "title": track_info.title,
                        "artist": track_info.artist,
                        "filename": safe_filename,
                        "filepath": str(upload_path),
                        "category": category or "main",
                        "size_bytes": track_info.size_bytes,
                        "duration_seconds": track_info.duration_seconds
                    }
                )
            else:
                # File was saved but couldn't be processed
                return AudioResponse(
                    success=False,
                    message="File uploaded but could not be processed as audio",
                    error="processing_failed"
                )
                
        except Exception as e:
            logger.error(f"Failed to process uploaded file: {e}")
            # File was uploaded but processing failed
            return AudioResponse(
                success=False,
                message="File uploaded but processing failed",
                error=str(e)
            )
            
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Upload error: {e}")
        raise HTTPException(status_code=500, detail=f"Upload failed: {str(e)}")


@router.post("/upload/batch", response_model=AudioResponse)
async def upload_multiple_songs(
    files: List[UploadFile] = File(..., description="Multiple audio files to upload"),
    category: Optional[str] = Form(None, description="Category/folder for the songs"),
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Upload multiple songs to the music library"""
    try:
        if not files:
            raise HTTPException(status_code=400, detail="No files provided")
        
        config = audio_mgr.config
        supported_formats = [fmt.value for fmt in audio_mgr.config.supported_formats]
        
        # Determine upload path
        if category:
            upload_dir = Path(config.music_folder) / category
        else:
            upload_dir = Path(config.music_folder)
        
        # Create directory if it doesn't exist
        upload_dir.mkdir(parents=True, exist_ok=True)
        
        uploaded_files = []
        failed_files = []
        
        for file in files:
            try:
                if not file.filename:
                    failed_files.append({"filename": "unknown", "error": "No filename provided"})
                    continue
                
                # Check file extension
                file_ext = Path(file.filename).suffix.lower().lstrip('.')
                if file_ext not in supported_formats:
                    failed_files.append({
                        "filename": file.filename, 
                        "error": f"Unsupported format: {file_ext}"
                    })
                    continue
                
                # Generate safe filename
                safe_filename = _generate_safe_filename(file.filename)
                upload_path = upload_dir / safe_filename
                
                # Check if file already exists
                if upload_path.exists():
                    counter = 1
                    while upload_path.exists():
                        name_without_ext = Path(safe_filename).stem
                        ext = Path(safe_filename).suffix
                        safe_filename = f"{name_without_ext}_{counter}{ext}"
                        upload_path = upload_dir / safe_filename
                        counter += 1
                
                # Save uploaded file
                with open(upload_path, "wb") as buffer:
                    shutil.copyfileobj(file.file, buffer)
                
                # Process the file
                track_info = await audio_mgr._extract_track_info(upload_path)
                if track_info:
                    audio_mgr.tracks[track_info.id] = track_info
                    uploaded_files.append({
                        "track_id": track_info.id,
                        "title": track_info.title,
                        "artist": track_info.artist,
                        "filename": safe_filename,
                        "size_bytes": track_info.size_bytes
                    })
                else:
                    failed_files.append({
                        "filename": file.filename,
                        "error": "Could not process as audio file"
                    })
                    
            except Exception as e:
                failed_files.append({
                    "filename": file.filename,
                    "error": str(e)
                })
        
        # Update library statistics
        if uploaded_files:
            audio_mgr.last_scan_time = datetime.utcnow()
        
        return AudioResponse(
            success=True,
            message=f"Batch upload completed: {len(uploaded_files)} successful, {len(failed_files)} failed",
            data={
                "uploaded_count": len(uploaded_files),
                "failed_count": len(failed_files),
                "uploaded_files": uploaded_files,
                "failed_files": failed_files,
                "category": category or "main"
            }
        )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Batch upload error: {e}")
        raise HTTPException(status_code=500, detail=f"Batch upload failed: {str(e)}")


@router.delete("/tracks/{track_id}", response_model=AudioResponse)
async def delete_song(
    track_id: str,
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Delete a song from the music library"""
    try:
        # Get track information
        track = audio_mgr.get_track(track_id)
        if not track:
            raise HTTPException(status_code=404, detail=f"Track not found: {track_id}")
        
        # Check if track is currently playing
        if audio_mgr.current_track and audio_mgr.current_track.id == track_id:
            # Stop playback first
            await audio_mgr.stop_playback()
        
        # Remove from tracks dictionary
        del audio_mgr.tracks[track_id]
        
        # Remove from current playlist if present
        if audio_mgr.current_playlist:
            audio_mgr.current_playlist.tracks = [
                t for t in audio_mgr.current_playlist.tracks if t.id != track_id
            ]
        
        # Remove from track queue if present
        audio_mgr.track_queue = [
            t for t in audio_mgr.track_queue if t.id != track_id
        ]
        
        # Delete the actual file
        try:
            file_path = Path(track.filepath)
            if file_path.exists():
                file_path.unlink()
                logger.info(f"Deleted audio file: {track.filepath}")
            else:
                logger.warning(f"Audio file not found on disk: {track.filepath}")
        except Exception as e:
            logger.error(f"Failed to delete audio file: {e}")
            # Continue even if file deletion fails
        
        return AudioResponse(
            success=True,
            message=f"Song deleted successfully: {track.title}",
            data={
                "track_id": track_id,
                "title": track.title,
                "filename": track.filename
            }
        )
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Delete song error: {e}")
        raise HTTPException(status_code=500, detail=f"Delete failed: {str(e)}")


@router.post("/scan/uploaded", response_model=AudioResponse)
async def scan_uploaded_files(
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Scan for newly uploaded files and add them to the library"""
    try:
        # Scan music library for new files
        scan_result = await audio_mgr.scan_music_library()
        
        if scan_result.success:
            return AudioResponse(
                success=True,
                message="Uploaded files scan completed",
                data={
                    "tracks_count": scan_result.data.get('tracks_count', 0),
                    "playlists_count": scan_result.data.get('playlists_count', 0),
                    "scan_duration": scan_result.data.get('scan_duration', 0)
                }
            )
        else:
            return scan_result
            
    except Exception as e:
        logger.error(f"Scan uploaded files error: {e}")
        raise HTTPException(status_code=500, detail=f"Scan failed: {str(e)}")


def _generate_safe_filename(filename: str) -> str:
    """Generate a safe filename for uploads"""
    # Remove or replace unsafe characters
    unsafe_chars = '<>:"/\\|?*'
    safe_filename = filename
    
    for char in unsafe_chars:
        safe_filename = safe_filename.replace(char, '_')
    
    # Limit filename length
    if len(safe_filename) > 200:
        name_part = Path(safe_filename).stem[:180]
        ext_part = Path(safe_filename).suffix
        safe_filename = name_part + ext_part
    
    return safe_filename


# Playback control endpoints
@router.post("/play", response_model=AudioResponse)
async def play_audio(
    control: AudioControl,
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Play audio based on control parameters"""
    try:
        if control.action == "play_track" and control.track_id:
            # Play specific track
            response = await audio_mgr.play_track(control.track_id)
        elif control.action == "play_playlist" and control.playlist_id:
            # Load and play playlist
            response = await audio_mgr.load_playlist(control.playlist_id)
            if response.success:
                # Start playing first track
                playlist = audio_mgr.get_playlist(control.playlist_id)
                if playlist and playlist.tracks:
                    response = await audio_mgr.play_track(playlist.tracks[0].id)
        elif control.action == "resume":
            # Resume paused playback
            response = await audio_mgr.resume_playback()
        else:
            # Resume or start playback
            if audio_mgr.playback_state.value == "paused":
                response = await audio_mgr.resume_playback()
            else:
                # Start playing first available track
                tracks = audio_mgr.get_tracks()
                if tracks:
                    response = await audio_mgr.play_track(tracks[0].id)
                else:
                    response = AudioResponse(
                        success=False,
                        message="No tracks available to play",
                        error="no_tracks"
                    )
        
        return response
        
    except Exception as e:
        logger.error(f"Play audio error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/pause", response_model=AudioResponse)
async def pause_audio(audio_mgr: AudioManager = Depends(get_audio_manager)):
    """Pause current playback"""
    try:
        response = await audio_mgr.pause_playback()
        return response
    except Exception as e:
        logger.error(f"Pause audio error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/stop", response_model=AudioResponse)
async def stop_audio(audio_mgr: AudioManager = Depends(get_audio_manager)):
    """Stop current playback"""
    try:
        response = await audio_mgr.stop_playback()
        return response
    except Exception as e:
        logger.error(f"Stop audio error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/next", response_model=AudioResponse)
async def next_track(audio_mgr: AudioManager = Depends(get_audio_manager)):
    """Play next track in playlist"""
    try:
        response = await audio_mgr.next_track()
        return response
    except Exception as e:
        logger.error(f"Next track error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/previous", response_model=AudioResponse)
async def previous_track(audio_mgr: AudioManager = Depends(get_audio_manager)):
    """Play previous track in playlist"""
    try:
        response = await audio_mgr.previous_track()
        return response
    except Exception as e:
        logger.error(f"Previous track error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# Volume control endpoints
@router.post("/volume/{volume_level}", response_model=AudioResponse)
async def set_volume(
    volume_level: int,
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Set audio volume (0-100)"""
    try:
        if not 0 <= volume_level <= 100:
            raise HTTPException(status_code=400, detail="Volume must be between 0 and 100")
        
        response = audio_mgr.set_volume(volume_level)
        return response
    except Exception as e:
        logger.error(f"Set volume error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/mute", response_model=AudioResponse)
async def toggle_mute(audio_mgr: AudioManager = Depends(get_audio_manager)):
    """Toggle mute state"""
    try:
        response = audio_mgr.toggle_mute()
        return response
    except Exception as e:
        logger.error(f"Toggle mute error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# Status and information endpoints
@router.get("/status", response_model=PlaybackStatus)
async def get_playback_status(audio_mgr: AudioManager = Depends(get_audio_manager)):
    """Get current playback status"""
    try:
        return audio_mgr.get_playback_status()
    except Exception as e:
        logger.error(f"Get status error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/tracks", response_model=List[TrackInfo])
async def get_tracks(audio_mgr: AudioManager = Depends(get_audio_manager)):
    """Get all tracks in library"""
    try:
        return audio_mgr.get_tracks()
    except Exception as e:
        logger.error(f"Get tracks error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/tracks/{track_id}", response_model=TrackInfo)
async def get_track(
    track_id: str,
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Get specific track information"""
    try:
        track = audio_mgr.get_track(track_id)
        if not track:
            raise HTTPException(status_code=404, detail=f"Track not found: {track_id}")
        return track
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Get track error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/playlists", response_model=List[PlaylistInfo])
async def get_playlists(audio_mgr: AudioManager = Depends(get_audio_manager)):
    """Get all playlists"""
    try:
        return audio_mgr.get_playlists()
    except Exception as e:
        logger.error(f"Get playlists error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/playlists/{playlist_id}", response_model=PlaylistInfo)
async def get_playlist(
    playlist_id: str,
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Get specific playlist information"""
    try:
        playlist = audio_mgr.get_playlist(playlist_id)
        if not playlist:
            raise HTTPException(status_code=404, detail=f"Playlist not found: {playlist_id}")
        return playlist
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Get playlist error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# Library management endpoints
@router.post("/scan", response_model=AudioResponse)
async def scan_library(
    force_refresh: bool = Query(False, description="Force refresh library scan"),
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Scan music library for new tracks"""
    try:
        response = await audio_mgr.scan_music_library()
        return response
    except Exception as e:
        logger.error(f"Scan library error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.get("/stats", response_model=AudioStats)
async def get_audio_stats(audio_mgr: AudioManager = Depends(get_audio_manager)):
    """Get audio system statistics"""
    try:
        return audio_mgr.get_audio_stats()
    except Exception as e:
        logger.error(f"Get stats error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# Playlist management endpoints
@router.post("/playlists/{playlist_id}/load", response_model=AudioResponse)
async def load_playlist(
    playlist_id: str,
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Load a playlist for playback"""
    try:
        response = await audio_mgr.load_playlist(playlist_id)
        return response
    except Exception as e:
        logger.error(f"Load playlist error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


@router.post("/playlists/{playlist_id}/play", response_model=AudioResponse)
async def play_playlist(
    playlist_id: str,
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Load and start playing a playlist"""
    try:
        # Load playlist
        response = await audio_mgr.load_playlist(playlist_id)
        if not response.success:
            return response
        
        # Start playing first track
        playlist = audio_mgr.get_playlist(playlist_id)
        if playlist and playlist.tracks:
            response = await audio_mgr.play_track(playlist.tracks[0].id)
        
        return response
    except Exception as e:
        logger.error(f"Play playlist error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# Control endpoints with custom actions
@router.post("/control", response_model=AudioResponse)
async def audio_control(
    control: AudioControl,
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Execute audio control command"""
    try:
        action = control.action.lower()
        
        if action == "play":
            if control.track_id:
                response = await audio_mgr.play_track(control.track_id)
            elif control.playlist_id:
                response = await audio_mgr.load_playlist(control.playlist_id)
                if response.success and audio_mgr.current_playlist.tracks:
                    response = await audio_mgr.play_track(audio_mgr.current_playlist.tracks[0].id)
            else:
                # Resume or start playback
                if audio_mgr.playback_state.value == "paused":
                    response = await audio_mgr.resume_playback()
                else:
                    tracks = audio_mgr.get_tracks()
                    if tracks:
                        response = await audio_mgr.play_track(tracks[0].id)
                    else:
                        response = AudioResponse(
                            success=False,
                            message="No tracks available to play",
                            error="no_tracks"
                        )
        
        elif action == "pause":
            response = await audio_mgr.pause_playback()
        
        elif action == "stop":
            response = await audio_mgr.stop_playback()
        
        elif action == "next":
            response = await audio_mgr.next_track()
        
        elif action == "previous":
            response = await audio_mgr.previous_track()
        
        elif action == "volume":
            if control.volume is not None:
                response = audio_mgr.set_volume(control.volume)
            else:
                response = AudioResponse(
                    success=False,
                    message="Volume level not specified",
                    error="missing_volume"
                )
        
        elif action == "mute":
            response = audio_mgr.toggle_mute()
        
        elif action == "shuffle":
            if control.shuffle is not None:
                audio_mgr.shuffle = control.shuffle
                response = AudioResponse(
                    success=True,
                    message=f"Shuffle {'enabled' if control.shuffle else 'disabled'}",
                    data={"shuffle": control.shuffle}
                )
            else:
                response = AudioResponse(
                    success=False,
                    message="Shuffle value not specified",
                    error="missing_shuffle"
                )
        
        elif action == "repeat":
            if control.repeat is not None:
                audio_mgr.repeat = control.repeat
                response = AudioResponse(
                    success=True,
                    message=f"Repeat {'enabled' if control.repeat else 'disabled'}",
                    data={"repeat": control.repeat}
                )
            else:
                response = AudioResponse(
                    success=False,
                    message="Repeat value not specified",
                    error="missing_repeat"
                )
        
        else:
            response = AudioResponse(
                success=False,
                message=f"Unknown action: {action}",
                error="unknown_action"
            )
        
        return response
        
    except Exception as e:
        logger.error(f"Audio control error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


# Health check endpoint
@router.get("/health")
async def audio_health_check(audio_mgr: AudioManager = Depends(get_audio_manager)):
    """Check audio system health"""
    try:
        status = audio_mgr.get_playback_status()
        stats = audio_mgr.get_audio_stats()
        
        return {
            "status": "healthy",
            "audio_system": {
                "initialized": audio_mgr.is_initialized,
                "playback_state": status.state.value,
                "volume": status.volume,
                "muted": status.muted
            },
            "library": {
                "tracks_count": stats.total_tracks,
                "playlists_count": stats.total_playlists,
                "last_scan": stats.last_scan.isoformat() if stats.last_scan else None
            },
            "system": {
                "uptime_seconds": stats.uptime_seconds,
                "errors_count": stats.errors_count
            }
        }
        
    except Exception as e:
        logger.error(f"Audio health check error: {e}")
        return {
            "status": "unhealthy",
            "error": str(e)
        }

# Search and selection endpoints
@router.get("/tracks/search", response_model=List[TrackInfo])
async def search_tracks(
    query: Optional[str] = Query(None, description="Search query for track title, artist, or album"),
    artist: Optional[str] = Query(None, description="Filter by artist name"),
    album: Optional[str] = Query(None, description="Filter by album name"),
    genre: Optional[str] = Query(None, description="Filter by music genre"),
    format: Optional[str] = Query(None, description="Filter by audio format"),
    duration_min: Optional[float] = Query(None, description="Minimum duration in seconds"),
    duration_max: Optional[float] = Query(None, description="Maximum duration in seconds"),
    limit: Optional[int] = Query(100, description="Maximum number of results to return"),
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Search and filter tracks from the music library"""
    try:
        tracks = audio_mgr.get_tracks()
        
        if not tracks:
            return []
        
        # Apply filters
        filtered_tracks = tracks
        
        # Text search across title, artist, and album
        if query:
            query_lower = query.lower()
            filtered_tracks = [
                track for track in filtered_tracks
                if (track.title and query_lower in track.title.lower()) or
                   (track.artist and query_lower in track.artist.lower()) or
                   (track.album and query_lower in track.album.lower())
            ]
        
        # Filter by artist
        if artist:
            artist_lower = artist.lower()
            filtered_tracks = [
                track for track in filtered_tracks
                if track.artist and artist_lower in track.artist.lower()
            ]
        
        # Filter by album
        if album:
            album_lower = album.lower()
            filtered_tracks = [
                track for track in filtered_tracks
                if track.album and album_lower in track.album.lower()
            ]
        
        # Filter by genre
        if genre:
            genre_lower = genre.lower()
            filtered_tracks = [
                track for track in filtered_tracks
                if track.genre and genre_lower in track.genre.lower()
            ]
        
        # Filter by format
        if format:
            format_lower = format.lower()
            filtered_tracks = [
                track for track in filtered_tracks
                if track.format.value.lower() == format_lower
            ]
        
        # Filter by duration range
        if duration_min is not None:
            filtered_tracks = [
                track for track in filtered_tracks
                if track.duration_seconds and track.duration_seconds >= duration_min
            ]
        
        if duration_max is not None:
            filtered_tracks = [
                track for track in filtered_tracks
                if track.duration_seconds and track.duration_seconds <= duration_max
            ]
        
        # Apply limit
        if limit and limit > 0:
            filtered_tracks = filtered_tracks[:limit]
        
        return filtered_tracks
        
    except Exception as e:
        logger.error(f"Search tracks error: {e}")
        raise HTTPException(status_code=500, detail=f"Search failed: {str(e)}")


@router.get("/tracks/random", response_model=TrackInfo)
async def get_random_track(
    genre: Optional[str] = Query(None, description="Filter by music genre"),
    artist: Optional[str] = Query(None, description="Filter by artist name"),
    duration_min: Optional[float] = Query(None, description="Minimum duration in seconds"),
    duration_max: Optional[float] = Query(None, description="Maximum duration in seconds"),
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Get a random track from the library with optional filters"""
    try:
        tracks = audio_mgr.get_tracks()
        
        if not tracks:
            raise HTTPException(status_code=404, detail="No tracks available")
        
        # Apply filters
        filtered_tracks = tracks
        
        if genre:
            genre_lower = genre.lower()
            filtered_tracks = [
                track for track in filtered_tracks
                if track.genre and genre_lower in track.genre.lower()
            ]
        
        if artist:
            artist_lower = artist.lower()
            filtered_tracks = [
                track for track in filtered_tracks
                if track.artist and artist_lower in track.artist.lower()
            ]
        
        if duration_min is not None:
            filtered_tracks = [
                track for track in filtered_tracks
                if track.duration_seconds and track.duration_seconds >= duration_min
            ]
        
        if duration_max is not None:
            filtered_tracks = [
                track for track in filtered_tracks
                if track.duration_seconds and track.duration_seconds <= duration_max
            ]
        
        if not filtered_tracks:
            raise HTTPException(
                status_code=404, 
                detail="No tracks match the specified criteria"
            )
        
        # Select random track
        import random
        random_track = random.choice(filtered_tracks)
        
        return random_track
        
    except HTTPException:
        raise
    except Exception as e:
        logger.error(f"Get random track error: {e}")
        raise HTTPException(status_code=500, detail=f"Random track selection failed: {str(e)}")


@router.get("/tracks/popular", response_model=List[TrackInfo])
async def get_popular_tracks(
    limit: int = Query(10, ge=1, le=100, description="Number of popular tracks to return"),
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Get the most popular tracks based on play count"""
    try:
        tracks = audio_mgr.get_tracks()
        
        if not tracks:
            return []
        
        # Sort by play count (descending)
        sorted_tracks = sorted(
            tracks, 
            key=lambda x: x.play_count, 
            reverse=True
        )
        
        # Return top tracks
        return sorted_tracks[:limit]
        
    except Exception as e:
        logger.error(f"Get popular tracks error: {e}")
        raise HTTPException(status_code=500, detail=f"Popular tracks selection failed: {str(e)}")


@router.get("/tracks/recent", response_model=List[TrackInfo])
async def get_recently_played_tracks(
    limit: int = Query(10, ge=1, le=100, description="Number of recent tracks to return"),
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Get recently played tracks"""
    try:
        tracks = audio_mgr.get_tracks()
        
        if not tracks:
            return []
        
        # Filter tracks that have been played and sort by last played time
        played_tracks = [
            track for track in tracks 
            if track.last_played is not None
        ]
        
        if not played_tracks:
            return []
        
        # Sort by last played time (most recent first)
        sorted_tracks = sorted(
            played_tracks, 
            key=lambda x: x.last_played, 
            reverse=True
        )
        
        # Return recent tracks
        return sorted_tracks[:limit]
        
    except Exception as e:
        logger.error(f"Get recent tracks error: {e}")
        raise HTTPException(status_code=500, detail=f"Recent tracks selection failed: {str(e)}")


@router.get("/tracks/by-category/{category}", response_model=List[TrackInfo])
async def get_tracks_by_category(
    category: str,
    limit: Optional[int] = Query(None, ge=1, le=100, description="Maximum number of tracks to return"),
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Get tracks from a specific category (folder)"""
    try:
        tracks = audio_mgr.get_tracks()
        
        if not tracks:
            return []
        
        # Filter tracks by category (folder)
        category_tracks = []
        for track in tracks:
            track_path = Path(track.filepath)
            # Check if track is in the specified category folder
            if category.lower() in track_path.parts and track_path.parts.index(category.lower()) < len(track_path.parts) - 1:
                category_tracks.append(track)
        
        # Apply limit if specified
        if limit and limit > 0:
            category_tracks = category_tracks[:limit]
        
        return category_tracks
        
    except Exception as e:
        logger.error(f"Get tracks by category error: {e}")
        raise HTTPException(status_code=500, detail=f"Category tracks selection failed: {str(e)}")


@router.get("/tracks/selection/{selection_type}", response_model=List[TrackInfo])
async def get_track_selection(
    selection_type: str,
    count: int = Query(5, ge=1, le=50, description="Number of tracks to select"),
    genre: Optional[str] = Query(None, description="Filter by music genre"),
    artist: Optional[str] = Query(None, description="Filter by artist name"),
    duration_min: Optional[float] = Query(None, description="Minimum duration in seconds"),
    duration_max: Optional[float] = Query(None, description="Maximum duration in seconds"),
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Get a selection of tracks based on various criteria"""
    try:
        tracks = audio_mgr.get_tracks()
        
        if not tracks:
            return []
        
        # Apply filters
        filtered_tracks = tracks
        
        if genre:
            genre_lower = genre.lower()
            filtered_tracks = [
                track for track in filtered_tracks
                if track.genre and genre_lower in track.genre.lower()
            ]
        
        if artist:
            artist_lower = artist.lower()
            filtered_tracks = [
                track for track in filtered_tracks
                if track.artist and artist_lower in artist_lower
            ]
        
        if duration_min is not None:
            filtered_tracks = [
                track for track in filtered_tracks
                if track.duration_seconds and track.duration_seconds >= duration_min
            ]
        
        if duration_max is not None:
            filtered_tracks = [
                track for track in filtered_tracks
                if track.duration_seconds and track.duration_seconds <= duration_max
            ]
        
        if not filtered_tracks:
            return []
        
        # Apply selection logic based on type
        if selection_type == "random":
            import random
            selected_tracks = random.sample(filtered_tracks, min(count, len(filtered_tracks)))
        elif selection_type == "popular":
            # Sort by play count and take top tracks
            sorted_tracks = sorted(filtered_tracks, key=lambda x: x.play_count, reverse=True)
            selected_tracks = sorted_tracks[:count]
        elif selection_type == "recent":
            # Sort by last played time and take recent tracks
            played_tracks = [t for t in filtered_tracks if t.last_played]
            if played_tracks:
                sorted_tracks = sorted(played_tracks, key=lambda x: x.last_played, reverse=True)
                selected_tracks = sorted_tracks[:count]
            else:
                selected_tracks = []
        elif selection_type == "diverse":
            # Select diverse tracks (different artists, genres)
            selected_tracks = []
            artists_seen = set()
            genres_seen = set()
            
            for track in filtered_tracks:
                if len(selected_tracks) >= count:
                    break
                
                # Check if this track adds diversity
                artist_diverse = not track.artist or track.artist not in artists_seen
                genre_diverse = not track.genre or track.genre not in genres_seen
                
                if artist_diverse or genre_diverse:
                    selected_tracks.append(track)
                    if track.artist:
                        artists_seen.add(track.artist)
                    if track.genre:
                        genres_seen.add(track.genre)
            
            # If we don't have enough diverse tracks, fill with remaining
            if len(selected_tracks) < count:
                remaining_tracks = [t for t in filtered_tracks if t not in selected_tracks]
                selected_tracks.extend(remaining_tracks[:count - len(selected_tracks)])
        else:
            # Default to random selection
            import random
            selected_tracks = random.sample(filtered_tracks, min(count, len(filtered_tracks)))
        
        return selected_tracks
        
    except Exception as e:
        logger.error(f"Get track selection error: {e}")
        raise HTTPException(status_code=500, detail=f"Track selection failed: {str(e)}")


@router.post("/tracks/select", response_model=AudioResponse)
async def select_and_play_track(
    selection_request: dict,
    audio_mgr: AudioManager = Depends(get_audio_manager)
):
    """Select and play a track based on selection criteria"""
    try:
        # Extract selection parameters
        selection_type = selection_request.get("selection_type", "random")
        count = selection_request.get("count", 1)
        genre = selection_request.get("genre")
        artist = selection_request.get("artist")
        duration_min = selection_request.get("duration_min")
        duration_max = selection_request.get("duration_max")
        auto_play = selection_request.get("auto_play", True)
        
        # Get track selection
        tracks = await get_track_selection(
            selection_type=selection_type,
            count=count,
            genre=genre,
            artist=artist,
            duration_min=duration_min,
            duration_max=duration_max,
            audio_mgr=audio_mgr
        )
        
        if not tracks:
            return AudioResponse(
                success=False,
                message="No tracks match the selection criteria",
                error="no_matches"
            )
        
        # If auto_play is enabled and we have tracks, play the first one
        if auto_play and tracks:
            play_result = await audio_mgr.play_track(tracks[0].id)
            if play_result.success:
                return AudioResponse(
                    success=True,
                    message=f"Selected and playing: {tracks[0].title}",
                    data={
                        "selected_tracks": [
                            {
                                "id": track.id,
                                "title": track.title,
                                "artist": track.artist,
                                "album": track.album
                            } for track in tracks
                        ],
                        "playing_track": {
                            "id": tracks[0].id,
                            "title": tracks[0].title,
                            "artist": tracks[0].artist
                        }
                    }
                )
            else:
                return AudioResponse(
                    success=False,
                    message="Track selected but playback failed",
                    error="playback_failed",
                    data={
                        "selected_tracks": [
                            {
                                "id": track.id,
                                "title": track.title,
                                "artist": track.artist,
                                "album": track.album
                            } for track in tracks
                        ]
                    }
                )
        else:
            # Just return the selection without playing
            return AudioResponse(
                success=True,
                message=f"Selected {len(tracks)} tracks",
                data={
                    "selected_tracks": [
                        {
                            "id": track.id,
                            "title": track.title,
                            "artist": track.artist,
                            "album": track.album
                        } for track in tracks
                    ]
                }
            )
            
    except Exception as e:
        logger.error(f"Select and play track error: {e}")
        return AudioResponse(
            success=False,
            message="Track selection failed",
            error=str(e)
        )
