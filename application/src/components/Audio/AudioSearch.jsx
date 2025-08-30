import React, { useState } from 'react';
import { useAudio } from './AudioContext';
import { Search, Shuffle, TrendingUp, Clock, Zap, Music, Play, Filter } from 'lucide-react';

const AudioSearch = () => {
  const { state, searchTracks, getRandomTrack, getTracksByCategory, play } = useAudio();
  const { tracks } = state;
  
  const [searchQuery, setSearchQuery] = useState('');
  const [searchResults, setSearchResults] = useState([]);
  const [isSearching, setIsSearching] = useState(false);
  const [activeFilters, setActiveFilters] = useState({
    artist: '',
    album: '',
    genre: '',
    format: '',
    duration_min: '',
    duration_max: '',
    limit: 20
  });

  // Get unique values for filters
  const artists = [...new Set(tracks.map(track => track.artist).filter(Boolean))].sort();
  const albums = [...new Set(tracks.map(track => track.album).filter(Boolean))].sort();
  const genres = [...new Set(tracks.map(track => track.genre).filter(Boolean))].sort();
  const formats = [...new Set(tracks.map(track => track.format).filter(Boolean))].sort();

  const handleSearch = async () => {
    if (!searchQuery.trim()) return;
    
    setIsSearching(true);
    try {
      const filters = { ...activeFilters };
      if (searchQuery) filters.query = searchQuery;
      
      const result = await searchTracks(filters);
      setSearchResults(result.tracks || []);
    } catch (error) {
      console.error('Search failed:', error);
      setSearchResults([]);
    } finally {
      setIsSearching(false);
    }
  };

  const handleQuickAction = async (action, params = {}) => {
    try {
      let result;
      switch (action) {
        case 'random':
          result = await getRandomTrack(params);
          if (result.track) {
            await play(result.track.id);
          }
          break;
        case 'category':
          result = await getTracksByCategory(params.category, 1);
          if (result.tracks && result.tracks.length > 0) {
            await play(result.tracks[0].id);
          }
          break;
        default:
          break;
      }
    } catch (error) {
      console.error('Quick action failed:', error);
    }
  };

  const clearFilters = () => {
    setActiveFilters({
      artist: '',
      album: '',
      genre: '',
      format: '',
      duration_min: '',
      duration_max: '',
      limit: 20
    });
    setSearchResults([]);
  };

  const formatTime = (seconds) => {
    if (!seconds || seconds < 0) return '0:00';
    const minutes = Math.floor(seconds / 60);
    const remainingSeconds = Math.floor(seconds % 60);
    return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold text-white">Search & Select</h1>
          <p className="text-gray-400 mt-2">
            Find and select music with advanced search and smart selection
          </p>
        </div>
        <div className="flex items-center space-x-2 text-primary-400">
          <Search className="h-8 w-8" />
        </div>
      </div>

      {/* Quick Actions */}
      <div className="card">
        <h2 className="text-xl font-semibold text-white mb-4">Quick Actions</h2>
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-4 gap-4">
          <button
            onClick={() => handleQuickAction('random')}
            className="btn-secondary flex items-center space-x-3 p-4 hover:bg-primary-600 transition-colors"
          >
            <Shuffle className="h-6 w-6 text-primary-400" />
            <span>Random Track</span>
          </button>
          
          <button
            onClick={() => handleQuickAction('category', { category: 'ambient' })}
            className="btn-secondary flex items-center space-x-3 p-4 hover:bg-primary-600 transition-colors"
          >
            <Music className="h-6 w-6 text-primary-400" />
            <span>Ambient Mood</span>
          </button>
          
          <button
            onClick={() => handleQuickAction('category', { category: 'energetic' })}
            className="btn-secondary flex items-center space-x-3 p-4 hover:bg-primary-600 transition-colors"
          >
            <Zap className="h-6 w-6 text-primary-400" />
            <span>High Energy</span>
          </button>
          
          <button
            onClick={() => handleQuickAction('category', { category: 'chill' })}
            className="btn-secondary flex items-center space-x-3 p-4 hover:bg-primary-600 transition-colors"
          >
            <Clock className="h-6 w-6 text-primary-400" />
            <span>Chill Vibes</span>
          </button>
        </div>
      </div>

      {/* Search Interface */}
      <div className="card">
        <h2 className="text-xl font-semibold text-white mb-4">Advanced Search</h2>
        
        {/* Search Bar */}
        <div className="flex space-x-4 mb-6">
          <div className="flex-1 relative">
            <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 h-4 w-4 text-gray-400" />
            <input
              type="text"
              placeholder="Search tracks, artists, albums..."
              value={searchQuery}
              onChange={(e) => setSearchQuery(e.target.value)}
              onKeyPress={(e) => e.key === 'Enter' && handleSearch()}
              className="w-full pl-10 pr-4 py-2 bg-gray-700 border border-gray-600 rounded-lg text-white placeholder-gray-400 focus:outline-none focus:border-primary-500"
            />
          </div>
          
          <button
            onClick={handleSearch}
            disabled={isSearching || !searchQuery.trim()}
            className="btn-primary px-6 disabled:opacity-50 disabled:cursor-not-allowed"
          >
            {isSearching ? 'Searching...' : 'Search'}
          </button>
        </div>

        {/* Filters */}
        <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-4 mb-6">
          <div>
            <label className="block text-sm font-medium text-gray-400 mb-2">Artist</label>
            <select
              value={activeFilters.artist}
              onChange={(e) => setActiveFilters(prev => ({ ...prev, artist: e.target.value }))}
              className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-lg text-white focus:outline-none focus:border-primary-500"
            >
              <option value="">All Artists</option>
              {artists.map(artist => (
                <option key={artist} value={artist}>{artist}</option>
              ))}
            </select>
          </div>

          <div>
            <label className="block text-sm font-medium text-gray-400 mb-2">Album</label>
            <select
              value={activeFilters.album}
              onChange={(e) => setActiveFilters(prev => ({ ...prev, album: e.target.value }))}
              className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-lg text-white focus:outline-none focus:border-primary-500"
            >
              <option value="">All Albums</option>
              {albums.map(album => (
                <option key={album} value={album}>{album}</option>
              ))}
            </select>
          </div>

          <div>
            <label className="block text-sm font-medium text-gray-400 mb-2">Genre</label>
            <select
              value={activeFilters.genre}
              onChange={(e) => setActiveFilters(prev => ({ ...prev, genre: e.target.value }))}
              className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-lg text-white focus:outline-none focus:border-primary-500"
            >
              <option value="">All Genres</option>
              {genres.map(genre => (
                <option key={genre} value={genre}>{genre}</option>
              ))}
            </select>
          </div>

          <div>
            <label className="block text-sm font-medium text-gray-400 mb-2">Format</label>
            <select
              value={activeFilters.format}
              onChange={(e) => setActiveFilters(prev => ({ ...prev, format: e.target.value }))}
              className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-lg text-white focus:outline-none focus:border-primary-500"
            >
              <option value="">All Formats</option>
              {formats.map(format => (
                <option key={format} value={format}>{format.toUpperCase()}</option>
              ))}
            </select>
          </div>

          <div>
            <label className="block text-sm font-medium text-gray-400 mb-2">Min Duration (min)</label>
            <input
              type="number"
              placeholder="0"
              value={activeFilters.duration_min}
              onChange={(e) => setActiveFilters(prev => ({ ...prev, duration_min: e.target.value }))}
              className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-lg text-white placeholder-gray-400 focus:outline-none focus:border-primary-500"
            />
          </div>

          <div>
            <label className="block text-sm font-medium text-gray-400 mb-2">Max Duration (min)</label>
            <input
              type="number"
              placeholder="∞"
              value={activeFilters.duration_max}
              onChange={(e) => setActiveFilters(prev => ({ ...prev, duration_max: e.target.value }))}
              className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-lg text-white placeholder-gray-400 focus:outline-none focus:border-primary-500"
            />
          </div>
        </div>

        <div className="flex justify-between items-center">
          <div className="flex items-center space-x-4">
            <label className="flex items-center space-x-2">
              <span className="text-sm text-gray-400">Limit:</span>
              <select
                value={activeFilters.limit}
                onChange={(e) => setActiveFilters(prev => ({ ...prev, limit: parseInt(e.target.value) }))}
                className="px-2 py-1 bg-gray-700 border border-gray-600 rounded text-white text-sm focus:outline-none focus:border-primary-500"
              >
                <option value={10}>10</option>
                <option value={20}>20</option>
                <option value={50}>50</option>
                <option value={100}>100</option>
              </select>
            </label>
          </div>

          <button
            onClick={clearFilters}
            className="text-gray-400 hover:text-white text-sm"
          >
            Clear Filters
          </button>
        </div>
      </div>

      {/* Search Results */}
      {searchResults.length > 0 && (
        <div className="card">
          <h3 className="text-lg font-medium text-white mb-4">
            Search Results ({searchResults.length})
          </h3>
          
          <div className="space-y-2">
            {searchResults.map((track) => (
              <div 
                key={track.id} 
                className="flex items-center space-x-4 p-3 bg-gray-700 rounded-lg hover:bg-gray-600 transition-colors group"
              >
                <div className="w-12 h-12 bg-primary-600 rounded-lg flex items-center justify-center">
                  <Music className="h-6 w-6 text-white" />
                </div>
                
                <div className="flex-1 min-w-0">
                  <div className="text-white font-medium truncate">{track.title}</div>
                  <div className="text-gray-400 text-sm truncate">{track.artist}</div>
                  {track.album && (
                    <div className="text-gray-500 text-xs truncate">{track.album}</div>
                  )}
                </div>
                
                <div className="flex-shrink-0 text-right text-sm text-gray-400">
                  <div>{track.genre}</div>
                  <div>{track.format.toUpperCase()}</div>
                </div>
                
                <div className="flex-shrink-0 text-right text-sm text-gray-400">
                  <div>{formatTime(track.duration)}</div>
                  <div>{track.play_count || 0} plays</div>
                </div>
                
                <button
                  onClick={() => play(track.id)}
                  className="p-2 bg-primary-600 rounded-lg text-white hover:bg-primary-500 transition-colors opacity-0 group-hover:opacity-100"
                >
                  <Play className="h-4 w-4" />
                </button>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* Smart Selection */}
      <div className="card">
        <h2 className="text-xl font-semibold text-white mb-4">Smart Selection</h2>
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <button
            onClick={() => handleQuickAction('random', { genre: 'ambient', duration_min: 180 })}
            className="btn-secondary p-4 text-left"
          >
            <div className="flex items-center space-x-3 mb-2">
              <Clock className="h-6 w-6 text-primary-400" />
              <span className="font-medium">Long Ambient</span>
            </div>
            <p className="text-sm text-gray-400">3+ minute ambient tracks for relaxation</p>
          </button>
          
          <button
            onClick={() => handleQuickAction('random', { genre: 'energetic', duration_max: 180 })}
            className="btn-secondary p-4 text-left"
          >
            <div className="flex items-center space-x-3 mb-2">
              <Zap className="h-6 w-6 text-primary-400" />
              <span className="font-medium">Short Energy</span>
            </div>
            <p className="text-sm text-gray-400">Quick energetic tracks under 3 minutes</p>
          </button>
          
          <button
            onClick={() => handleQuickAction('random', { artist: 'popular' })}
            className="btn-secondary p-4 text-left"
          >
            <div className="flex items-center space-x-3 mb-2">
              <TrendingUp className="h-6 w-6 text-primary-400" />
              <span className="font-medium">Popular Artist</span>
            </div>
            <p className="text-sm text-gray-400">Tracks from frequently played artists</p>
          </button>
          
          <button
            onClick={() => handleQuickAction('random', { format: 'flac' })}
            className="btn-secondary p-4 text-left"
          >
            <div className="flex items-center space-x-3 mb-2">
              <Music className="h-6 w-6 text-primary-400" />
              <span className="font-medium">High Quality</span>
            </div>
            <p className="text-sm text-gray-400">Lossless FLAC format tracks</p>
          </button>
        </div>
      </div>
    </div>
  );
};

export default AudioSearch;
