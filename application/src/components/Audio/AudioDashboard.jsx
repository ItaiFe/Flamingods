import React from 'react';
import { useAudio } from './AudioContext';
import PlaybackControls from './PlaybackControls';
import LibraryStats from './LibraryStats';
import RecentTracks from './RecentTracks';
import PopularTracks from './PopularTracks';
import ErrorAlert from '../Layout/ErrorAlert';
import { Music, Shuffle, TrendingUp, Clock, Zap } from 'lucide-react';

const AudioDashboard = () => {
  const { 
    state, 
    play, 
    getRandomTrack, 
    getTracksByCategory 
  } = useAudio();

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

  if (state.loading) {
    return (
      <div className="flex items-center justify-center h-64">
        <div className="animate-spin rounded-full h-12 w-12 border-b-2 border-primary-500"></div>
      </div>
    );
  }

  return (
    <div className="space-y-6">
      {/* Header */}
      <div className="flex items-center justify-between">
        <div>
          <h1 className="text-3xl font-bold text-white">Audio Dashboard</h1>
          <p className="text-gray-400 mt-2">
            Control your music playback and manage your audio library
          </p>
        </div>
        <div className="flex items-center space-x-2 text-primary-400">
          <Music className="h-8 w-8" />
        </div>
      </div>

      {/* Error Display */}
      {state.error && (
        <ErrorAlert 
          message={state.error} 
          onClose={() => {}} // Error handling is done in context
        />
      )}

      {/* Playback Controls */}
      <div className="card">
        <PlaybackControls />
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

      {/* Library Statistics */}
      <div className="card">
        <LibraryStats />
      </div>

      {/* Recent and Popular Tracks */}
      <div className="grid grid-cols-1 lg:grid-cols-2 gap-6">
        <div className="card">
          <RecentTracks />
        </div>
        
        <div className="card">
          <PopularTracks />
        </div>
      </div>

      {/* System Status */}
      {state.stats && (
        <div className="card">
          <h2 className="text-xl font-semibold text-white mb-4">System Status</h2>
          <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
            <div className="bg-gray-700 rounded-lg p-4">
              <div className="text-sm text-gray-400">Server Uptime</div>
              <div className="text-lg font-semibold text-white">
                {Math.floor(state.stats.uptime_seconds / 3600)}h {Math.floor((state.stats.uptime_seconds % 3600) / 60)}m
              </div>
            </div>
            
            <div className="bg-gray-700 rounded-lg p-4">
              <div className="text-sm text-gray-400">Last Scan</div>
              <div className="text-lg font-semibold text-white">
                {state.stats.last_scan ? 
                  new Date(state.stats.last_scan).toLocaleDateString() : 
                  'Never'
                }
              </div>
            </div>
            
            <div className="bg-gray-700 rounded-lg p-4">
              <div className="text-sm text-gray-400">Memory Usage</div>
              <div className="text-lg font-semibold text-white">
                {Math.round(state.stats.memory_usage_mb)} MB
              </div>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default AudioDashboard;
