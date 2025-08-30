import React from 'react';
import { useAudio } from '../contexts/AudioContext';
import { Music, Clock, HardDrive, Play, Users } from 'lucide-react';

const LibraryStats: React.FC = () => {
  const { state } = useAudio();

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

  const formatFileSize = (bytes: number): string => {
    if (bytes === 0) return '0 B';
    
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
  };

  const stats = state.stats;

  if (!stats) {
    return (
      <div className="card">
        <h2 className="text-xl font-semibold text-white mb-4">Library Statistics</h2>
        <div className="text-center py-8 text-gray-400">
          <Music className="h-12 w-12 mx-auto mb-4 opacity-50" />
          <p>Loading statistics...</p>
        </div>
      </div>
    );
  }

  return (
    <div className="card">
      <h2 className="text-xl font-semibold text-white mb-4">Library Statistics</h2>
      
      <div className="space-y-4">
        {/* Basic Stats */}
        <div className="grid grid-cols-2 gap-4">
          <div className="bg-gray-700 rounded-lg p-4 text-center">
            <Music className="h-6 w-6 text-primary-400 mx-auto mb-2" />
            <div className="text-2xl font-bold text-white">{stats.total_tracks}</div>
            <div className="text-sm text-gray-400">Total Tracks</div>
          </div>
          
          <div className="bg-gray-700 rounded-lg p-4 text-center">
            <Play className="h-6 w-6 text-green-400 mx-auto mb-2" />
            <div className="text-2xl font-bold text-white">{stats.tracks_played}</div>
            <div className="text-sm text-gray-400">Tracks Played</div>
          </div>
        </div>

        {/* Duration Stats */}
        <div className="bg-gray-700 rounded-lg p-4">
          <div className="flex items-center space-x-2 mb-3">
            <Clock className="h-5 w-5 text-blue-400" />
            <span className="text-white font-medium">Duration</span>
          </div>
          <div className="text-2xl font-bold text-white">
            {formatDuration(stats.total_duration)}
          </div>
          <div className="text-sm text-gray-400">
            Total library duration
          </div>
        </div>

        {/* Storage Stats */}
        <div className="bg-gray-700 rounded-lg p-4">
          <div className="flex items-center space-x-2 mb-3">
            <HardDrive className="h-5 w-5 text-purple-400" />
            <span className="text-white font-medium">Storage</span>
          </div>
          <div className="text-2xl font-bold text-white">
            {formatFileSize(stats.library_size_bytes)}
          </div>
          <div className="text-sm text-gray-400">
            Library size on disk
          </div>
        </div>

        {/* Playlist Stats */}
        <div className="bg-gray-700 rounded-lg p-4">
          <div className="flex items-center space-x-2 mb-3">
            <Users className="h-5 w-5 text-yellow-400" />
            <span className="text-white font-medium">Playlists</span>
          </div>
          <div className="text-2xl font-bold text-white">
            {stats.total_playlists}
          </div>
          <div className="text-sm text-gray-400">
            Total playlists
          </div>
        </div>

        {/* Performance Stats */}
        <div className="bg-gray-700 rounded-lg p-4">
          <div className="text-sm text-gray-400 mb-2">Performance</div>
          <div className="space-y-2">
            <div className="flex justify-between">
              <span className="text-gray-300">Scan Time:</span>
              <span className="text-white">{stats.scan_time_seconds.toFixed(1)}s</span>
            </div>
            <div className="flex justify-between">
              <span className="text-gray-300">Uptime:</span>
              <span className="text-white">{formatDuration(stats.uptime_seconds)}</span>
            </div>
            <div className="flex justify-between">
              <span className="text-gray-300">Memory:</span>
              <span className="text-white">{stats.memory_usage_mb.toFixed(1)} MB</span>
            </div>
            <div className="flex justify-between">
              <span className="text-gray-300">CPU:</span>
              <span className="text-white">{stats.cpu_usage_percent.toFixed(1)}%</span>
            </div>
          </div>
        </div>

        {/* Last Scan */}
        {stats.last_scan && (
          <div className="bg-gray-700 rounded-lg p-4">
            <div className="text-sm text-gray-400 mb-2">Last Library Scan</div>
            <div className="text-white">
              {new Date(stats.last_scan).toLocaleString()}
            </div>
          </div>
        )}

        {/* Errors */}
        {stats.errors_count > 0 && (
          <div className="bg-red-900/20 border border-red-700 rounded-lg p-4">
            <div className="text-sm text-red-400 mb-2">⚠️ Errors Detected</div>
            <div className="text-white">
              {stats.errors_count} error(s) in library
            </div>
          </div>
        )}
      </div>
    </div>
  );
};

export default LibraryStats;
