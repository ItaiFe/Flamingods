"""
Audio models for music playback system

This module defines the data models used by the audio system including:
- Track information
- Playlist management
- Playback status
- Audio control commands
"""

from datetime import datetime, timezone
from enum import Enum
from typing import List, Optional, Dict, Any
from pydantic import BaseModel, Field


class PlaybackState(str, Enum):
    """Audio playback states"""
    STOPPED = "stopped"
    PLAYING = "playing"
    PAUSED = "paused"
    LOADING = "loading"
    ERROR = "error"


class AudioFormat(str, Enum):
    """Supported audio formats"""
    MP3 = "mp3"
    WAV = "wav"
    FLAC = "flac"
    OGG = "ogg"
    M4A = "m4a"
    AAC = "aac"


class TrackInfo(BaseModel):
    """Information about an audio track"""
    
    # Basic track information
    id: str = Field(description="Unique track identifier")
    title: str = Field(description="Track title")
    artist: Optional[str] = Field(default=None, description="Track artist")
    album: Optional[str] = Field(default=None, description="Album name")
    
    # File information
    filename: str = Field(description="Audio file name")
    filepath: str = Field(description="Full path to audio file")
    format: AudioFormat = Field(description="Audio file format")
    size_bytes: int = Field(description="File size in bytes")
    
    # Audio properties
    duration_seconds: Optional[float] = Field(default=None, description="Track duration in seconds")
    bitrate: Optional[int] = Field(default=None, description="Audio bitrate in kbps")
    sample_rate: Optional[int] = Field(default=None, description="Sample rate in Hz")
    channels: Optional[int] = Field(default=2, description="Number of audio channels")
    
    # Metadata
    genre: Optional[str] = Field(default=None, description="Music genre")
    year: Optional[int] = Field(default=None, description="Release year")
    track_number: Optional[int] = Field(default=None, description="Track number on album")
    
    # System information
    created_at: datetime = Field(default_factory=lambda: datetime.now(timezone.utc), description="When track was added")
    last_played: Optional[datetime] = Field(default=None, description="Last time track was played")
    play_count: int = Field(default=0, description="Number of times track was played")


class PlaylistInfo(BaseModel):
    """Information about a playlist"""
    
    # Basic playlist information
    id: str = Field(description="Unique playlist identifier")
    name: str = Field(description="Playlist name")
    description: Optional[str] = Field(default=None, description="Playlist description")
    
    # Playlist content
    tracks: List[TrackInfo] = Field(default_factory=list, description="Tracks in playlist")
    total_duration: Optional[float] = Field(default=None, description="Total duration in seconds")
    
    # Playlist settings
    shuffle: bool = Field(default=False, description="Whether playlist shuffles tracks")
    repeat: bool = Field(default=False, description="Whether playlist repeats")
    auto_advance: bool = Field(default=True, description="Whether to auto-advance to next track")
    
    # System information
    created_at: datetime = Field(default_factory=lambda: datetime.now(timezone.utc), description="When playlist was created")
    last_modified: datetime = Field(default_factory=lambda: datetime.now(timezone.utc), description="Last modification time")
    last_played: Optional[datetime] = Field(default=None, description="Last time playlist was played")


class PlaybackStatus(BaseModel):
    """Current playback status"""
    
    # Playback state
    state: PlaybackState = Field(description="Current playback state")
    current_track: Optional[TrackInfo] = Field(default=None, description="Currently playing track")
    current_playlist: Optional[PlaylistInfo] = Field(default=None, description="Current playlist")
    
    # Playback progress
    position_seconds: float = Field(default=0.0, description="Current position in track (seconds)")
    duration_seconds: float = Field(default=0.0, description="Total track duration (seconds)")
    progress_percent: float = Field(default=0.0, description="Playback progress as percentage")
    
    # Audio settings
    volume: int = Field(default=70, ge=0, le=100, description="Current volume level (0-100)")
    muted: bool = Field(default=False, description="Whether audio is muted")
    
    # Queue information
    queue_position: int = Field(default=0, description="Position in current playlist")
    queue_length: int = Field(default=0, description="Total tracks in current playlist")
    
    # System information
    last_updated: datetime = Field(default_factory=lambda: datetime.now(timezone.utc), description="Last status update")
    error_message: Optional[str] = Field(default=None, description="Error message if any")


