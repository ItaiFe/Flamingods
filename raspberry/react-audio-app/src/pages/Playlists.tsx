import React, { useState, useEffect } from 'react';
import { useAudio } from '../contexts/AudioContext';
import { List, Play, Load, Music, Plus, Edit, Trash2, Shuffle, Repeat } from 'lucide-react';

const Playlists: React.FC = () => {
  const { state, loadPlaylists, playPlaylist, loadPlaylist } = useAudio();
  const [searchTerm, setSearchTerm] = useState('');

  useEffect(() => {
    loadPlaylists();
  }, [loadPlaylists]);

  // Filter playlists
  const filteredPlaylists = state.playlists.filter(playlist =>
    playlist.name.toLowerCase().includes(searchTerm.toLowerCase()) ||
    (playlist.description && playlist.description.toLowerCase().includes(searchTerm.toLowerCase()))
  );

  const formatDuration = (seconds: number): string => {
    if (!seconds || seconds < 0) return '0:00';
    
    const hours = Math.floor(seconds / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    
    if (hours > 0) {
      return `${hours}:${minutes.toString().padStart(2, '0')}:00`;
    } else {
      return `${minutes}:00`;
    }
  };

  const handlePlayPlaylist = (playlistId: string) => {
    playPlaylist(playlistId);
  };

  const handleLoadPlaylist = (playlistId: string) => {
    loadPlaylist(playlistId);
  };

  const formatDate = (dateString: string): string => {
    const date = new Date(dateString);
    return date.toLocaleDateString();
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold text-white">Playlists</h1>
          <p className="text-gray-400 mt-2">
            Manage and play your music playlists ({state.playlists.length} playlists)
          </p>
        </div>
        <button className="btn-primary flex items-center space-x-2">
          <Plus className="h-4 w-4" />
          <span>Create Playlist</span>
        </button>
      </div>

      {/* Search */}
      <div className="card">
        <div className="relative">
          <List className="absolute left-3 top-1/2 transform -translate-y-1/2 h-4 w-4 text-gray-400" />
          <input
            type="text"
            placeholder="Search playlists..."
            value={searchTerm}
            onChange={(e) => setSearchTerm(e.target.value)}
            className="input-field w-full pl-10"
          />
        </div>
      </div>

      {/* Playlists Grid */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
        {filteredPlaylists.map((playlist) => (
          <div key={playlist.id} className="card hover:bg-gray-750 transition-colors">
            {/* Playlist Header */}
            <div className="flex items-start justify-between mb-4">
              <div className="flex-1">
                <h3 className="text-lg font-semibold text-white mb-1">{playlist.name}</h3>
                {playlist.description && (
                  <p className="text-sm text-gray-400">{playlist.description}</p>
                )}
              </div>
              <div className="flex items-center space-x-1">
                {playlist.shuffle && (
                  <Shuffle className="h-4 w-4 text-primary-400" title="Shuffle enabled" />
                )}
                {playlist.repeat && (
                  <Repeat className="h-4 w-4 text-primary-400" title="Repeat enabled" />
                )}
              </div>
            </div>

            {/* Playlist Stats */}
            <div className="grid grid-cols-2 gap-4 mb-4">
              <div className="text-center">
                <div className="text-lg font-bold text-white">{playlist.tracks.length}</div>
                <div className="text-xs text-gray-400">Tracks</div>
              </div>
              <div className="text-center">
                <div className="text-lg font-bold text-white">
                  {formatDuration(playlist.total_duration || 0)}
                </div>
                <div className="text-xs text-gray-400">Duration</div>
              </div>
            </div>

            {/* Sample Tracks */}
            <div className="mb-4">
              <div className="text-xs text-gray-400 mb-2">Sample tracks:</div>
              <div className="space-y-1">
                {playlist.tracks.slice(0, 3).map((track) => (
                  <div key={track.id} className="flex items-center space-x-2 text-sm">
                    <Music className="h-3 w-3 text-gray-500" />
                    <span className="text-gray-300 truncate">{track.title}</span>
                    <span className="text-gray-500 text-xs">
                      {track.artist || 'Unknown Artist'}
                    </span>
                  </div>
                ))}
                {playlist.tracks.length > 3 && (
                  <div className="text-xs text-gray-500">
                    +{playlist.tracks.length - 3} more tracks
                  </div>
                )}
              </div>
            </div>

            {/* Playlist Info */}
            <div className="text-xs text-gray-500 mb-4">
              <div>Created: {formatDate(playlist.created_at)}</div>
              {playlist.last_modified !== playlist.created_at && (
                <div>Modified: {formatDate(playlist.last_modified)}</div>
              )}
              {playlist.last_played && (
                <div>Last played: {formatDate(playlist.last_played)}</div>
              )}
            </div>

            {/* Actions */}
            <div className="flex items-center space-x-2">
              <button
                onClick={() => handlePlayPlaylist(playlist.id)}
                className="btn-primary flex-1 flex items-center justify-center space-x-2"
              >
                <Play className="h-4 w-4" />
                <span>Play</span>
              </button>
              
              <button
                onClick={() => handleLoadPlaylist(playlist.id)}
                className="btn-secondary flex items-center justify-center space-x-2 px-3 py-2"
                title="Load playlist into queue"
              >
                <Load className="h-4 w-4" />
              </button>
              
              <button
                className="btn-secondary flex items-center justify-center space-x-2 px-3 py-2"
                title="Edit playlist"
              >
                <Edit className="h-4 w-4" />
              </button>
              
              <button
                className="btn-danger flex items-center justify-center space-x-2 px-3 py-2"
                title="Delete playlist"
              >
                <Trash2 className="h-4 w-4" />
              </button>
            </div>
          </div>
        ))}
      </div>

      {/* Empty State */}
      {filteredPlaylists.length === 0 && (
        <div className="text-center py-12 text-gray-400">
          <List className="h-16 w-16 mx-auto mb-4 opacity-50" />
          <p className="text-lg">No playlists found</p>
          <p className="text-sm">
            {searchTerm 
              ? 'Try adjusting your search'
              : 'Create your first playlist to get started!'
            }
          </p>
        </div>
      )}

      {/* Create Playlist Section */}
      <div className="card border-dashed border-2 border-gray-600 hover:border-primary-500 transition-colors">
        <div className="text-center py-8">
          <Plus className="h-12 w-12 mx-auto mb-4 text-gray-400" />
          <h3 className="text-lg font-medium text-white mb-2">Create New Playlist</h3>
          <p className="text-gray-400 mb-4">
            Organize your music into custom playlists
          </p>
          <button className="btn-primary">
            Create Playlist
          </button>
        </div>
      </div>
    </div>
  );
};

export default Playlists;
