import React from 'react';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import { AudioProvider } from './components/Audio/AudioContext';
import Dashboard from './components/Layout/Dashboard';
import AudioDashboard from './components/Audio/AudioDashboard';
import AudioLibrary from './components/Audio/AudioLibrary';
import AudioPlaylists from './components/Audio/AudioPlaylists';
import AudioUpload from './components/Audio/AudioUpload';
import AudioSearch from './components/Audio/AudioSearch';
import AudioSettings from './components/Audio/AudioSettings';
import './styles/App.css';

function App() {
  return (
    <AudioProvider>
      <Router>
        <div className="App">
          <Routes>
            <Route path="/" element={<Dashboard />} />
            <Route path="/audio" element={<AudioDashboard />} />
            <Route path="/audio/library" element={<AudioLibrary />} />
            <Route path="/audio/playlists" element={<AudioPlaylists />} />
            <Route path="/audio/upload" element={<AudioUpload />} />
            <Route path="/audio/search" element={<AudioSearch />} />
            <Route path="/audio/settings" element={<AudioSettings />} />
          </Routes>
        </div>
      </Router>
    </AudioProvider>
  );
}

export default App;