class AudioControl(BaseModel):
    """Audio control commands"""
    
    # Playback control
    action: str = Field(description="Control action to perform")
    
    # Optional parameters
    track_id: Optional[str] = Field(default=None, description="Specific track to play")
    playlist_id: Optional[str] = Field(default=None, description="Playlist to load")
    volume: Optional[int] = Field(default=None, ge=0, le=100, description="Volume level to set")
    position: Optional[float] = Field(default=None, ge=0, description="Position in track (seconds)")
    
    # Additional options
    shuffle: Optional[bool] = Field(default=None, description="Set shuffle mode")
    repeat: Optional[bool] = Field(default=None, description="Set repeat mode")
    fade_in: Optional[float] = Field(default=None, description="Fade in duration (seconds)")
    fade_out: Optional[float] = Field(default=None, description="Fade out duration (seconds)")


class AudioResponse(BaseModel):
    """Response from audio control operations"""
    
    # Operation result
    success: bool = Field(description="Whether operation was successful")
    message: str = Field(description="Response message")
    
    # Response data
    data: Optional[Dict[str, Any]] = Field(default=None, description="Additional response data")
    
    # Error information
    error: Optional[str] = Field(default=None, description="Error details if operation failed")
    error_code: Optional[str] = Field(default=None, description="Error code if applicable")


class AudioEvent(BaseModel):
    """WebSocket audio events"""
    
    # Event information
    event_type: str = Field(description="Type of audio event")
    timestamp: datetime = Field(default_factory=lambda: datetime.now(timezone.utc), description="Event timestamp")
    
    # Event data
    data: Dict[str, Any] = Field(default_factory=dict, description="Event data")
    
    # Related information
    track_id: Optional[str] = Field(default=None, description="Related track ID")
    playlist_id: Optional[str] = Field(default=None, description="Related playlist ID")


class AudioConfig(BaseModel):
    """Audio system configuration"""
    
    # Music folder settings
    music_folder: str = Field(default="./music", description="Path to music folder")
    playlist_folder: str = Field(default="./music/playlists", description="Path to playlist folder")
    
    # Supported audio formats
    supported_formats: List[AudioFormat] = Field(
        default=[AudioFormat.MP3, AudioFormat.WAV, AudioFormat.FLAC, AudioFormat.OGG],
        description="Supported audio file formats"
    )
    
    # Audio device settings
    audio_device: Optional[str] = Field(default=None, description="Audio output device (auto-detect if None)")
    sample_rate: int = Field(default=44100, description="Audio sample rate in Hz")
    channels: int = Field(default=2, description="Number of audio channels")
    
    # Playback settings
    default_volume: int = Field(default=70, ge=0, le=100, description="Default volume level (0-100)")
    fade_duration: float = Field(default=1.0, description="Default fade duration in seconds")
    crossfade_duration: float = Field(default=2.0, description="Crossfade duration between tracks")
    
    # Performance settings
    buffer_size: int = Field(default=4096, description="Audio buffer size")
    max_queue_size: int = Field(default=1000, description="Maximum tracks in queue")
    
    # Metadata settings
    scan_on_startup: bool = Field(default=True, description="Scan music folder on startup")
    auto_update_metadata: bool = Field(default=True, description="Auto-update track metadata")
    extract_cover_art: bool = Field(default=False, description="Extract cover art from audio files")


class AudioStats(BaseModel):
    """Audio system statistics"""
    
    # Library information
    total_tracks: int = Field(default=0, description="Total tracks in library")
    total_playlists: int = Field(default=0, description="Total playlists")
    total_duration: float = Field(default=0.0, description="Total music duration in hours")
    library_size_bytes: int = Field(default=0, description="Total library size in bytes")
    
    # Playback statistics
    tracks_played: int = Field(default=0, description="Total tracks played")
    total_play_time: float = Field(default=0.0, description="Total play time in hours")
    average_session_length: float = Field(default=0.0, description="Average session length in minutes")
    
    # Performance statistics
    scan_time_seconds: float = Field(default=0.0, description="Last library scan time")
    last_scan: Optional[datetime] = Field(default=None, description="Last library scan time")
    errors_count: int = Field(default=0, description="Total errors encountered")
    
    # System information
    uptime_seconds: float = Field(default=0.0, description="Audio system uptime")
    memory_usage_mb: float = Field(default=0.0, description="Memory usage in MB")
    cpu_usage_percent: float = Field(default=0.0, description="CPU usage percentage")
