import React, { useState, useEffect } from 'react';
import { useAudio } from '../contexts/AudioContext';
import { Music, Play, Trash2, Search, Filter } from 'lucide-react';

const Library: React.FC = () => {
  const { state, loadTracks, play, deleteSong } = useAudio();
  const [searchTerm, setSearchTerm] = useState('');
  const [selectedGenre, setSelectedGenre] = useState<string>('');
  const [sortBy, setSortBy] = useState<'title' | 'artist' | 'album' | 'duration' | 'date'>('title');
  const [sortOrder, setSortOrder] = useState<'asc' | 'desc'>('asc');

  useEffect(() => {
    loadTracks();
  }, [loadTracks]);

  // Filter and sort tracks
  const filteredTracks = state.tracks
    .filter(track => {
      const matchesSearch = !searchTerm || 
        track.title.toLowerCase().includes(searchTerm.toLowerCase()) ||
        (track.artist && track.artist.toLowerCase().includes(searchTerm.toLowerCase())) ||
        (track.album && track.album.toLowerCase().includes(searchTerm.toLowerCase()));
      
      const matchesGenre = !selectedGenre || track.genre === selectedGenre;
      
      return matchesSearch && matchesGenre;
    })
    .sort((a, b) => {
      let aValue: any, bValue: any;
      
      switch (sortBy) {
        case 'title':
          aValue = a.title.toLowerCase();
          bValue = b.title.toLowerCase();
          break;
        case 'artist':
          aValue = (a.artist || '').toLowerCase();
          bValue = (b.artist || '').toLowerCase();
          break;
        case 'album':
          aValue = (a.album || '').toLowerCase();
          bValue = (b.album || '').toLowerCase();
          break;
        case 'duration':
          aValue = a.duration_seconds || 0;
          bValue = b.duration_seconds || 0;
          break;
        case 'date':
          aValue = new Date(a.created_at).getTime();
          bValue = new Date(b.created_at).getTime();
          break;
        default:
          aValue = a.title.toLowerCase();
          bValue = b.title.toLowerCase();
      }
      
      if (sortOrder === 'asc') {
        return aValue < bValue ? -1 : aValue > bValue ? 1 : 0;
      } else {
        return aValue > bValue ? -1 : aValue < bValue ? 1 : 0;
      }
    });

  // Get unique genres
  const genres = Array.from(new Set(state.tracks.map(track => track.genre).filter(Boolean)));

  const formatDuration = (seconds: number): string => {
    if (!seconds || seconds < 0) return '0:00';
    
    const minutes = Math.floor(seconds / 60);
    const remainingSeconds = Math.floor(seconds % 60);
    return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
  };

  const formatFileSize = (bytes: number): string => {
    if (bytes === 0) return '0 B';
    
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const handlePlayTrack = (trackId: string) => {
    play({ track_id: trackId });
  };

  const handleDeleteTrack = async (trackId: string) => {
    if (window.confirm('Are you sure you want to delete this track?')) {
      await deleteSong(trackId);
    }
  };

  const toggleSortOrder = () => {
    setSortOrder(sortOrder === 'asc' ? 'desc' : 'asc');
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold text-white">Music Library</h1>
          <p className="text-gray-400 mt-2">
            Browse and manage your music collection ({state.tracks.length} tracks)
          </p>
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
                placeholder="Search tracks, artists, or albums..."
                value={searchTerm}
                onChange={(e) => setSearchTerm(e.target.value)}
                className="input-field w-full pl-10"
              />
            </div>
          </div>

          {/* Genre Filter */}
          <div className="md:w-48">
            <select
              value={selectedGenre}
              onChange={(e) => setSelectedGenre(e.target.value)}
              className="input-field w-full"
            >
              <option value="">All Genres</option>
              {genres.map(genre => (
                <option key={genre} value={genre}>{genre}</option>
              ))}
            </select>
          </div>

          {/* Sort */}
          <div className="md:w-48">
            <select
              value={sortBy}
              onChange={(e) => setSortBy(e.target.value as any)}
              className="input-field w-full"
            >
              <option value="title">Title</option>
              <option value="artist">Artist</option>
              <option value="album">Album</option>
              <option value="duration">Duration</option>
              <option value="date">Date Added</option>
            </select>
          </div>

          {/* Sort Order */}
          <button
            onClick={toggleSortOrder}
            className="btn-secondary flex items-center space-x-2"
          >
            <Filter className="h-4 w-4" />
            <span>{sortOrder === 'asc' ? '↑' : '↓'}</span>
          </button>
        </div>
      </div>

      {/* Tracks List */}
      <div className="card">
        <div className="overflow-x-auto">
          <table className="w-full">
            <thead>
              <tr className="border-b border-gray-700">
                <th className="text-left p-3 text-gray-400 font-medium">Track</th>
                <th className="text-left p-3 text-gray-400 font-medium">Artist</th>
                <th className="text-left p-3 text-gray-400 font-medium">Album</th>
                <th className="text-left p-3 text-gray-400 font-medium">Genre</th>
                <th className="text-left p-3 text-gray-400 font-medium">Duration</th>
                <th className="text-left p-3 text-gray-400 font-medium">Size</th>
                <th className="text-left p-3 text-gray-400 font-medium">Plays</th>
                <th className="text-left p-3 text-gray-400 font-medium">Actions</th>
              </tr>
            </thead>
            <tbody>
              {filteredTracks.map((track) => (
                <tr
                  key={track.id}
                  className="border-b border-gray-800 hover:bg-gray-800/50 transition-colors"
                >
                  <td className="p-3">
                    <div className="flex items-center space-x-3">
                      <div className="w-10 h-10 bg-primary-600 rounded-lg flex items-center justify-center">
                        <Music className="h-5 w-5 text-white" />
                      </div>
                      <div>
                        <div className="font-medium text-white">{track.title}</div>
                        <div className="text-sm text-gray-400">{track.format.toUpperCase()}</div>
                      </div>
                    </div>
                  </td>
                  <td className="p-3 text-gray-300">
                    {track.artist || 'Unknown Artist'}
                  </td>
                  <td className="p-3 text-gray-300">
                    {track.album || 'Unknown Album'}
                  </td>
                  <td className="p-3">
                    {track.genre && (
                      <span className="px-2 py-1 bg-gray-700 rounded-full text-xs text-gray-300">
                        {track.genre}
                      </span>
                    )}
                  </td>
                  <td className="p-3 text-gray-300">
                    {formatDuration(track.duration_seconds || 0)}
                  </td>
                  <td className="p-3 text-gray-300">
                    {formatFileSize(track.size_bytes)}
                  </td>
                  <td className="p-3 text-gray-300">
                    {track.play_count}
                  </td>
                  <td className="p-3">
                    <div className="flex items-center space-x-2">
                      <button
                        onClick={() => handlePlayTrack(track.id)}
                        className="p-2 bg-primary-600 hover:bg-primary-700 rounded-lg transition-colors"
                        title="Play track"
                      >
                        <Play className="h-4 w-4 text-white" />
                      </button>
                      
                      <button
                        onClick={() => handleDeleteTrack(track.id)}
                        className="p-2 bg-red-600 hover:bg-red-700 rounded-lg transition-colors"
                        title="Delete track"
                      >
                        <Trash2 className="h-4 w-4 text-white" />
                      </button>
                    </div>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>

        {filteredTracks.length === 0 && (
          <div className="text-center py-12 text-gray-400">
            <Music className="h-16 w-16 mx-auto mb-4 opacity-50" />
            <p className="text-lg">No tracks found</p>
            <p className="text-sm">
              {searchTerm || selectedGenre 
                ? 'Try adjusting your search or filters'
                : 'Your library is empty. Upload some music to get started!'
              }
            </p>
          </div>
        )}
      </div>

      {/* Library Stats */}
      <div className="grid grid-cols-1 md:grid-cols-4 gap-4">
        <div className="bg-gray-800 rounded-lg p-4 text-center">
          <div className="text-2xl font-bold text-white">{filteredTracks.length}</div>
          <div className="text-sm text-gray-400">Tracks Found</div>
        </div>
        
        <div className="bg-gray-800 rounded-lg p-4 text-center">
          <div className="text-2xl font-bold text-white">
            {formatDuration(filteredTracks.reduce((total, track) => total + (track.duration_seconds || 0), 0))}
          </div>
          <div className="text-sm text-gray-400">Total Duration</div>
        </div>
        
        <div className="bg-gray-800 rounded-lg p-4 text-center">
          <div className="text-2xl font-bold text-white">
            {formatFileSize(filteredTracks.reduce((total, track) => total + track.size_bytes, 0))}
          </div>
          <div className="text-sm text-gray-400">Total Size</div>
        </div>
        
        <div className="bg-gray-800 rounded-lg p-4 text-center">
          <div className="text-2xl font-bold text-white">
            {filteredTracks.reduce((total, track) => total + track.play_count, 0)}
          </div>
          <div className="text-sm text-gray-400">Total Plays</div>
        </div>
      </div>
    </div>
  );
};

export default Library;
