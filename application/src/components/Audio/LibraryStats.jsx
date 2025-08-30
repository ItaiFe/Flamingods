import React from 'react';
import { useAudio } from './AudioContext';
import { Database, Clock, HardDrive, Play, TrendingUp, ListMusic } from 'lucide-react';

const LibraryStats = () => {
  const { state } = useAudio();
  const { tracks, playlists, stats } = state;

  const formatFileSize = (bytes) => {
    if (!bytes || bytes === 0) return '0 B';
    
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    
    return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
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

  const formatDate = (dateString) => {
    if (!dateString) return 'Never';
    return new Date(dateString).toLocaleDateString();
  };

  return (
    <div className="space-y-4">
      <h2 className="text-xl font-semibold text-white">Library Statistics</h2>
      
      {/* Basic Stats */}
      <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
        <div className="bg-gray-700 rounded-lg p-4 text-center">
          <Database className="h-8 w-8 text-primary-400 mx-auto mb-2" />
          <div className="text-2xl font-bold text-white">{tracks.length}</div>
          <div className="text-sm text-gray-400">Total Tracks</div>
        </div>
        
        <div className="bg-gray-700 rounded-lg p-4 text-center">
          <ListMusic className="h-8 w-8 text-primary-400 mx-auto mb-2" />
          <div className="text-2xl font-bold text-white">{playlists.length}</div>
          <div className="text-sm text-gray-400">Playlists</div>
        </div>
        
        <div className="bg-gray-700 rounded-lg p-4 text-center">
          <Clock className="h-8 w-8 text-primary-400 mx-auto mb-2" />
          <div className="text-lg font-bold text-white">
            {stats ? formatDuration(stats.total_duration) : '0m'}
          </div>
          <div className="text-sm text-gray-400">Total Duration</div>
        </div>
        
        <div className="bg-gray-700 rounded-lg p-4 text-center">
          <HardDrive className="h-8 w-8 text-primary-400 mx-auto mb-2" />
          <div className="text-lg font-bold text-white">
            {stats ? formatFileSize(stats.library_size_bytes) : '0 B'}
          </div>
          <div className="text-sm text-gray-400">Library Size</div>
        </div>
      </div>

      {/* Detailed Stats */}
      {stats && (
        <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
          <div className="bg-gray-700 rounded-lg p-4">
            <div className="flex items-center space-x-2 mb-3">
              <Play className="h-5 w-5 text-primary-400" />
              <h3 className="font-semibold text-white">Playback Stats</h3>
            </div>
            <div className="space-y-2 text-sm">
              <div className="flex justify-between">
                <span className="text-gray-400">Tracks Played:</span>
                <span className="text-white">{stats.tracks_played}</span>
              </div>
              <div className="flex justify-between">
                <span className="text-gray-400">Total Plays:</span>
                <span className="text-white">{stats.total_plays}</span>
              </div>
              <div className="flex justify-between">
                <span className="text-gray-400">Avg Play Time:</span>
                <span className="text-white">{formatDuration(stats.avg_play_time_seconds)}</span>
              </div>
            </div>
          </div>
          
          <div className="bg-gray-700 rounded-lg p-4">
            <div className="flex items-center space-x-2 mb-3">
              <TrendingUp className="h-5 w-5 text-primary-400" />
              <h3 className="font-semibold text-white">Performance</h3>
            </div>
            <div className="space-y-2 text-sm">
              <div className="flex justify-between">
                <span className="text-gray-400">Scan Time:</span>
                <span className="text-white">{stats.scan_time_seconds ? stats.scan_time_seconds.toFixed(1) + 's' : 'N/A'}</span>
              </div>
              <div className="flex justify-between">
                <span className="text-gray-400">Memory Usage:</span>
                <span className="text-white">{Math.round(stats.memory_usage_mb)} MB</span>
              </div>
              <div className="flex justify-between">
                <span className="text-gray-400">CPU Usage:</span>
                <span className="text-white">{stats.cpu_usage_percent ? stats.cpu_usage_percent.toFixed(1) + '%' : 'N/A'}</span>
              </div>
            </div>
          </div>
          
          <div className="bg-gray-700 rounded-lg p-4">
            <div className="flex items-center space-x-2 mb-3">
              <Clock className="h-5 w-5 text-primary-400" />
              <h3 className="font-semibold text-white">System Info</h3>
            </div>
            <div className="space-y-2 text-sm">
              <div className="flex justify-between">
                <span className="text-gray-400">Uptime:</span>
                <span className="text-white">{formatDuration(stats.uptime_seconds)}</span>
              </div>
              <div className="flex justify-between">
                <span className="text-gray-400">Last Scan:</span>
                <span className="text-white">{formatDate(stats.last_scan)}</span>
              </div>
              <div className="flex justify-between">
                <span className="text-gray-400">Version:</span>
                <span className="text-white">1.0.0</span>
              </div>
            </div>
          </div>
        </div>
      )}

      {/* Format Distribution */}
      {tracks.length > 0 && (
        <div className="bg-gray-700 rounded-lg p-4">
          <h3 className="font-semibold text-white mb-3">Format Distribution</h3>
          <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
            {Object.entries(
              tracks.reduce((acc, track) => {
                acc[track.format] = (acc[track.format] || 0) + 1;
                return acc;
              }, {})
            ).map(([format, count]) => (
              <div key={format} className="text-center">
                <div className="text-lg font-semibold text-white">{count}</div>
                <div className="text-sm text-gray-400">{format.toUpperCase()}</div>
              </div>
            ))}
          </div>
        </div>
      )}
    </div>
  );
};

export default LibraryStats;
