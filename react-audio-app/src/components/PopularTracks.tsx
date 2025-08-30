import React from 'react';
import { useAudio } from '../contexts/AudioContext';
import { TrendingUp, Play, Music, BarChart3 } from 'lucide-react';

const PopularTracks: React.FC = () => {
  const { state, play } = useAudio();

  const handlePlayTrack = (trackId: string) => {
    play({ track_id: trackId });
  };

  const formatDuration = (seconds: number): string => {
    if (!seconds || seconds < 0) return '0:00';
    
    const minutes = Math.floor(seconds / 60);
    const remainingSeconds = Math.floor(seconds % 60);
    return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
  };

  // Get popular tracks (most played, top 5)
  const popularTracks = state.tracks
    .filter(track => track.play_count > 0)
    .sort((a, b) => b.play_count - a.play_count)
    .slice(0, 5);

  if (popularTracks.length === 0) {
    return (
      <div className="text-center py-8 text-gray-400">
        <TrendingUp className="h-12 w-12 mx-auto mb-4 opacity-50" />
        <p>No popular tracks yet</p>
        <p className="text-sm">Start playing music to see popular tracks</p>
      </div>
    );
  }

  return (
    <div className="space-y-3">
      {popularTracks.map((track, index) => (
        <div
          key={track.id}
          className="flex items-center space-x-3 p-3 bg-gray-700 rounded-lg hover:bg-gray-600 transition-colors cursor-pointer"
          onClick={() => handlePlayTrack(track.id)}
        >
          {/* Rank */}
          <div className="flex-shrink-0">
            <div className={`w-8 h-8 rounded-full flex items-center justify-center text-sm font-bold ${
              index === 0 ? 'bg-yellow-500 text-black' :
              index === 1 ? 'bg-gray-400 text-black' :
              index === 2 ? 'bg-amber-600 text-white' :
              'bg-gray-600 text-white'
            }`}>
              {index + 1}
            </div>
          </div>

          {/* Track Icon */}
          <div className="flex-shrink-0">
            <div className="w-10 h-10 bg-primary-600 rounded-lg flex items-center justify-center">
              <Music className="h-5 w-5 text-white" />
            </div>
          </div>

          {/* Track Info */}
          <div className="flex-1 min-w-0">
            <div className="text-sm font-medium text-white truncate">
              {track.title}
            </div>
            <div className="text-xs text-gray-400 truncate">
              {track.artist || 'Unknown Artist'}
            </div>
            {track.album && (
              <div className="text-xs text-gray-500 truncate">
                {track.album}
              </div>
            )}
          </div>

          {/* Play Count */}
          <div className="flex-shrink-0 text-center">
            <div className="flex items-center space-x-1">
              <BarChart3 className="h-3 w-3 text-green-400" />
              <span className="text-xs text-green-400 font-medium">
                {track.play_count}
              </span>
            </div>
            <div className="text-xs text-gray-500">
              plays
            </div>
          </div>

          {/* Track Duration */}
          <div className="flex-shrink-0 text-right">
            <div className="text-xs text-gray-400">
              {formatDuration(track.duration_seconds || 0)}
            </div>
          </div>

          {/* Play Button */}
          <button
            onClick={(e) => {
              e.stopPropagation();
              handlePlayTrack(track.id);
            }}
            className="flex-shrink-0 p-2 bg-primary-600 hover:bg-primary-700 rounded-lg transition-colors"
          >
            <Play className="h-4 w-4 text-white" />
          </button>
        </div>
      ))}

      {/* View All Link */}
      <div className="text-center pt-2">
        <button className="text-sm text-primary-400 hover:text-primary-300 transition-colors">
          View all popular tracks →
        </button>
      </div>
    </div>
  );
};

export default PopularTracks;
