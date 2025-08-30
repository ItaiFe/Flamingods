import React from 'react';
import { useAudio } from '../contexts/AudioContext';
import { Clock, Play, Music } from 'lucide-react';

const RecentTracks: React.FC = () => {
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

  const formatDate = (dateString: string): string => {
    const date = new Date(dateString);
    const now = new Date();
    const diffTime = Math.abs(now.getTime() - date.getTime());
    const diffDays = Math.ceil(diffTime / (1000 * 60 * 60 * 24));
    
    if (diffDays === 1) return 'Today';
    if (diffDays === 2) return 'Yesterday';
    if (diffDays <= 7) return `${diffDays - 1} days ago`;
    
    return date.toLocaleDateString();
  };

  // Get recent tracks (last 5 played)
  const recentTracks = state.tracks
    .filter(track => track.last_played)
    .sort((a, b) => new Date(b.last_played!).getTime() - new Date(a.last_played!).getTime())
    .slice(0, 5);

  if (recentTracks.length === 0) {
    return (
      <div className="text-center py-8 text-gray-400">
        <Clock className="h-12 w-12 mx-auto mb-4 opacity-50" />
        <p>No tracks played yet</p>
        <p className="text-sm">Start playing music to see recent tracks</p>
      </div>
    );
  }

  return (
    <div className="space-y-3">
      {recentTracks.map((track) => (
        <div
          key={track.id}
          className="flex items-center space-x-3 p-3 bg-gray-700 rounded-lg hover:bg-gray-600 transition-colors cursor-pointer"
          onClick={() => handlePlayTrack(track.id)}
        >
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

          {/* Track Details */}
          <div className="flex-shrink-0 text-right">
            <div className="text-xs text-gray-400">
              {formatDuration(track.duration_seconds || 0)}
            </div>
            <div className="text-xs text-gray-500">
              {formatDate(track.last_played!)}
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
          View all recent tracks →
        </button>
      </div>
    </div>
  );
};

export default RecentTracks;
