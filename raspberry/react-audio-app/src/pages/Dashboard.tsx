import React, { useEffect } from 'react';
import { useAudio } from '../contexts/AudioContext';
import { Play, Pause, Stop, SkipBack, SkipForward, Volume2, VolumeX, RotateCcw } from 'lucide-react';
import PlaybackControls from '../components/PlaybackControls';
import LibraryStats from '../components/LibraryStats';
import RecentTracks from '../components/RecentTracks';
import PopularTracks from '../components/PopularTracks';
import ErrorAlert from '../components/ErrorAlert';

const Dashboard: React.FC = () => {
  const { state, loadStats, getPopularTracks, getRecentTracks, refreshLibrary } = useAudio();

  useEffect(() => {
    // Load dashboard data
    const loadDashboardData = async () => {
      await Promise.all([
        loadStats(),
        getPopularTracks(5),
        getRecentTracks(5),
      ]);
    };

    loadDashboardData();
  }, [loadStats, getPopularTracks, getRecentTracks]);

  const handleRefreshLibrary = async () => {
    await refreshLibrary();
  };

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
        <button
          onClick={handleRefreshLibrary}
          disabled={state.isLoading}
          className="btn-secondary flex items-center space-x-2"
        >
          <RotateCcw className={`h-4 w-4 ${state.isLoading ? 'animate-spin' : ''}`} />
          <span>Refresh Library</span>
        </button>
      </div>

      {/* Error Alert */}
      {state.error && <ErrorAlert message={state.error} />}

      {/* Playback Controls */}
      <div className="card">
        <h2 className="text-xl font-semibold text-white mb-4">Playback Controls</h2>
        <PlaybackControls />
      </div>

      {/* Library Overview */}
      <div className="grid grid-cols-1 lg:grid-cols-3 gap-6">
        {/* Library Stats */}
        <div className="lg:col-span-1">
          <LibraryStats />
        </div>

        {/* Current Track Info */}
        <div className="lg:col-span-2">
          <div className="card">
            <h2 className="text-xl font-semibold text-white mb-4">Now Playing</h2>
            {state.currentTrack ? (
              <div className="space-y-4">
                <div className="flex items-center space-x-4">
                  <div className="w-16 h-16 bg-primary-600 rounded-lg flex items-center justify-center">
                    <Music className="h-8 w-8 text-white" />
                  </div>
                  <div className="flex-1">
                    <h3 className="text-lg font-medium text-white">{state.currentTrack.title}</h3>
                    <p className="text-gray-400">
                      {state.currentTrack.artist || 'Unknown Artist'}
                    </p>
                    {state.currentTrack.album && (
                      <p className="text-sm text-gray-500">{state.currentTrack.album}</p>
                    )}
                  </div>
                </div>

                {/* Progress Bar */}
                {state.playbackStatus && (
                  <div className="space-y-2">
                    <div className="flex justify-between text-sm text-gray-400">
                      <span>{formatTime(state.playbackStatus.position_seconds)}</span>
                      <span>{formatTime(state.playbackStatus.duration_seconds)}</span>
                    </div>
                    <div className="w-full bg-gray-700 rounded-full h-2">
                      <div
                        className="bg-primary-600 h-2 rounded-full transition-all duration-300"
                        style={{ width: `${state.playbackStatus.progress_percent}%` }}
                      />
                    </div>
                  </div>
                )}

                {/* Playback State */}
                <div className="flex items-center space-x-2">
                  <span className="text-sm text-gray-400">Status:</span>
                  <span
                    className={`px-2 py-1 rounded-full text-xs font-medium ${
                      state.isPlaying
                        ? 'bg-green-600 text-white'
                        : state.isPaused
                        ? 'bg-yellow-600 text-white'
                        : 'bg-gray-600 text-white'
                    }`}
                  >
                    {state.playbackStatus?.state || 'stopped'}
                  </span>
                </div>
              </div>
            ) : (
              <div className="text-center py-8 text-gray-400">
                <Music className="h-12 w-12 mx-auto mb-4 opacity-50" />
                <p>No track currently playing</p>
                <p className="text-sm">Select a track from your library to start listening</p>
              </div>
            )}
          </div>
        </div>
      </div>

      {/* Quick Actions */}
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        {/* Recent Tracks */}
        <div className="card">
          <h2 className="text-xl font-semibold text-white mb-4">Recent Tracks</h2>
          <RecentTracks />
        </div>

        {/* Popular Tracks */}
        <div className="card">
          <h2 className="text-xl font-semibold text-white mb-4">Popular Tracks</h2>
          <PopularTracks />
        </div>
      </div>

      {/* Quick Selection */}
      <div className="card">
        <h2 className="text-xl font-semibold text-white mb-4">Quick Selection</h2>
        <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
          <button
            onClick={() => getRandomTrack()}
            className="btn-secondary flex flex-col items-center space-y-2 py-4"
          >
            <Shuffle className="h-6 w-6" />
            <span>Random Track</span>
          </button>
          <button
            onClick={() => getPopularTracks(1)}
            className="btn-secondary flex flex-col items-center space-y-2 py-4"
          >
            <TrendingUp className="h-6 w-6" />
            <span>Popular</span>
          </button>
          <button
            onClick={() => getRecentTracks(1)}
            className="btn-secondary flex flex-col items-center space-y-2 py-4"
          >
            <Clock className="h-6 w-6" />
            <span>Recent</span>
          </button>
          <button
            onClick={() => getTracksByCategory('ambient', 1)}
            className="btn-secondary flex flex-col items-center space-y-2 py-4"
          >
            <Zap className="h-6 w-6" />
            <span>Ambient</span>
          </button>
        </div>
      </div>
    </div>
  );
};

// Helper function to format time
const formatTime = (seconds: number): string => {
  if (!seconds || seconds < 0) return '0:00';
  
  const minutes = Math.floor(seconds / 60);
  const remainingSeconds = Math.floor(seconds % 60);
  return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
};

export default Dashboard;
