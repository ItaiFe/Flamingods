import React, { useState, useEffect } from 'react';
import { useAudio } from '../contexts/AudioContext';
import { Search, Music, Play, Shuffle, TrendingUp, Clock, Zap, Filter, X } from 'lucide-react';

const Search: React.FC = () => {
  const { state, searchTracks, getRandomTrack, getPopularTracks, getRecentTracks, getTracksByCategory, selectAndPlayTrack } = useAudio();
  
  const [searchQuery, setSearchQuery] = useState('');
  const [searchFilters, setSearchFilters] = useState({
    artist: '',
    album: '',
    genre: '',
    format: '',
    duration_min: '',
    duration_max: '',
    limit: 20
  });
  const [searchResults, setSearchResults] = useState(state.searchResults);
  const [isSearching, setIsSearching] = useState(false);

  useEffect(() => {
    setSearchResults(state.searchResults);
  }, [state.searchResults]);

  const handleSearch = async () => {
    if (!searchQuery.trim() && !Object.values(searchFilters).some(v => v)) return;
    
    setIsSearching(true);
    try {
      const filters = {
        query: searchQuery.trim() || undefined,
        ...searchFilters,
        duration_min: searchFilters.duration_min ? parseInt(searchFilters.duration_min) : undefined,
        duration_max: searchFilters.duration_max ? parseInt(searchFilters.duration_max) : undefined,
        limit: searchFilters.limit ? parseInt(searchFilters.limit) : undefined
      };
      
      await searchTracks(filters);
    } finally {
      setIsSearching(false);
    }
  };

  const handleQuickSearch = async (type: string, filters?: any) => {
    setIsSearching(true);
    try {
      let results: any[] = [];
      
      switch (type) {
        case 'random':
          const randomTrack = await getRandomTrack(filters);
          if (randomTrack) results = [randomTrack];
          break;
        case 'popular':
          results = await getPopularTracks(filters?.limit || 10);
          break;
        case 'recent':
          results = await getRecentTracks(filters?.limit || 10);
          break;
        case 'category':
          results = await getTracksByCategory(filters?.category || 'ambient', filters?.limit);
          break;
      }
      
      setSearchResults(results);
    } finally {
      setIsSearching(false);
    }
  };

  const handleSmartSelection = async (selectionType: string, count: number = 1) => {
    setIsSearching(true);
    try {
      await selectAndPlayTrack({
        selection_type: selectionType as any,
        count,
        auto_play: true
      });
    } finally {
      setIsSearching(false);
    }
  };

  const clearFilters = () => {
    setSearchQuery('');
    setSearchFilters({
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

  const formatDuration = (seconds: number): string => {
    if (!seconds || seconds < 0) return '0:00';
    
    const minutes = Math.floor(seconds / 60);
    const remainingSeconds = Math.floor(seconds % 60);
    return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
  };

  const handlePlayTrack = (trackId: string) => {
    // This would need to be implemented in the audio context
    console.log('Play track:', trackId);
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div>
        <h1 className="text-3xl font-bold text-white">Search & Selection</h1>
        <p className="text-gray-400 mt-2">
          Find and select music using advanced search and smart selection
        </p>
      </div>

      {/* Search Form */}
      <div className="card">
        <div className="space-y-4">
          {/* Main Search */}
          <div className="flex space-x-4">
            <div className="flex-1">
              <div className="relative">
                <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 h-4 w-4 text-gray-400" />
                <input
                  type="text"
                  placeholder="Search for tracks, artists, or albums..."
                  value={searchQuery}
                  onChange={(e) => setSearchQuery(e.target.value)}
                  className="input-field w-full pl-10"
                />
              </div>
            </div>
            <button
              onClick={handleSearch}
              disabled={isSearching}
              className="btn-primary px-6"
            >
              {isSearching ? 'Searching...' : 'Search'}
            </button>
          </div>

          {/* Advanced Filters */}
          <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
            <input
              type="text"
              placeholder="Artist"
              value={searchFilters.artist}
              onChange={(e) => setSearchFilters(prev => ({ ...prev, artist: e.target.value }))}
              className="input-field"
            />
            <input
              type="text"
              placeholder="Album"
              value={searchFilters.album}
              onChange={(e) => setSearchFilters(prev => ({ ...prev, album: e.target.value }))}
              className="input-field"
            />
            <input
              type="text"
              placeholder="Genre"
              value={searchFilters.genre}
              onChange={(e) => setSearchFilters(prev => ({ ...prev, genre: e.target.value }))}
              className="input-field"
            />
          </div>

          <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
            <select
              value={searchFilters.format}
              onChange={(e) => setSearchFilters(prev => ({ ...prev, format: e.target.value }))}
              className="input-field"
            >
              <option value="">All Formats</option>
              <option value="mp3">MP3</option>
              <option value="wav">WAV</option>
              <option value="flac">FLAC</option>
              <option value="ogg">OGG</option>
              <option value="m4a">M4A</option>
              <option value="aac">AAC</option>
            </select>
            <input
              type="number"
              placeholder="Min Duration (sec)"
              value={searchFilters.duration_min}
              onChange={(e) => setSearchFilters(prev => ({ ...prev, duration_min: e.target.value }))}
              className="input-field"
            />
            <input
              type="number"
              placeholder="Max Duration (sec)"
              value={searchFilters.duration_max}
              onChange={(e) => setSearchFilters(prev => ({ ...prev, duration_max: e.target.value }))}
              className="input-field"
            />
            <input
              type="number"
              placeholder="Limit"
              value={searchFilters.limit}
              onChange={(e) => setSearchFilters(prev => ({ ...prev, limit: e.target.value }))}
              className="input-field"
            />
          </div>

          {/* Filter Actions */}
          <div className="flex items-center space-x-4">
            <button
              onClick={clearFilters}
              className="btn-secondary flex items-center space-x-2"
            >
              <X className="h-4 w-4" />
              <span>Clear Filters</span>
            </button>
          </div>
        </div>
      </div>

      {/* Quick Actions */}
      <div className="card">
        <h3 className="text-lg font-medium text-white mb-4">Quick Actions</h3>
        <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
          <button
            onClick={() => handleQuickSearch('random')}
            className="btn-secondary flex flex-col items-center space-y-2 py-4"
          >
            <Shuffle className="h-6 w-6" />
            <span>Random Track</span>
          </button>
          
          <button
            onClick={() => handleQuickSearch('popular', { limit: 10 })}
            className="btn-secondary flex flex-col items-center space-y-2 py-4"
          >
            <TrendingUp className="h-6 w-6" />
            <span>Popular</span>
          </button>
          
          <button
            onClick={() => handleQuickSearch('recent', { limit: 10 })}
            className="btn-secondary flex flex-col items-center space-y-2 py-4"
          >
            <Clock className="h-6 w-6" />
            <span>Recent</span>
          </button>
          
          <button
            onClick={() => handleQuickSearch('category', { category: 'ambient', limit: 10 })}
            className="btn-secondary flex flex-col items-center space-y-2 py-4"
          >
            <Zap className="h-6 w-6" />
            <span>Ambient</span>
          </button>
        </div>
      </div>

      {/* Smart Selection */}
      <div className="card">
        <h3 className="text-lg font-medium text-white mb-4">Smart Selection</h3>
        <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
          <button
            onClick={() => handleSmartSelection('random', 5)}
            className="btn-primary flex items-center justify-center space-x-2 py-3"
          >
            <Shuffle className="h-4 w-4" />
            <span>Random Mix (5 tracks)</span>
          </button>
          
          <button
            onClick={() => handleSmartSelection('diverse', 10)}
            className="btn-primary flex items-center justify-center space-x-2 py-3"
          >
            <Zap className="h-4 w-4" />
            <span>Diverse Selection (10 tracks)</span>
          </button>
          
          <button
            onClick={() => handleSmartSelection('popular', 15)}
            className="btn-primary flex items-center justify-center space-x-2 py-3"
          >
            <TrendingUp className="h-4 w-4" />
            <span>Popular Mix (15 tracks)</span>
          </button>
        </div>
      </div>

      {/* Search Results */}
      {searchResults.length > 0 && (
        <div className="card">
          <div className="flex items-center justify-between mb-4">
            <h3 className="text-lg font-medium text-white">
              Search Results ({searchResults.length} tracks)
            </h3>
            <button
              onClick={() => setSearchResults([])}
              className="text-gray-400 hover:text-white"
            >
              <X className="h-5 w-5" />
            </button>
          </div>

          <div className="space-y-3">
            {searchResults.map((track) => (
              <div
                key={track.id}
                className="flex items-center space-x-3 p-3 bg-gray-700 rounded-lg hover:bg-gray-600 transition-colors"
              >
                {/* Track Icon */}
                <div className="flex-shrink-0">
                  <div className="w-10 h-10 bg-primary-600 rounded-lg flex items-center justify-center">
                    <Music className="h-5 w-5 text-white" />
                  </div>
                </div>

                {/* Track Info */}
                <div className="flex-1 min-w-0">
                  <div className="text-sm font-medium text-white truncate">
                    {track.title}
                  </div>
                  <div className="text-xs text-gray-400 truncate">
                    {track.artist || 'Unknown Artist'}
                  </div>
                  {track.album && (
                    <div className="text-xs text-gray-500 truncate">
                      {track.album}
                    </div>
                  )}
                </div>

                {/* Track Details */}
                <div className="flex-shrink-0 text-right">
                  <div className="text-xs text-gray-400">
                    {formatDuration(track.duration_seconds || 0)}
                  </div>
                  <div className="text-xs text-gray-500">
                    {track.format.toUpperCase()}
                  </div>
                </div>

                {/* Play Button */}
                <button
                  onClick={() => handlePlayTrack(track.id)}
                  className="flex-shrink-0 p-2 bg-primary-600 hover:bg-primary-700 rounded-lg transition-colors"
                  title="Play track"
                >
                  <Play className="h-4 w-4 text-white" />
                </button>
              </div>
            ))}
          </div>
        </div>
      )}

      {/* No Results */}
      {searchResults.length === 0 && searchQuery && !isSearching && (
        <div className="card text-center py-12 text-gray-400">
          <Search className="h-16 w-16 mx-auto mb-4 opacity-50" />
          <p className="text-lg">No tracks found</p>
          <p className="text-sm">Try adjusting your search terms or filters</p>
        </div>
      )}

      {/* Search Tips */}
      <div className="card bg-gray-800/50">
        <h3 className="text-lg font-medium text-white mb-3">Search Tips</h3>
        <ul className="space-y-2 text-sm text-gray-300">
          <li>• Use quotes for exact phrase matching</li>
          <li>• Combine multiple filters for precise results</li>
          <li>• Use duration filters to find tracks of specific lengths</li>
          <li>• Try smart selection for curated music experiences</li>
          <li>• Quick actions provide instant access to popular content</li>
        </ul>
      </div>
    </div>
  );
};

export default Search;
