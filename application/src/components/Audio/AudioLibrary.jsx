import React, { useState, useMemo } from 'react';
import { useAudio } from './AudioContext';
import { Library, Search, Filter, Play, Trash2, Music } from 'lucide-react';

const AudioLibrary = () => {
  const { state, play, deleteTrack } = useAudio();
  const { tracks } = state;
  
  const [searchQuery, setSearchQuery] = useState('');
  const [selectedGenre, setSelectedGenre] = useState('');
  const [selectedFormat, setSelectedFormat] = useState('');
  const [sortBy, setSortBy] = useState('title');
  const [sortOrder, setSortOrder] = useState('asc');

  // Get unique genres and formats for filters
  const genres = useMemo(() => 
    [...new Set(tracks.map(track => track.genre).filter(Boolean))].sort(), 
    [tracks]
  );
  
  const formats = useMemo(() => 
    [...new Set(tracks.map(track => track.format).filter(Boolean))].sort(), 
    [tracks]
  );

  // Filter and sort tracks
  const filteredTracks = useMemo(() => {
    let filtered = tracks.filter(track => {
      const matchesSearch = !searchQuery || 
        track.title.toLowerCase().includes(searchQuery.toLowerCase()) ||
        track.artist.toLowerCase().includes(searchQuery.toLowerCase()) ||
        (track.album && track.album.toLowerCase().includes(searchQuery.toLowerCase()));
      
      const matchesGenre = !selectedGenre || track.genre === selectedGenre;
      const matchesFormat = !selectedFormat || track.format === selectedFormat;
      
      return matchesSearch && matchesGenre && matchesFormat;
    });

    // Sort tracks
    filtered.sort((a, b) => {
      let aVal = a[sortBy];
      let bVal = b[sortBy];
      
      if (sortBy === 'duration') {
        aVal = aVal || 0;
        bVal = bVal || 0;
      } else if (sortBy === 'play_count') {
        aVal = aVal || 0;
        bVal = bVal || 0;
      } else {
        aVal = (aVal || '').toString().toLowerCase();
        bVal = (bVal || '').toString().toLowerCase();
      }
      
      if (sortOrder === 'asc') {
        return aVal > bVal ? 1 : -1;
      } else {
        return aVal < bVal ? 1 : -1;
      }
    });

    return filtered;
  }, [tracks, searchQuery, selectedGenre, selectedFormat, sortBy, sortOrder]);

  const formatTime = (seconds) => {
    if (!seconds || seconds < 0) return '0:00';
    const minutes = Math.floor(seconds / 60);
    const remainingSeconds = Math.floor(seconds % 60);
    return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
  };

  const formatFileSize = (bytes) => {
    if (!bytes || bytes === 0) return '0 B';
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
  };

  const handleDeleteTrack = async (trackId) => {
    if (window.confirm('Are you sure you want to delete this track?')) {
      try {
        await deleteTrack(trackId);
      } catch (error) {
        console.error('Failed to delete track:', error);
      }
    }
  };

  const clearFilters = () => {
    setSearchQuery('');
    setSelectedGenre('');
    setSelectedFormat('');
    setSortBy('title');
    setSortOrder('asc');
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold text-white">Music Library</h1>
          <p className="text-gray-400 mt-2">
            Browse and manage your music collection
          </p>
        </div>
        <div className="flex items-center space-x-2 text-primary-400">
          <Library className="h-8 w-8" />
        </div>
      </div>

      {/* Summary Stats */}
      <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
        <div className="bg-gray-700 rounded-lg p-4 text-center">
          <div className="text-2xl font-bold text-white">{tracks.length}</div>
          <div className="text-sm text-gray-400">Total Tracks</div>
        </div>
        
        <div className="bg-gray-700 rounded-lg p-4 text-center">
          <div className="text-lg font-bold text-white">
            {filteredTracks.length}
          </div>
          <div className="text-sm text-gray-400">Filtered</div>
        </div>
        
        <div className="bg-gray-700 rounded-lg p-4 text-center">
          <div className="text-lg font-bold text-white">
            {genres.length}
          </div>
          <div className="text-sm text-gray-400">Genres</div>
        </div>
        
        <div className="bg-gray-700 rounded-lg p-4 text-center">
          <div className="text-lg font-bold text-white">
            {formats.length}
          </div>
          <div className="text-sm text-gray-400">Formats</div>
        </div>
      </div>

      {/* Search and Filters */}
      <div className="card">
        <div className="flex flex-col md:flex-row gap-4">
          {/* Search */}
          <div className="flex-1">
            <div className="relative">
              <Search className="absolute left-3 top-1/2 transform -translate-y-1/2 h-4 w-4 text-gray-400" />
              <input
                type="text"
                placeholder="Search tracks, artists, albums..."
                value={searchQuery}
                onChange={(e) => setSearchQuery(e.target.value)}
                className="w-full pl-10 pr-4 py-2 bg-gray-700 border border-gray-600 rounded-lg text-white placeholder-gray-400 focus:outline-none focus:border-primary-500"
              />
            </div>
          </div>

          {/* Genre Filter */}
          <div className="w-full md:w-48">
            <select
              value={selectedGenre}
              onChange={(e) => setSelectedGenre(e.target.value)}
              className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-lg text-white focus:outline-none focus:border-primary-500"
            >
              <option value="">All Genres</option>
              {genres.map(genre => (
                <option key={genre} value={genre}>{genre}</option>
              ))}
            </select>
          </div>

          {/* Format Filter */}
          <div className="w-full md:w-48">
            <select
              value={selectedFormat}
              onChange={(e) => setSelectedFormat(e.target.value)}
              className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-lg text-white focus:outline-none focus:border-primary-500"
            >
              <option value="">All Formats</option>
              {formats.map(format => (
                <option key={format} value={format}>{format.toUpperCase()}</option>
              ))}
            </select>
          </div>

          {/* Sort */}
          <div className="w-full md:w-48">
            <select
              value={sortBy}
              onChange={(e) => setSortBy(e.target.value)}
              className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-lg text-white focus:outline-none focus:border-primary-500"
            >
              <option value="title">Title</option>
              <option value="artist">Artist</option>
              <option value="album">Album</option>
              <option value="genre">Genre</option>
              <option value="duration">Duration</option>
              <option value="play_count">Play Count</option>
              <option value="last_played">Last Played</option>
            </select>
          </div>

          {/* Sort Order */}
          <div className="w-full md:w-32">
            <select
              value={sortOrder}
              onChange={(e) => setSortOrder(e.target.value)}
              className="w-full px-3 py-2 bg-gray-700 border border-gray-600 rounded-lg text-white focus:outline-none focus:border-primary-500"
            >
              <option value="asc">↑ Asc</option>
              <option value="desc">↓ Desc</option>
            </select>
          </div>

          {/* Clear Filters */}
          <button
            onClick={clearFilters}
            className="px-4 py-2 bg-gray-600 text-white rounded-lg hover:bg-gray-500 transition-colors"
          >
            Clear
          </button>
        </div>
      </div>

      {/* Tracks List */}
      <div className="card">
        {filteredTracks.length === 0 ? (
          <div className="text-center py-12 text-gray-400">
            <Music className="h-16 w-16 mx-auto mb-4 opacity-50" />
            <p className="text-lg">No tracks found</p>
            <p className="text-sm">Try adjusting your search or filters</p>
          </div>
        ) : (
          <div className="space-y-2">
            {filteredTracks.map((track) => (
              <div 
                key={track.id} 
                className="flex items-center space-x-4 p-4 bg-gray-700 rounded-lg hover:bg-gray-600 transition-colors group"
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
                  <div>{formatFileSize(track.file_size_bytes)}</div>
                </div>
                
                <div className="flex-shrink-0 text-right text-sm text-gray-400">
                  <div>{track.play_count || 0} plays</div>
                  {track.last_played && (
                    <div className="text-xs">
                      {new Date(track.last_played).toLocaleDateString()}
                    </div>
                  )}
                </div>
                
                <div className="flex-shrink-0 flex space-x-2">
                  <button
                    onClick={() => play(track.id)}
                    className="p-2 bg-primary-600 rounded-lg text-white hover:bg-primary-500 transition-colors"
                  >
                    <Play className="h-4 w-4" />
                  </button>
                  
                  <button
                    onClick={() => handleDeleteTrack(track.id)}
                    className="p-2 bg-red-600 rounded-lg text-white hover:bg-red-500 transition-colors opacity-0 group-hover:opacity-100"
                  >
                    <Trash2 className="h-4 w-4" />
                  </button>
                </div>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
};

export default AudioLibrary;
