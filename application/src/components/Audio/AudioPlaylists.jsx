import React, { useState } from 'react';
import { useAudio } from './AudioContext';
import { ListMusic, Play, Loader, Edit, Trash2, Plus, Music } from 'lucide-react';

const AudioPlaylists = () => {
  const { state, loadPlaylist, play } = useAudio();
  const { playlists, tracks } = state;
  
  const [selectedPlaylist, setSelectedPlaylist] = useState(null);
  const [isLoading, setIsLoading] = useState(false);

  const handleLoadPlaylist = async (playlistId) => {
    setIsLoading(true);
    try {
      await loadPlaylist(playlistId);
      setSelectedPlaylist(playlists.find(p => p.id === playlistId));
    } catch (error) {
      console.error('Failed to load playlist:', error);
    } finally {
      setIsLoading(false);
    }
  };

  const handlePlayPlaylist = async (playlistId) => {
    try {
      await loadPlaylist(playlistId);
      const playlist = playlists.find(p => p.id === playlistId);
      if (playlist.tracks && playlist.tracks.length > 0) {
        await play(playlist.tracks[0].id);
      }
    } catch (error) {
      console.error('Failed to play playlist:', error);
    }
  };

  const formatTime = (seconds) => {
    if (!seconds || seconds < 0) return '0:00';
    const minutes = Math.floor(seconds / 60);
    const remainingSeconds = Math.floor(seconds % 60);
    return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
  };

  const formatDuration = (seconds) => {
    if (!seconds || seconds < 0) return '0:00';
    const hours = Math.floor(seconds / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    if (hours > 0) {
      return `${hours}h ${minutes}m`;
    } else {
      return `${minutes}m`;
    }
  };

  if (playlists.length === 0) {
    return (
      <div className="space-y-6">
        <div className="flex items-center justify-between">
          <div>
            <h1 className="text-3xl font-bold text-white">Playlists</h1>
            <p className="text-gray-400 mt-2">
              Create and manage your music playlists
            </p>
          </div>
          <div className="flex items-center space-x-2 text-primary-400">
            <ListMusic className="h-8 w-8" />
          </div>
        </div>

        <div className="text-center py-16 text-gray-400">
          <ListMusic className="h-24 w-24 mx-auto mb-6 opacity-50" />
          <h3 className="text-xl font-semibold mb-2">No Playlists Yet</h3>
          <p className="mb-6">Create your first playlist to get started</p>
          <button className="btn-primary flex items-center space-x-2 mx-auto">
            <Plus className="h-5 w-5" />
            <span>Create Playlist</span>
          </button>
        </div>
      </div>
    );
  }

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold text-white">Playlists</h1>
          <p className="text-gray-400 mt-2">
            Create and manage your music playlists
          </p>
        </div>
        <div className="flex items-center space-x-2 text-primary-400">
          <ListMusic className="h-8 w-8" />
        </div>
      </div>

      {/* Playlists Grid */}
      <div className="grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
        {playlists.map((playlist) => (
          <div 
            key={playlist.id} 
            className="bg-gray-700 rounded-lg p-6 hover:bg-gray-600 transition-colors cursor-pointer"
            onClick={() => setSelectedPlaylist(playlist)}
          >
            <div className="flex items-center justify-between mb-4">
              <div className="w-12 h-12 bg-primary-600 rounded-lg flex items-center justify-center">
                <ListMusic className="h-6 w-6 text-white" />
              </div>
              <div className="flex space-x-2">
                <button
                  onClick={(e) => {
                    e.stopPropagation();
                    handleLoadPlaylist(playlist.id);
                  }}
                  className="p-2 bg-primary-600 rounded-lg text-white hover:bg-primary-500 transition-colors"
                  disabled={isLoading}
                >
                  {isLoading ? (
                    <Loader className="h-4 w-4 animate-spin" />
                  ) : (
                    <Loader className="h-4 w-4" />
                  )}
                </button>
                
                <button
                  onClick={(e) => {
                    e.stopPropagation();
                    handlePlayPlaylist(playlist.id);
                  }}
                  className="p-2 bg-green-600 rounded-lg text-white hover:bg-green-500 transition-colors"
                >
                  <Play className="h-4 w-4" />
                </button>
              </div>
            </div>

            <h3 className="text-lg font-semibold text-white mb-2">{playlist.name}</h3>
            <p className="text-gray-400 text-sm mb-4">{playlist.description}</p>

            <div className="space-y-2 text-sm text-gray-400">
              <div className="flex justify-between">
                <span>Tracks:</span>
                <span>{playlist.tracks ? playlist.tracks.length : 0}</span>
              </div>
              <div className="flex justify-between">
                <span>Duration:</span>
                <span>{formatDuration(playlist.total_duration)}</span>
              </div>
              <div className="flex justify-between">
                <span>Created:</span>
                <span>{new Date(playlist.created_at).toLocaleDateString()}</span>
              </div>
            </div>

            {playlist.tracks && playlist.tracks.length > 0 && (
              <div className="mt-4 pt-4 border-t border-gray-600">
                <h4 className="text-sm font-medium text-white mb-2">Preview:</h4>
                <div className="space-y-1">
                  {playlist.tracks.slice(0, 3).map((track) => (
                    <div key={track.id} className="flex items-center space-x-2 text-xs">
                      <Music className="h-3 w-3 text-gray-500" />
                      <span className="text-gray-400 truncate">{track.title}</span>
                      <span className="text-gray-500">•</span>
                      <span className="text-gray-500">{track.artist}</span>
                    </div>
                  ))}
                  {playlist.tracks.length > 3 && (
                    <div className="text-xs text-gray-500">
                      +{playlist.tracks.length - 3} more tracks
                    </div>
                  )}
                </div>
              </div>
            )}
          </div>
        ))}
      </div>

      {/* Create New Playlist Button */}
      <div className="text-center">
        <button className="btn-primary flex items-center space-x-2 mx-auto">
          <Plus className="h-5 w-5" />
          <span>Create New Playlist</span>
        </button>
      </div>

      {/* Playlist Details Modal */}
      {selectedPlaylist && (
        <div className="fixed inset-0 bg-black bg-opacity-50 flex items-center justify-center z-50">
          <div className="bg-gray-800 rounded-lg p-6 max-w-2xl w-full mx-4 max-h-[80vh] overflow-y-auto">
            <div className="flex items-center justify-between mb-4">
              <h2 className="text-xl font-semibold text-white">{selectedPlaylist.name}</h2>
              <button
                onClick={() => setSelectedPlaylist(null)}
                className="text-gray-400 hover:text-white"
              >
                ✕
              </button>
            </div>

            <p className="text-gray-400 mb-4">{selectedPlaylist.description}</p>

            <div className="grid grid-cols-2 md:grid-cols-4 gap-4 mb-6">
              <div className="bg-gray-700 rounded-lg p-3 text-center">
                <div className="text-lg font-bold text-white">
                  {selectedPlaylist.tracks ? selectedPlaylist.tracks.length : 0}
                </div>
                <div className="text-sm text-gray-400">Tracks</div>
              </div>
              
              <div className="bg-gray-700 rounded-lg p-3 text-center">
                <div className="text-lg font-bold text-white">
                  {formatDuration(selectedPlaylist.total_duration)}
                </div>
                <div className="text-sm text-gray-400">Duration</div>
              </div>
              
              <div className="bg-gray-700 rounded-lg p-3 text-center">
                <div className="text-sm text-white">
                  {selectedPlaylist.shuffle ? 'Yes' : 'No'}
                </div>
                <div className="text-sm text-gray-400">Shuffle</div>
              </div>
              
              <div className="bg-gray-700 rounded-lg p-3 text-center">
                <div className="text-sm text-white">
                  {selectedPlaylist.repeat ? 'Yes' : 'No'}
                </div>
                <div className="text-sm text-gray-400">Repeat</div>
              </div>
            </div>

            {selectedPlaylist.tracks && selectedPlaylist.tracks.length > 0 ? (
              <div className="space-y-2">
                <h3 className="font-medium text-white">Tracks:</h3>
                {selectedPlaylist.tracks.map((track, index) => (
                  <div 
                    key={track.id} 
                    className="flex items-center space-x-3 p-3 bg-gray-700 rounded-lg hover:bg-gray-600 transition-colors cursor-pointer"
                    onClick={() => play(track.id)}
                  >
                    <div className="w-8 h-8 bg-primary-600 rounded-full flex items-center justify-center text-white font-bold text-sm">
                      {index + 1}
                    </div>
                    
                    <div className="flex-1 min-w-0">
                      <div className="text-white font-medium truncate">{track.title}</div>
                      <div className="text-gray-400 text-sm truncate">{track.artist}</div>
                    </div>
                    
                    <div className="text-gray-400 text-sm">
                      {formatTime(track.duration)}
                    </div>
                    
                    <button 
                      className="p-2 bg-primary-600 rounded-lg text-white hover:bg-primary-500 transition-colors"
                      onClick={(e) => {
                        e.stopPropagation();
                        play(track.id);
                      }}
                    >
                      <Play className="h-4 w-4" />
                    </button>
                  </div>
                ))}
              </div>
            ) : (
              <div className="text-center py-8 text-gray-400">
                <Music className="h-12 w-12 mx-auto mb-3 opacity-50" />
                <p>No tracks in this playlist</p>
              </div>
            )}

            <div className="flex space-x-3 mt-6">
              <button
                onClick={() => handlePlayPlaylist(selectedPlaylist.id)}
                className="btn-primary flex-1"
              >
                Play Playlist
              </button>
              
              <button
                onClick={() => handleLoadPlaylist(selectedPlaylist.id)}
                className="btn-secondary flex-1"
              >
                Load to Queue
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default AudioPlaylists;
