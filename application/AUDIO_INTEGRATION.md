# 🎵 Audio Integration for Stage LED Dashboard

This document describes the integration of audio functionality into the existing Stage LED Dashboard application.

## ✨ New Features Added

### 🎧 Audio System
- **Audio Dashboard** - Overview of audio system status and quick controls
- **Music Library** - Browse, search, and manage your music collection
- **Playlist Management** - Create and manage music playlists
- **Music Upload** - Drag-and-drop interface for adding new music files
- **Advanced Search** - Find music with filters and smart selection
- **Audio Settings** - Configure playback preferences and system settings

### 🔧 Technical Integration
- **Unified Navigation** - Audio features integrated into existing sidebar
- **Shared State Management** - Audio context integrated with existing app structure
- **Consistent UI/UX** - Audio components follow existing design patterns
- **API Integration** - Connects to Raspberry Pi audio server

## 🚀 Getting Started

### 1. Install Dependencies
```bash
cd application
chmod +x setup-audio.sh
./setup-audio.sh
```

### 2. Start the Application
```bash
npm start
```

### 3. Access Audio Features
Navigate to any of these routes:
- `/audio` - Audio Dashboard
- `/audio/library` - Music Library
- `/audio/playlists` - Playlist Management
- `/audio/upload` - Music Upload
- `/audio/search` - Advanced Search
- `/audio/settings` - Audio Settings

## 🏗️ Architecture

### Component Structure
```
src/components/Audio/
├── AudioContext.jsx          # State management for audio
├── AudioDashboard.jsx        # Main audio overview
├── AudioLibrary.jsx          # Music library browser
├── AudioPlaylists.jsx        # Playlist management
├── AudioUpload.jsx           # File upload interface
├── AudioSearch.jsx           # Search and selection
├── AudioSettings.jsx         # Configuration
├── PlaybackControls.jsx      # Audio playback controls
├── LibraryStats.jsx          # Library statistics
├── RecentTracks.jsx          # Recently played tracks
└── PopularTracks.jsx         # Popular tracks display
```

### State Management
- **AudioContext** - Central state management using React Context + useReducer
- **Audio API Service** - HTTP client for Raspberry Pi communication
- **Real-time Updates** - WebSocket integration for live status updates

### API Integration
- **Base URL**: `http://192.168.1.203:8000` (configurable via environment)
- **Endpoints**: Full REST API coverage for all audio operations
- **Error Handling**: Comprehensive error handling and user feedback

## 🎯 Key Features

### 🎵 Playback Control
- Play, pause, stop, next, previous
- Volume control and mute
- Progress tracking and seeking
- Crossfade between tracks

### 📚 Library Management
- Automatic metadata extraction
- Genre and format organization
- Play count tracking
- Last played timestamps

### 🔍 Smart Search
- Text search across titles, artists, albums
- Filter by genre, format, duration
- Quick action buttons for common selections
- Random track selection with filters

### 📤 File Management
- Drag-and-drop upload interface
- Multiple file format support (MP3, WAV, FLAC, AAC, OGG)
- File size validation (100MB limit)
- Progress tracking and error handling

### ⚙️ Configuration
- Audio preferences (auto-play, crossfade, volume)
- Interface settings (theme, language, notifications)
- Library management (refresh, statistics)
- System information and status

## 🔌 Integration Points

### Existing Application
- **Sidebar Navigation** - Audio routes added to existing navigation
- **Routing** - New audio routes integrated with React Router
- **Styling** - Consistent with existing design system
- **State Management** - Audio context wrapped around entire app

### Raspberry Pi Backend
- **FastAPI Server** - Audio endpoints for all operations
- **WebSocket Events** - Real-time status updates
- **File Storage** - Local music library management
- **Audio Playback** - pygame-based audio engine

## 🎨 UI/UX Features

### Responsive Design
- Mobile-friendly interface
- Adaptive layouts for different screen sizes
- Touch-friendly controls

### Visual Feedback
- Loading states and progress indicators
- Success/error notifications
- Hover effects and transitions
- Icon-based navigation

### Accessibility
- Keyboard navigation support
- Screen reader friendly
- High contrast mode support
- Focus management

## 🚦 Development Workflow

### Adding New Audio Features
1. Create component in `src/components/Audio/`
2. Add route to `App.js`
3. Update navigation in `Sidebar.jsx`
4. Add API methods to `audioApi.js`
5. Update state management in `AudioContext.jsx`

### Testing
- Test audio playback functionality
- Verify file upload and management
- Check search and filtering
- Validate playlist operations

## 🔧 Configuration

### Environment Variables
```bash
REACT_APP_API_URL=http://192.168.1.203:8000
REACT_APP_AUDIO_ENABLED=true
REACT_APP_AUDIO_UPLOAD_MAX_SIZE=104857600
```

### Audio Settings
- **Auto-play**: Automatically continue to next track
- **Crossfade**: Smooth transitions between tracks
- **Default Volume**: Initial volume level
- **Theme**: Light/dark/auto theme selection
- **Language**: Multi-language support

## 🐛 Troubleshooting

### Common Issues
1. **Audio not playing**: Check Raspberry Pi server status
2. **Upload failures**: Verify file size and format
3. **Search not working**: Check API connectivity
4. **Playlist issues**: Verify file permissions

### Debug Mode
Enable console logging for debugging:
```javascript
// In browser console
localStorage.setItem('audio_debug', 'true')
```

## 🔮 Future Enhancements

### Planned Features
- **Multi-room Audio** - Control audio in different areas
- **Voice Commands** - Speech recognition for controls
- **Mobile App** - Native mobile application
- **Cloud Sync** - Backup and sync across devices
- **AI Recommendations** - Smart music suggestions

### Performance Optimizations
- **Lazy Loading** - Load components on demand
- **Virtual Scrolling** - Handle large libraries efficiently
- **Caching** - Local storage for offline access
- **Compression** - Optimize audio file storage

## 📚 Additional Resources

- **API Documentation**: See Raspberry Pi backend docs
- **Component Library**: Reusable UI components
- **State Management**: Context API patterns
- **Testing**: Component testing examples

---

🎵 **Audio integration complete!** Your Stage LED Dashboard now includes a full-featured audio system alongside the existing LED control functionality.
