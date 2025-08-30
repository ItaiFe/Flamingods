# Audio System for Midburn Art Installation

## Overview

The Raspberry Pi component now includes a comprehensive audio system that allows you to play music from a folder, manage playlists, and control audio playback via REST API. This system is designed to enhance the immersive art experience with background music, interactive audio, and synchronized audio-visual effects.

## Features

### 🎵 **Core Audio Capabilities**
- **Local Music Playback**: Play music from local folder (no internet required)
- **Multiple Format Support**: MP3, WAV, FLAC, OGG, M4A, AAC
- **Playlist Management**: Create, load, and manage playlists
- **Volume Control**: 0-100% volume with mute/unmute
- **Playback Control**: Play, pause, stop, next, previous

### 🎛️ **Advanced Features**
- **Auto-advance**: Automatically play next track in playlist
- **Shuffle Mode**: Random track selection
- **Repeat Mode**: Loop playlist or single track
- **Metadata Extraction**: Artist, album, duration, genre information
- **Real-time Status**: Live playback progress and status updates

### 🔌 **Integration Features**
- **REST API**: Full HTTP API for remote control
- **WebSocket Events**: Real-time audio status updates
- **Stage Synchronization**: Audio can trigger lighting effects
- **Device Integration**: Audio events can control Sonoff devices

## System Requirements

### **Hardware Requirements**
- **Raspberry Pi**: 3B+ or 4 (recommended)
- **Audio Output**: 3.5mm jack, HDMI audio, or USB audio
- **Storage**: Sufficient space for music files
- **Memory**: 2GB+ RAM recommended

### **Software Dependencies**
- **Python 3.8+**: Core runtime
- **pygame**: Audio playback engine
- **mutagen**: Audio metadata extraction
- **FastAPI**: REST API framework
- **WebSocket**: Real-time communication

## Installation

### **1. Install Dependencies**
```bash
cd raspberry
uv pip install -e .
```

### **2. Create Music Directory**
```bash
mkdir -p music/playlists
```

### **3. Add Music Files**
Place your audio files in the `music/` folder:
```
music/
├── ambient/
│   ├── track1.mp3
│   ├── track2.wav
│   └── track3.flac
├── electronic/
│   ├── beat1.mp3
│   └── beat2.ogg
└── playlists/
    ├── ambient.json
    └── electronic.json
```

## Configuration

### **Audio Settings**
The audio system is configured via environment variables or the `config.py` file:

```python
# Music folder settings
MUSIC_FOLDER=./music
PLAYLIST_FOLDER=./music/playlists

# Supported audio formats
SUPPORTED_FORMATS=mp3,wav,flac,ogg

# Audio device settings
AUDIO_DEVICE=  # Auto-detect if empty
SAMPLE_RATE=44100
CHANNELS=2

# Playback settings
DEFAULT_VOLUME=70
FADE_DURATION=1.0
CROSSFADE_DURATION=2.0

# Performance settings
BUFFER_SIZE=4096
MAX_QUEUE_SIZE=1000

# Metadata settings
SCAN_ON_STARTUP=true
AUTO_UPDATE_METADATA=true
EXTRACT_COVER_ART=false
```

## Usage

### **Starting the Server**
```bash
cd raspberry
uv run python main.py
```

The server will automatically:
1. Initialize the audio system
2. Scan the music folder for audio files
3. Load existing playlists
4. Start the REST API and WebSocket server

### **REST API Endpoints**

#### **Playback Control**
```bash
# Play audio
curl -X POST http://localhost:8000/audio/play

# Pause playback
curl -X POST http://localhost:8000/audio/pause

# Stop playback
curl -X POST http://localhost:8000/audio/stop

# Next track
curl -X POST http://localhost:8000/audio/next

# Previous track
curl -X POST http://localhost:8000/audio/previous
```

#### **Volume Control**
```bash
# Set volume to 80%
curl -X POST http://localhost:8000/audio/volume/80

# Toggle mute
curl -X POST http://localhost:8000/audio/mute
```

