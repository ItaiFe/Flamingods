"""
Audio Manager for music playback system

This module provides the core audio management functionality including:
- Music playback control
- Playlist management
- Audio file scanning and metadata extraction
- Volume control and audio effects
"""

import asyncio
import os
import time
import hashlib
import random
from datetime import datetime
from pathlib import Path
from typing import Dict, List, Optional, Tuple, Any
from concurrent.futures import ThreadPoolExecutor
import structlog

# Audio libraries
try:
    import pygame
    PYGAME_AVAILABLE = True
except ImportError:
    PYGAME_AVAILABLE = False

try:
    from mutagen import File as MutagenFile
    from mutagen.mp3 import MP3
    from mutagen.wave import WAVE
    from mutagen.flac import FLAC
    MUTAGEN_AVAILABLE = True
except ImportError:
    MUTAGEN_AVAILABLE = False

from audio_models import (
    TrackInfo, PlaylistInfo, PlaybackStatus, AudioControl, 
    AudioResponse, AudioEvent, AudioConfig, AudioStats,
    PlaybackState, AudioFormat
)

logger = structlog.get_logger()


class AudioManager:
    """Manages audio playback and playlist functionality"""
    
    def __init__(self, config: AudioConfig):
        self.config = config
        self.executor = ThreadPoolExecutor(max_workers=2)
        
        # Audio state
        self.current_track: Optional[TrackInfo] = None
        self.current_playlist: Optional[PlaylistInfo] = None
        self.playback_state = PlaybackState.STOPPED
        self.volume = config.default_volume
        self.muted = False
        self.position_seconds = 0.0
        self.duration_seconds = 0.0
        
        # Library management
        self.tracks: Dict[str, TrackInfo] = {}
        self.playlists: Dict[str, PlaylistInfo] = {}
        self.track_queue: List[TrackInfo] = []
        self.current_queue_index = 0
        
        # Playback settings
        self.shuffle = False
        self.repeat = False
        self.auto_advance = True
        
        # System state
        self.is_initialized = False
        self.startup_time = time.time()
        self.last_scan_time = None
        self.scan_duration = 0.0
        self.errors_count = 0
        
        # Event callbacks
        self.status_callbacks: List[callable] = []
        self.event_callbacks: List[callable] = []
        
        # Initialize pygame mixer if available
        if PYGAME_AVAILABLE:
            try:
                pygame.mixer.init(
                    frequency=self.config.sample_rate,
                    size=-16,
                    channels=self.config.channels,
                    buffer=self.config.buffer_size
                )
                self.is_initialized = True
                logger.info("Pygame mixer initialized successfully")
            except Exception as e:
                logger.error(f"Failed to initialize pygame mixer: {e}")
                self.errors_count += 1
        else:
            logger.warning("Pygame not available - audio playback disabled")
    
    async def start(self):
        """Start the audio manager"""
        logger.info("Starting Audio Manager")
        
        # Create music directories if they don't exist
        os.makedirs(self.config.music_folder, exist_ok=True)
        os.makedirs(self.config.playlist_folder, exist_ok=True)
        
        # Scan music library if enabled
        if self.config.scan_on_startup:
            await self.scan_music_library()
        
        # Set initial volume
        self._set_volume(self.volume)
        
        logger.info("Audio Manager started successfully")
    
    async def stop(self):
        """Stop the audio manager"""
        logger.info("Stopping Audio Manager")
        
        # Stop current playback
        await self.stop_playback()
        
        # Shutdown pygame mixer
        if PYGAME_AVAILABLE and self.is_initialized:
            pygame.mixer.quit()
        
        # Shutdown executor
        self.executor.shutdown(wait=True)
        
        logger.info("Audio Manager stopped")
    
    async def scan_music_library(self) -> AudioResponse:
        """Scan music folder for audio files"""
        logger.info(f"Scanning music library: {self.config.music_folder}")
        
        start_time = time.time()
        self.tracks.clear()
        
        try:
            # Scan music folder
            music_path = Path(self.config.music_folder)
            if not music_path.exists():
                return AudioResponse(
                    success=False,
                    message=f"Music folder does not exist: {self.config.music_folder}",
                    error="folder_not_found"
                )
            
            # Find audio files
            audio_files = []
            for format_ext in self.config.supported_formats:
                audio_files.extend(music_path.rglob(f"*.{format_ext.value}"))
                audio_files.extend(music_path.rglob(f"*.{format_ext.value.upper()}"))
            
            logger.info(f"Found {len(audio_files)} audio files")
            
            # Process each audio file
            for audio_file in audio_files:
                try:
                    track_info = await self._extract_track_info(audio_file)
                    if track_info:
                        self.tracks[track_info.id] = track_info
                except Exception as e:
                    logger.warning(f"Failed to process {audio_file}: {e}")
                    self.errors_count += 1
            
            # Update scan statistics
            self.last_scan_time = datetime.utcnow()
            self.scan_duration = time.time() - start_time
            
            # Load playlists
            await self._load_playlists()
            
            logger.info(f"Library scan completed: {len(self.tracks)} tracks, {len(self.playlists)} playlists")
            
            return AudioResponse(
                success=True,
                message=f"Library scan completed: {len(self.tracks)} tracks found",
                data={
                    "tracks_count": len(self.tracks),
                    "playlists_count": len(self.playlists),
                    "scan_duration": self.scan_duration
                }
            )
            
        except Exception as e:
            logger.error(f"Library scan failed: {e}")
            self.errors_count += 1
            return AudioResponse(
                success=False,
                message="Library scan failed",
                error=str(e),
                error_code="scan_error"
            )
    
    async def _extract_track_info(self, file_path: Path) -> Optional[TrackInfo]:
        """Extract track information from audio file"""
        try:
            # Generate unique ID
            file_hash = hashlib.md5(str(file_path).encode()).hexdigest()
            track_id = f"track_{file_hash[:8]}"
            
            # Basic file info
            filename = file_path.name
            filepath = str(file_path.absolute())
            size_bytes = file_path.stat().st_size
            
            # Determine format
            format_ext = file_path.suffix.lower().lstrip('.')
            try:
                audio_format = AudioFormat(format_ext)
            except ValueError:
                audio_format = AudioFormat.MP3  # Default fallback
            
            # Extract metadata if mutagen is available
            title = filename
            artist = None
            album = None
            duration_seconds = None
            bitrate = None
            sample_rate = None
            channels = 2
            genre = None
            year = None
            track_number = None
            
            if MUTAGEN_AVAILABLE:
                try:
                    audio_file = MutagenFile(filepath)
                    if audio_file:
                        # Extract basic metadata
                        if hasattr(audio_file, 'tags'):
                            tags = audio_file.tags
                            
                            # Title
                            if 'title' in tags:
                                title = str(tags['title'][0])
                            elif 'TIT2' in tags:
                                title = str(tags['TIT2'][0])
                            
                            # Artist
                            if 'artist' in tags:
                                artist = str(tags['artist'][0])
                            elif 'TPE1' in tags:
                                artist = str(tags['TPE1'][0])
                            
                            # Album
                            if 'album' in tags:
                                album = str(tags['album'][0])
                            elif 'TALB' in tags:
                                album = str(tags['TALB'][0])
                            
                            # Genre
                            if 'genre' in tags:
                                genre = str(tags['genre'][0])
                            elif 'TCON' in tags:
                                genre = str(tags['TCON'][0])
                            
                            # Year
                            if 'date' in tags:
                                year_str = str(tags['date'][0])
                                try:
                                    year = int(year_str[:4])
                                except ValueError:
                                    pass
                            elif 'TDRC' in tags:
                                year_str = str(tags['TDRC'][0])
                                try:
                                    year = int(year_str[:4])
                                except ValueError:
                                    pass
                            
                            # Track number
                            if 'tracknumber' in tags:
                                track_str = str(tags['tracknumber'][0])
                                try:
                                    track_number = int(track_str)
                                except ValueError:
                                    pass
                            elif 'TRCK' in tags:
                                track_str = str(tags['TRCK'][0])
                                try:
                                    track_number = int(track_str)
                                except ValueError:
                                    pass
                        
                        # Extract audio properties
                        if hasattr(audio_file, 'info'):
                            info = audio_file.info
                            if hasattr(info, 'length'):
                                duration_seconds = info.length
                            if hasattr(info, 'bitrate'):
                                bitrate = info.bitrate // 1000  # Convert to kbps
                            if hasattr(info, 'sample_rate'):
                                sample_rate = info.sample_rate
                            if hasattr(info, 'channels'):
                                channels = info.channels
                
                except Exception as e:
                    logger.debug(f"Failed to extract metadata from {file_path}: {e}")
            
            # Create track info
            track_info = TrackInfo(
                id=track_id,
                title=title,
                artist=artist,
                album=album,
                filename=filename,
                filepath=filepath,
                format=audio_format,
                size_bytes=size_bytes,
                duration_seconds=duration_seconds,
                bitrate=bitrate,
                sample_rate=sample_rate,
                channels=channels,
                genre=genre,
                year=year,
                track_number=track_number
            )
            
            return track_info
            
        except Exception as e:
            logger.error(f"Failed to extract track info from {file_path}: {e}")
            return None
    
    async def _load_playlists(self):
        """Load playlist files from playlist folder"""
        try:
            playlist_path = Path(self.config.playlist_folder)
            if not playlist_path.exists():
                return
            
            # Find playlist files (JSON format)
            playlist_files = list(playlist_path.glob("*.json"))
            
            for playlist_file in playlist_files:
                try:
                    import json
                    with open(playlist_file, 'r') as f:
                        playlist_data = json.load(f)
                    
                    # Create playlist info
                    playlist = PlaylistInfo(**playlist_data)
                    self.playlists[playlist.id] = playlist
                    
                except Exception as e:
                    logger.warning(f"Failed to load playlist {playlist_file}: {e}")
        
        except Exception as e:
            logger.error(f"Failed to load playlists: {e}")
    
    async def play_track(self, track_id: str) -> AudioResponse:
        """Play a specific track"""
        if not self.is_initialized:
            return AudioResponse(
                success=False,
                message="Audio system not initialized",
                error="not_initialized"
            )
        
        if track_id not in self.tracks:
            return AudioResponse(
                success=False,
                message=f"Track not found: {track_id}",
                error="track_not_found"
            )
        
        try:
            track = self.tracks[track_id]
            
            # Stop current playback
            await self.stop_playback()
            
            # Load and play new track
            await self._load_track(track)
            
            # Update state
            self.current_track = track
            self.playback_state = PlaybackState.PLAYING
            self.position_seconds = 0.0
            self.duration_seconds = track.duration_seconds or 0.0
            
            # Update track statistics
            track.play_count += 1
            track.last_played = datetime.utcnow()
            
            # Notify status change
            await self._notify_status_change()
            
            logger.info(f"Playing track: {track.title}")
            
            return AudioResponse(
                success=True,
                message=f"Playing track: {track.title}",
                data={"track_id": track_id, "title": track.title}
            )
            
        except Exception as e:
            logger.error(f"Failed to play track {track_id}: {e}")
            self.errors_count += 1
            return AudioResponse(
                success=False,
                message="Failed to play track",
                error=str(e),
                error_code="playback_error"
            )
    
    async def _load_track(self, track: TrackInfo):
        """Load a track for playback"""
        try:
            # Load audio file with pygame
            if PYGAME_AVAILABLE:
                # Stop any current playback
                pygame.mixer.music.stop()
                
                # Load the file
                pygame.mixer.music.load(track.filepath)
                
                # Set volume
                self._set_volume(self.volume)
                
                # Start playback
                pygame.mixer.music.play()
                
                # Start position monitoring
                asyncio.create_task(self._monitor_playback())
            
        except Exception as e:
            logger.error(f"Failed to load track {track.title}: {e}")
            raise
    
    async def _monitor_playback(self):
        """Monitor playback progress"""
        try:
            while self.playback_state == PlaybackState.PLAYING:
                if PYGAME_AVAILABLE and pygame.mixer.music.get_busy():
                    # Update position
                    if self.duration_seconds > 0:
                        self.position_seconds = pygame.mixer.music.get_pos() / 1000.0
                        self.progress_percent = (self.position_seconds / self.duration_seconds) * 100
                    
                    # Check if track finished
                    if self.position_seconds >= self.duration_seconds:
                        await self._on_track_finished()
                        break
                    
                    # Update status every second
                    await asyncio.sleep(1.0)
                    await self._notify_status_change()
                else:
                    # Playback stopped
                    break
                    
        except Exception as e:
            logger.error(f"Playback monitoring error: {e}")
            self.errors_count += 1
    
    async def _on_track_finished(self):
        """Handle track completion"""
        if self.auto_advance and self.current_playlist:
            # Auto-advance to next track
            await self.next_track()
        else:
            # Stop playback
            self.playback_state = PlaybackState.STOPPED
            self.current_track = None
            await self._notify_status_change()
    
    async def pause_playback(self) -> AudioResponse:
        """Pause current playback"""
        if self.playback_state != PlaybackState.PLAYING:
            return AudioResponse(
                success=False,
                message="No track currently playing",
                error="no_track_playing"
            )
        
        try:
            if PYGAME_AVAILABLE:
                pygame.mixer.music.pause()
            
            self.playback_state = PlaybackState.PAUSED
            await self._notify_status_change()
            
            return AudioResponse(
                success=True,
                message="Playback paused"
            )
            
        except Exception as e:
            logger.error(f"Failed to pause playback: {e}")
            return AudioResponse(
                success=False,
                message="Failed to pause playback",
                error=str(e)
            )
    
    async def resume_playback(self) -> AudioResponse:
        """Resume paused playback"""
        if self.playback_state != PlaybackState.PAUSED:
            return AudioResponse(
                success=False,
                message="Playback is not paused",
                error="not_paused"
            )
        
        try:
            if PYGAME_AVAILABLE:
                pygame.mixer.music.unpause()
            
            self.playback_state = PlaybackState.PLAYING
            await self._notify_status_change()
            
            return AudioResponse(
                success=True,
                message="Playback resumed"
            )
            
        except Exception as e:
            logger.error(f"Failed to resume playback: {e}")
            return AudioResponse(
                success=False,
                message="Failed to resume playback",
                error=str(e)
            )
    
    async def stop_playback(self) -> AudioResponse:
        """Stop current playback"""
        try:
            if PYGAME_AVAILABLE:
                pygame.mixer.music.stop()
            
            self.playback_state = PlaybackState.STOPPED
            self.current_track = None
            self.position_seconds = 0.0
            self.progress_percent = 0.0
            
            await self._notify_status_change()
            
            return AudioResponse(
                success=True,
                message="Playback stopped"
            )
            
        except Exception as e:
            logger.error(f"Failed to stop playback: {e}")
            return AudioResponse(
                success=False,
                message="Failed to stop playback",
                error=str(e)
            )
    
    async def next_track(self) -> AudioResponse:
        """Play next track in playlist"""
        if not self.current_playlist or not self.track_queue:
            return AudioResponse(
                success=False,
                message="No playlist loaded",
                error="no_playlist"
            )
        
        try:
            # Move to next track
            self.current_queue_index += 1
            
            # Handle repeat and shuffle
            if self.current_queue_index >= len(self.track_queue):
                if self.repeat:
                    self.current_queue_index = 0
                else:
                    return AudioResponse(
                        success=False,
                        message="End of playlist",
                        error="end_of_playlist"
                    )
            
            # Play next track
            next_track = self.track_queue[self.current_queue_index]
            return await self.play_track(next_track.id)
            
        except Exception as e:
            logger.error(f"Failed to play next track: {e}")
            return AudioResponse(
                success=False,
                message="Failed to play next track",
                error=str(e)
            )
    
    async def previous_track(self) -> AudioResponse:
        """Play previous track in playlist"""
        if not self.current_playlist or not self.track_queue:
            return AudioResponse(
                success=False,
                message="No playlist loaded",
                error="no_playlist"
            )
        
        try:
            # Move to previous track
            self.current_queue_index -= 1
            
            # Handle repeat
            if self.current_queue_index < 0:
                if self.repeat:
                    self.current_queue_index = len(self.track_queue) - 1
                else:
                    return AudioResponse(
                        success=False,
                        message="Beginning of playlist",
                        error="beginning_of_playlist"
                    )
            
            # Play previous track
            prev_track = self.track_queue[self.current_queue_index]
            return await self.play_track(prev_track.id)
            
        except Exception as e:
            logger.error(f"Failed to play previous track: {e}")
            return AudioResponse(
                success=False,
                message="Failed to play previous track",
                error=str(e)
            )
    
    def set_volume(self, volume: int) -> AudioResponse:
        """Set audio volume"""
        try:
            if not 0 <= volume <= 100:
                return AudioResponse(
                    success=False,
                    message="Volume must be between 0 and 100",
                    error="invalid_volume"
                )
            
            self.volume = volume
            self._set_volume(volume)
            
            return AudioResponse(
                success=True,
                message=f"Volume set to {volume}%",
                data={"volume": volume}
            )
            
        except Exception as e:
            logger.error(f"Failed to set volume: {e}")
            return AudioResponse(
                success=False,
                message="Failed to set volume",
                error=str(e)
            )
    
    def _set_volume(self, volume: int):
        """Set volume in pygame mixer"""
        if PYGAME_AVAILABLE and self.is_initialized:
            # Convert percentage to pygame volume (0.0 to 1.0)
            pygame_volume = volume / 100.0
            pygame.mixer.music.set_volume(pygame_volume)
    
    def toggle_mute(self) -> AudioResponse:
        """Toggle mute state"""
        try:
            self.muted = not self.muted
            
            if self.muted:
                # Store current volume and set to 0
                self._set_volume(0)
                message = "Audio muted"
            else:
                # Restore volume
                self._set_volume(self.volume)
                message = "Audio unmuted"
            
            return AudioResponse(
                success=True,
                message=message,
                data={"muted": self.muted}
            )
            
        except Exception as e:
            logger.error(f"Failed to toggle mute: {e}")
            return AudioResponse(
                success=False,
                message="Failed to toggle mute",
                error=str(e)
            )
    
    async def load_playlist(self, playlist_id: str) -> AudioResponse:
        """Load a playlist for playback"""
        if playlist_id not in self.playlists:
            return AudioResponse(
                success=False,
                message=f"Playlist not found: {playlist_id}",
                error="playlist_not_found"
            )
        
        try:
            playlist = self.playlists[playlist_id]
            
            # Set current playlist
            self.current_playlist = playlist
            self.track_queue = playlist.tracks.copy()
            self.current_queue_index = 0
            
            # Apply playlist settings
            self.shuffle = playlist.shuffle
            self.repeat = playlist.repeat
            self.auto_advance = playlist.auto_advance
            
            # Shuffle if enabled
            if self.shuffle:
                random.shuffle(self.track_queue)
            
            # Update playlist statistics
            playlist.last_played = datetime.utcnow()
            
            await self._notify_status_change()
            
            return AudioResponse(
                success=True,
                message=f"Playlist loaded: {playlist.name}",
                data={
                    "playlist_id": playlist_id,
                    "name": playlist.name,
                    "tracks_count": len(playlist.tracks)
                }
            )
            
        except Exception as e:
            logger.error(f"Failed to load playlist {playlist_id}: {e}")
            return AudioResponse(
                success=False,
                message="Failed to load playlist",
                error=str(e)
            )
    
    def get_playback_status(self) -> PlaybackStatus:
        """Get current playback status"""
        return PlaybackStatus(
            state=self.playback_state,
            current_track=self.current_track,
            current_playlist=self.current_playlist,
            position_seconds=self.position_seconds,
            duration_seconds=self.duration_seconds,
            progress_percent=self.progress_percent,
            volume=self.volume,
            muted=self.muted,
            queue_position=self.current_queue_index,
            queue_length=len(self.track_queue),
            last_updated=datetime.utcnow(),
            error_message=None
        )
    
    def get_audio_stats(self) -> AudioStats:
        """Get audio system statistics"""
        total_duration = sum(
            track.duration_seconds or 0 
            for track in self.tracks.values()
        ) / 3600  # Convert to hours
        
        total_play_time = sum(
            track.play_count * (track.duration_seconds or 0)
            for track in self.tracks.values()
        ) / 3600  # Convert to hours
        
        library_size_bytes = sum(
            track.size_bytes for track in self.tracks.values()
        )
        
        uptime = time.time() - self.startup_time
        
        return AudioStats(
            total_tracks=len(self.tracks),
            total_playlists=len(self.playlists),
            total_duration=total_duration,
            library_size_bytes=library_size_bytes,
            tracks_played=sum(track.play_count for track in self.tracks.values()),
            total_play_time=total_play_time,
            average_session_length=total_play_time / max(len(self.tracks), 1) * 60,  # Convert to minutes
            scan_time_seconds=self.scan_duration,
            last_scan=self.last_scan_time,
            errors_count=self.errors_count,
            uptime_seconds=uptime,
            memory_usage_mb=0.0,  # Could be enhanced with psutil
            cpu_usage_percent=0.0  # Could be enhanced with psutil
        )
    
    async def _notify_status_change(self):
        """Notify status change to callbacks"""
        status = self.get_playback_status()
        
        # Call status callbacks
        for callback in self.status_callbacks:
            try:
                await callback(status)
            except Exception as e:
                logger.error(f"Status callback error: {e}")
        
        # Create and send audio event
        event = AudioEvent(
            event_type="playback_status_changed",
            data={
                "state": status.state.value,
                "track_id": status.current_track.id if status.current_track else None,
                "position": status.position_seconds,
                "volume": status.volume,
                "muted": status.muted
            },
            track_id=status.current_track.id if status.current_track else None,
            playlist_id=status.current_playlist.id if status.current_playlist else None
        )
        
        # Call event callbacks
        for callback in self.event_callbacks:
            try:
                await callback(event)
            except Exception as e:
                logger.error(f"Event callback error: {e}")
    
    def add_status_callback(self, callback: callable):
        """Add a callback for status changes"""
        self.status_callbacks.append(callback)
    
    def add_event_callback(self, callback: callable):
        """Add a callback for audio events"""
        self.event_callbacks.append(callback)
    
    def get_tracks(self) -> List[TrackInfo]:
        """Get all tracks in library"""
        return list(self.tracks.values())
    
    def get_playlists(self) -> List[PlaylistInfo]:
        """Get all playlists"""
        return list(self.playlists.values())
    
    def get_track(self, track_id: str) -> Optional[TrackInfo]:
        """Get a specific track"""
        return self.tracks.get(track_id)
    
    def get_playlist(self, playlist_id: str) -> Optional[PlaylistInfo]:
        """Get a specific playlist"""
        return self.playlists.get(playlist_id)
