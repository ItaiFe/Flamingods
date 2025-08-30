import React, { useState } from 'react';
import { useAudio } from './AudioContext';
import { 
  Play, 
  Pause, 
  SkipBack, 
  SkipForward, 
  Volume2, 
  VolumeX,
  RotateCcw,
  Shuffle,
  Repeat,
  Music
} from 'lucide-react';

const PlaybackControls = () => {
  const { 
    state, 
    play, 
    pause, 
    stop, 
    next, 
    previous, 
    setVolume, 
    toggleMute 
  } = useAudio();

  const [volumeSlider, setVolumeSlider] = useState(state.playbackStatus.volume);

  const handleVolumeChange = (newVolume) => {
    setVolumeSlider(newVolume);
    setVolume(newVolume);
  };

  const handlePlayPause = () => {
    if (state.playbackStatus.isPlaying) {
      pause();
    } else {
      play();
    }
  };

  const formatTime = (seconds) => {
    if (!seconds || seconds < 0) return '0:00';
    
    const minutes = Math.floor(seconds / 60);
    const remainingSeconds = Math.floor(seconds % 60);
    return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
  };

  const currentTrack = state.currentTrack;
  const playbackStatus = state.playbackStatus;

  return (
    <div className="space-y-4">
      {/* Track Info */}
      {currentTrack && (
        <div className="flex items-center space-x-4 p-4 bg-gray-700 rounded-lg">
          <div className="w-16 h-16 bg-primary-600 rounded-lg flex items-center justify-center">
            <Music className="h-8 w-8 text-white" />
          </div>
          <div className="flex-1">
            <h3 className="text-lg font-semibold text-white">{currentTrack.title}</h3>
            <p className="text-gray-400">{currentTrack.artist}</p>
            {currentTrack.album && (
              <p className="text-sm text-gray-500">{currentTrack.album}</p>
            )}
          </div>
          <div className="text-right">
            <div className="text-sm text-gray-400">
              {formatTime(playbackStatus.currentTime)} / {formatTime(playbackStatus.duration)}
            </div>
            <div className="text-xs text-gray-500">
              {currentTrack.genre} • {currentTrack.format}
            </div>
          </div>
        </div>
      )}

      {/* Playback Controls */}
      <div className="flex items-center justify-center space-x-4">
        <button
          onClick={previous}
          disabled={!currentTrack}
          className="btn-secondary p-3 disabled:opacity-50 disabled:cursor-not-allowed"
        >
          <SkipBack className="h-5 w-5" />
        </button>

        <button
          onClick={handlePlayPause}
          disabled={!currentTrack}
          className="btn-primary p-4 disabled:opacity-50 disabled:cursor-not-allowed"
        >
          {playbackStatus.isPlaying ? (
            <Pause className="h-6 w-6" />
          ) : (
            <Play className="h-6 w-6" />
          )}
        </button>

        <button
          onClick={stop}
          disabled={!currentTrack}
          className="btn-secondary p-3 disabled:opacity-50 disabled:cursor-not-allowed"
        >
          <RotateCcw className="h-5 w-5" />
        </button>

        <button
          onClick={next}
          disabled={!currentTrack}
          className="btn-secondary p-3 disabled:opacity-50 disabled:cursor-not-allowed"
        >
          <SkipForward className="h-5 w-5" />
        </button>
      </div>

      {/* Progress Bar */}
      {currentTrack && (
        <div className="space-y-2">
          <div className="w-full bg-gray-700 rounded-full h-2">
            <div 
              className="bg-primary-500 h-2 rounded-full transition-all duration-300"
              style={{ 
                width: `${playbackStatus.duration > 0 ? 
                  (playbackStatus.currentTime / playbackStatus.duration) * 100 : 0
                }%` 
              }}
            ></div>
          </div>
          <div className="flex justify-between text-sm text-gray-400">
            <span>{formatTime(playbackStatus.currentTime)}</span>
            <span>{formatTime(playbackStatus.duration)}</span>
          </div>
        </div>
      )}

      {/* Volume and Additional Controls */}
      <div className="flex items-center justify-between">
        <div className="flex items-center space-x-4">
          <button
            onClick={toggleMute}
            className="btn-secondary p-2"
          >
            {playbackStatus.isMuted ? (
              <VolumeX className="h-5 w-5" />
            ) : (
              <Volume2 className="h-5 w-5" />
            )}
          </button>
          
          <div className="flex items-center space-x-2">
            <VolumeX className="h-4 w-4 text-gray-400" />
            <input
              type="range"
              min="0"
              max="100"
              value={volumeSlider}
              onChange={(e) => handleVolumeChange(parseInt(e.target.value))}
              className="w-24 h-2 bg-gray-700 rounded-lg appearance-none cursor-pointer slider"
            />
            <Volume2 className="h-4 w-4 text-gray-400" />
          </div>
        </div>

        <div className="flex items-center space-x-2">
          <button
            onClick={() => {}} // TODO: Implement shuffle
            className={`p-2 rounded-lg transition-colors ${
              playbackStatus.shuffle 
                ? 'bg-primary-600 text-white' 
                : 'bg-gray-700 text-gray-400 hover:bg-gray-600'
            }`}
          >
            <Shuffle className="h-4 w-4" />
          </button>
          
          <button
            onClick={() => {}} // TODO: Implement repeat
            className={`p-2 rounded-lg transition-colors ${
              playbackStatus.repeat 
                ? 'bg-primary-600 text-white' 
                : 'bg-gray-700 text-gray-400 hover:bg-gray-600'
            }`}
          >
            <Repeat className="h-4 w-4" />
          </button>
        </div>
      </div>

      {/* Playlist Info */}
      {playbackStatus.currentPlaylist && (
        <div className="p-3 bg-gray-700 rounded-lg">
          <div className="text-sm text-gray-400">Now Playing from:</div>
          <div className="text-white font-medium">{playbackStatus.currentPlaylist.name}</div>
          <div className="text-xs text-gray-500">
            {playbackStatus.queue.length} tracks in queue
          </div>
        </div>
      )}
    </div>
  );
};

export default PlaybackControls;