#### **Library Management**
```bash
# Get all tracks
curl http://localhost:8000/audio/tracks

# Get specific track
curl http://localhost:8000/audio/tracks/{track_id}

# Get all playlists
curl http://localhost:8000/audio/playlists

# Get specific playlist
curl http://localhost:8000/audio/playlists/{playlist_id}
```

#### **Advanced Control**
```bash
# Custom control command
curl -X POST http://localhost:8000/audio/control \
  -H "Content-Type: application/json" \
  -d '{
    "action": "play",
    "track_id": "track_123",
    "volume": 75,
    "shuffle": true
  }'

# Load and play playlist
curl -X POST http://localhost:8000/audio/playlists/{playlist_id}/play

# Scan music library
curl -X POST http://localhost:8000/audio/scan

# Get audio statistics
curl http://localhost:8000/audio/stats

# Health check
curl http://localhost:8000/audio/health
```

#### **File Upload Management**
```bash
# Upload single song
curl -X POST http://localhost:8000/audio/upload \
  -F "file=@song.mp3" \
  -F "category=electronic"

# Upload multiple songs
curl -X POST http://localhost:8000/audio/upload/batch \
  -F "files=@song1.mp3" \
  -F "files=@song2.wav" \
  -F "files=@song3.flac" \
  -F "category=ambient"

# Delete a song
curl -X DELETE http://localhost:8000/audio/tracks/{track_id}

# Scan for newly uploaded files
curl -X POST http://localhost:8000/audio/scan/uploaded
```

#### **Track Selection and Search**
```bash
# Search tracks by query
curl "http://localhost:8000/audio/tracks/search?query=ambient&limit=10"

# Search by specific criteria
curl "http://localhost:8000/audio/tracks/search?artist=artist_name&genre=electronic&duration_min=120"

# Get random track
curl "http://localhost:8000/audio/tracks/random?genre=ambient"

# Get popular tracks
curl "http://localhost:8000/audio/tracks/popular?limit=5"

# Get recently played tracks
curl "http://localhost:8000/audio/tracks/recent?limit=10"

# Get tracks by category
curl "http://localhost:8000/audio/tracks/by-category/electronic?limit=20"

# Get track selection
curl "http://localhost:8000/audio/tracks/selection/random?count=5&genre=ambient"

# Select and play track
curl -X POST http://localhost:8000/audio/tracks/select \
  -H "Content-Type: application/json" \
  -d '{
    "selection_type": "diverse",
    "count": 3,
    "genre": "electronic",
    "auto_play": true
  }'
```

### **WebSocket Events**
Connect to `ws://localhost:8000/ws` to receive real-time audio events:

```json
{
  "event_type": "audio_event",
  "device_id": "audio_system",
  "data": {
    "event_type": "playback_status_changed",
    "timestamp": "2024-08-30T17:30:00Z",
    "track_id": "track_123",
    "playlist_id": "ambient_playlist",
    "event_data": {
      "state": "playing",
      "position": 45.2,
      "volume": 75,
      "muted": false
    }
  }
}
```

## Testing

### **Test Audio System**
```bash
uv run python test_audio.py
```

This will test:
- Audio manager initialization
- Library scanning
- Playback control
- Volume and mute
- Audio statistics

### **Test API Endpoints**
```bash
uv run python test_server.py
```

### **Test Full Server**
```bash
uv run python main.py
```

Then test endpoints in another terminal:
```bash
curl http://localhost:8000/audio/health
curl http://localhost:8000/audio/status
```

## Playlist Management

### **Creating Playlists**
Playlists are stored as JSON files in the `music/playlists/` folder:

```json
{
  "id": "my_playlist",
  "name": "My Custom Playlist",
  "description": "A collection of my favorite tracks",
  "tracks": [
    {
      "id": "track_123",
      "title": "Track Title",
      "artist": "Artist Name",
      "album": "Album Name"
    }
  ],
  "shuffle": true,
  "repeat": true,
  "auto_advance": true
}
```

