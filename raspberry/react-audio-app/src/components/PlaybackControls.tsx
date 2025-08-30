import React, { useState } from 'react';
import { useAudio } from '../contexts/AudioContext';
import { Play, Pause, Stop, SkipBack, SkipForward, Volume2, VolumeX } from 'lucide-react';

const PlaybackControls: React.FC = () => {
  const { state, play, pause, stop, next, previous, setVolume, toggleMute } = useAudio();
  const [volume, setVolumeLocal] = useState(state.volume || 70);

  const handleVolumeChange = (newVolume: number) => {
    setVolumeLocal(newVolume);
    setVolume(newVolume);
  };

  const handlePlay = () => {
    if (state.isPaused) {
      // Resume playback
      play();
    } else if (state.isStopped) {
      // Start playing first available track
      play();
    }
  };

  return (
    <div className="space-y-6">
      {/* Main Playback Controls */}
      <div className="flex items-center justify-center space-x-4">
        <button
          onClick={previous}
          disabled={!state.currentTrack}
          className="p-3 rounded-full bg-gray-700 hover:bg-gray-600 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
        >
          <SkipBack className="h-6 w-6 text-white" />
        </button>

        <button
          onClick={handlePlay}
          disabled={!state.currentTrack}
          className="p-4 rounded-full bg-primary-600 hover:bg-primary-700 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
        >
          {state.isPlaying ? (
            <Pause className="h-8 w-8 text-white" />
          ) : (
            <Play className="h-8 w-8 text-white" />
          )}
        </button>

        <button
          onClick={stop}
          disabled={!state.currentTrack}
          className="p-3 rounded-full bg-gray-700 hover:bg-gray-600 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
        >
          <Stop className="h-6 w-6 text-white" />
        </button>

        <button
          onClick={next}
          disabled={!state.currentTrack}
          className="p-3 rounded-full bg-gray-700 hover:bg-gray-600 disabled:opacity-50 disabled:cursor-not-allowed transition-colors"
        >
          <SkipForward className="h-6 w-6 text-white" />
        </button>
      </div>

      {/* Volume Controls */}
      <div className="flex items-center justify-center space-x-4">
        <button
          onClick={toggleMute}
          className="p-2 rounded-lg bg-gray-700 hover:bg-gray-600 transition-colors"
        >
          {state.muted ? (
            <VolumeX className="h-5 w-5 text-white" />
          ) : (
            <Volume2 className="h-5 w-5 text-white" />
          )}
        </button>

        <div className="flex items-center space-x-2 min-w-[200px]">
          <span className="text-sm text-gray-400 w-12">0%</span>
          <input
            type="range"
            min="0"
            max="100"
            value={state.muted ? 0 : volume}
            onChange={(e) => handleVolumeChange(parseInt(e.target.value))}
            className="flex-1 h-2 bg-gray-700 rounded-lg appearance-none cursor-pointer slider"
          />
          <span className="text-sm text-gray-400 w-12">{volume}%</span>
        </div>
      </div>

      {/* Playback Status */}
      <div className="text-center">
        <div className="text-sm text-gray-400">
          {state.currentTrack ? (
            <span>
              Now playing: <span className="text-white font-medium">{state.currentTrack.title}</span>
              {state.currentTrack.artist && (
                <span> by <span className="text-white font-medium">{state.currentTrack.artist}</span></span>
              )}
            </span>
          ) : (
            <span>No track selected</span>
          )}
        </div>
        
        {state.currentPlaylist && (
          <div className="text-xs text-gray-500 mt-1">
            Playlist: {state.currentPlaylist.name} 
            ({state.playbackStatus?.queue_position || 0 + 1} of {state.playbackStatus?.queue_length || 0})
          </div>
        )}
      </div>

      {/* Quick Actions */}
      <div className="flex justify-center space-x-4">
        <button
          onClick={() => play()}
          disabled={state.isPlaying}
          className="px-4 py-2 bg-green-600 hover:bg-green-700 disabled:opacity-50 disabled:cursor-not-allowed rounded-lg text-white text-sm font-medium transition-colors"
        >
          Play
        </button>
        
        <button
          onClick={pause}
          disabled={!state.isPlaying}
          className="px-4 py-2 bg-yellow-600 hover:bg-yellow-700 disabled:opacity-50 disabled:cursor-not-allowed rounded-lg text-white text-sm font-medium transition-colors"
        >
          Pause
        </button>
        
        <button
          onClick={stop}
          disabled={state.isStopped}
          className="px-4 py-2 bg-red-600 hover:bg-red-700 disabled:opacity-50 disabled:cursor-not-allowed rounded-lg text-white text-sm font-medium transition-colors"
        >
          Stop
        </button>
      </div>
    </div>
  );
};

export default PlaybackControls;
