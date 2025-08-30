# Flamingods Audio React App

A modern React web application for controlling the Flamingods audio system on your Raspberry Pi. This app provides an intuitive interface for music playback, library management, and audio control.

## Features

### 🎵 **Playback Control**
- Play, pause, stop, next, previous track controls
- Volume control with mute/unmute
- Real-time playback progress
- Current track information display

### 📚 **Library Management**
- Browse all tracks in your music library
- View track metadata (artist, album, duration, etc.)
- Delete tracks from the library
- Library statistics and overview

### 📋 **Playlist Management**
- View and manage playlists
- Load and play playlists
- Create custom playlists
- Playlist settings (shuffle, repeat, auto-advance)

### 📤 **File Upload**
- Upload individual audio files
- Batch upload multiple files
- Organize uploads by category
- Automatic library scanning after uploads

### 🔍 **Search and Selection**
- Search tracks by title, artist, album, or genre
- Filter by audio format, duration, and other criteria
- Get random tracks with filters
- Popular and recently played tracks
- Smart track selection algorithms

### 📊 **Dashboard and Statistics**
- Real-time audio system status
- Library statistics and health
- Recent activity overview
- Quick action buttons

## Screenshots

*Screenshots will be added after the app is fully implemented*

## Installation

### Prerequisites
- Node.js 16+ and npm/yarn
- The Flamingods audio system running on your Raspberry Pi
- Audio system accessible at `http://localhost:8000` (or configure custom URL)

### Setup

1. **Clone and install dependencies**
   ```bash
   cd raspberry/react-audio-app
   npm install
   # or
   yarn install
   ```

2. **Configure the API endpoint**
   Create a `.env` file in the root directory:
   ```env
   REACT_APP_API_URL=http://localhost:8000
   ```
   
   Or set the environment variable:
   ```bash
   export REACT_APP_API_URL=http://your-raspberry-pi-ip:8000
   ```

3. **Start the development server**
   ```bash
   npm start
   # or
   yarn start
   ```

4. **Build for production**
   ```bash
   npm run build
   # or
   yarn build
   ```

## Usage

### Starting the App
1. Ensure your Raspberry Pi audio system is running
2. Start the React development server: `npm start`
3. Open your browser to `http://localhost:3000`
4. The app will automatically connect to your audio system

### Navigation
- **Dashboard**: Main control center with playback controls and overview
- **Library**: Browse and manage your music collection
- **Playlists**: Manage and play playlists
- **Upload**: Add new music files to your library
- **Search**: Find specific tracks using various filters
- **Settings**: Configure app preferences and audio system settings

### Basic Controls
- **Play**: Start playback of current track or selected track
- **Pause**: Pause current playback
- **Stop**: Stop playback completely
- **Next/Previous**: Navigate between tracks in playlist
- **Volume**: Adjust volume using the slider
- **Mute**: Toggle mute on/off

### Uploading Music
1. Navigate to the Upload page
2. Drag and drop audio files or click to select
3. Choose a category (optional)
4. Click Upload to add files to your library
5. The library will automatically scan for new files

### Searching and Filtering
1. Go to the Search page
2. Enter search terms or use filters
3. Results update in real-time
4. Click on tracks to play them
5. Use advanced filters for precise results

## Configuration

### Environment Variables
- `REACT_APP_API_URL`: URL of your audio system API (default: `http://localhost:8000`)

### Audio System Requirements
- FastAPI server running with audio endpoints
- Audio files accessible in configured music folder
- Proper CORS configuration for web access
- Audio playback system (pygame) initialized

## Development

### Project Structure
```
src/
├── components/          # Reusable UI components
├── contexts/           # React contexts (AudioContext)
├── pages/              # Page components
├── services/           # API service layer
├── types/              # TypeScript type definitions
├── App.tsx            # Main app component
├── index.tsx          # App entry point
└── index.css          # Global styles with Tailwind
```

### Key Components
- **AudioContext**: Central state management for audio system
- **audioApi**: Service layer for API communication
- **PlaybackControls**: Main audio control interface
- **LibraryStats**: Display library information
- **TrackList**: Browse and manage tracks

### Adding New Features
1. Define types in `src/types/audio.ts`
2. Add API methods in `src/services/audioApi.ts`
3. Update context in `src/contexts/AudioContext.tsx`
4. Create UI components in `src/components/`
5. Add pages in `src/pages/`

## API Integration

The app communicates with your Raspberry Pi audio system through REST API endpoints:

- **Playback**: `/audio/play`, `/audio/pause`, `/audio/stop`, etc.
- **Library**: `/audio/tracks`, `/audio/playlists`, `/audio/stats`
- **Upload**: `/audio/upload`, `/audio/upload/batch`
- **Search**: `/audio/tracks/search`, `/audio/tracks/random`
- **Control**: `/audio/control`, `/audio/volume/{level}`

## Troubleshooting

### Common Issues

#### **App won't connect to audio system**
- Check if the audio system is running on your Raspberry Pi
- Verify the API URL in `.env` or environment variables
- Check CORS configuration on the server
- Ensure network connectivity between devices

#### **Audio controls not responding**
- Check browser console for error messages
- Verify audio system health endpoint
- Check if pygame audio system is initialized
- Review server logs for audio-related errors

#### **Uploads failing**
- Check file size limits
- Verify supported audio formats
- Check disk space on Raspberry Pi
- Review file permissions in music folder

#### **Search not working**
- Ensure library has been scanned
- Check search filter parameters
- Verify track metadata extraction
- Review search endpoint logs

### Debug Information
- Check browser developer console for API errors
- Review network tab for failed requests
- Check audio system health: `GET /audio/health`
- Review server logs on Raspberry Pi

## Performance Optimization

### For Large Libraries
- Implement virtual scrolling for track lists
- Add pagination for search results
- Cache frequently accessed data
- Optimize image loading and rendering

### For Mobile Devices
- Responsive design for all screen sizes
- Touch-friendly controls
- Optimized bundle size
- Progressive Web App features

## Security Considerations

- **Local Network Only**: App designed for local network use
- **No Authentication**: Basic access control (can be added)
- **File Upload Validation**: Server-side file validation
- **CORS Configuration**: Proper cross-origin settings

## Future Enhancements

### Planned Features
- **Real-time Updates**: WebSocket integration for live status
- **Audio Visualization**: Spectrum analyzer and waveform display
- **Advanced Playlists**: Smart playlists and auto-generation
- **Mobile App**: React Native version for mobile devices

### Possible Integrations
- **Voice Control**: Speech recognition for hands-free operation
- **Smart Home**: Integration with home automation systems
- **Social Features**: Share playlists and track recommendations
- **Cloud Sync**: Backup and sync across multiple devices

## Support

### Getting Help
1. Check the troubleshooting section above
2. Review browser console for error messages
3. Check audio system logs on Raspberry Pi
4. Verify network connectivity and configuration

### Contributing
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests if applicable
5. Submit a pull request

## License

This project is part of the Flamingods art installation and follows the same licensing terms.

---

**Note**: This React app is designed to work with the Flamingods audio system running on a Raspberry Pi. Make sure your audio system is properly configured and running before using the app.