### **Auto-generated Playlists**
The system can automatically create playlists from folder structures:
- `music/ambient/` → Ambient playlist
- `music/electronic/` → Electronic playlist
- `music/classical/` → Classical playlist

## Integration Examples

### **Stage Lighting with Audio**
```python
# Audio event callback
async def on_audio_event(audio_event):
    if audio_event.event_type == "playback_status_changed":
        if audio_event.event_data["state"] == "playing":
            # Trigger stage lighting
            await stage_manager.set_lighting_plan("audio_active")
        elif audio_event.event_type == "track_finished":
            # Transition lighting
            await stage_manager.fade_to_plan("ambient")

# Register callback
audio_manager.add_event_callback(on_audio_event)
```

### **Device Control with Audio**
```python
# Volume-based device control
async def on_volume_change(volume):
    if volume > 80:
        # High volume - activate party mode
        await device_manager.control_device("party_lights", "on")
    elif volume < 30:
        # Low volume - ambient mode
        await device_manager.control_device("ambient_lights", "on")

# Register volume callback
audio_manager.add_status_callback(on_volume_change)
```

## Troubleshooting

### **Common Issues**

#### **No Audio Output**
1. Check audio device connection
2. Verify volume settings
3. Check system audio configuration
4. Test with `test_audio.py`

#### **Library Scan Fails**
1. Verify music folder exists
2. Check file permissions
3. Ensure supported audio formats
4. Check available disk space

#### **Playback Errors**
1. Verify audio file integrity
2. Check file format support
3. Ensure sufficient system resources
4. Review error logs

### **Debug Information**
```bash
# Check audio system health
curl http://localhost:8000/audio/health

# Get detailed status
curl http://localhost:8000/audio/status

# View audio statistics
curl http://localhost:8000/audio/stats

# Check server logs
tail -f logs/server.log
```

## Performance Optimization

### **For Large Music Libraries**
- Increase `BUFFER_SIZE` for better audio quality
- Set `SCAN_ON_STARTUP=false` for faster startup
- Use SSD storage for better file access
- Optimize audio file formats (MP3 for size, FLAC for quality)

### **For Real-time Performance**
- Reduce `CROSSFADE_DURATION` for faster transitions
- Set `MAX_QUEUE_SIZE` based on available memory
- Use lower sample rates for basic audio needs
- Enable audio compression for network streaming

## Security Considerations

### **Local Network Only**
- Audio system operates on local network
- No external audio streaming
- File access limited to music folder
- API endpoints accessible only locally

### **Access Control**
- Consider adding authentication for API endpoints
- Restrict file upload capabilities
- Monitor audio file sources
- Validate playlist file integrity

## Future Enhancements

### **Planned Features**
- **Beat Detection**: Audio analysis for visual effects
- **Audio Effects**: Equalizer, reverb, echo
- **Streaming Support**: Internet radio integration
- **Multi-room Audio**: Synchronized playback across devices
- **Voice Control**: Speech recognition for audio control

### **Integration Possibilities**
- **MIDI Support**: Musical instrument integration
- **Audio Visualization**: Real-time spectrum analysis
- **Smart Playlists**: AI-powered music selection
- **Event Scheduling**: Time-based playlist changes
- **Audio Recording**: Capture ambient sounds

## Support

### **Getting Help**
1. Check the troubleshooting section
2. Review server logs for error details
3. Test with `test_audio.py` script
4. Verify configuration settings
5. Check system audio setup

### **Reporting Issues**
Include the following information:
- Raspberry Pi model and OS version
- Audio system configuration
- Error messages and logs
- Steps to reproduce the issue
- Expected vs. actual behavior

---

**Note**: The audio system is designed for local network operation and does not require internet connectivity. This makes it perfect for art installations and events where reliable, offline audio playback is essential.
