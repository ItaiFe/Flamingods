import React from 'react';
import { BrowserRouter as Router, Routes, Route } from 'react-router-dom';
import { AudioProvider } from './contexts/AudioContext';
import Navbar from './components/Navbar';
import Dashboard from './pages/Dashboard';
import Library from './pages/Library';
import Playlists from './pages/Playlists';
import Upload from './pages/Upload';
import Search from './pages/Search';
import Settings from './pages/Settings';

function App() {
  return (
    <AudioProvider>
      <Router>
        <div className="min-h-screen bg-gray-900">
          <Navbar />
          <main className="container mx-auto px-4 py-8">
            <Routes>
              <Route path="/" element={<Dashboard />} />
              <Route path="/library" element={<Library />} />
              <Route path="/playlists" element={<Playlists />} />
              <Route path="/upload" element={<Upload />} />
              <Route path="/search" element={<Search />} />
              <Route path="/settings" element={<Settings />} />
            </Routes>
          </main>
        </div>
      </Router>
    </AudioProvider>
  );
}

export default App;
