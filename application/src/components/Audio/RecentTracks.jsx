import React from 'react';
import { useAudio } from './AudioContext';
import { Clock, Play, Music } from 'lucide-react';

const RecentTracks = () => {
  const { state, play } = useAudio();
  const { tracks } = state;

  // Get recent tracks (last 5 played)
  const recentTracks = tracks
    .filter(track => track.last_played)
    .sort((a, b) => new Date(b.last_played) - new Date(a.last_played))
    .slice(0, 5);

  const formatTime = (seconds) => {
    if (!seconds || seconds < 0) return '0:00';
    const minutes = Math.floor(seconds / 60);
    const remainingSeconds = Math.floor(seconds % 60);
    return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
  };

  const formatDate = (dateString) => {
    if (!dateString) return 'Never';
    const date = new Date(dateString);
    const now = new Date();
    const diffTime = Math.abs(now - date);
    const diffDays = Math.ceil(diffTime / (1000 * 60 * 60 * 24));
    
    if (diffDays === 1) return 'Today';
    if (diffDays === 2) return 'Yesterday';
    if (diffDays <= 7) return `${diffDays - 1} days ago`;
    return date.toLocaleDateString();
  };

  if (recentTracks.length === 0) {
    return (
      <div className="space-y-4">
        <div className="flex items-center space-x-2">
          <Clock className="h-5 w-5 text-primary-400" />
          <h2 className="text-xl font-semibold text-white">Recent Tracks</h2>
        </div>
        <div className="text-center py-8 text-gray-400">
          <Music className="h-12 w-12 mx-auto mb-3 opacity-50" />
          <p>No tracks played yet</p>
          <p className="text-sm">Start playing music to see recent tracks here</p>
        </div>
      </div>
    );
  }

  return (
    <div className="space-y-4">
      <div className="flex items-center space-x-2">
        <Clock className="h-5 w-5 text-primary-400" />
        <h2 className="text-xl font-semibold text-white">Recent Tracks</h2>
      </div>
      
      <div className="space-y-2">
        {recentTracks.map((track, index) => (
          <div 
            key={track.id} 
            className="flex items-center space-x-3 p-3 bg-gray-700 rounded-lg hover:bg-gray-600 transition-colors cursor-pointer group"
            onClick={() => play(track.id)}
          >
            <div className="flex-shrink-0 w-8 h-8 bg-primary-600 rounded-full flex items-center justify-center text-white font-bold text-sm">
              {index + 1}
            </div>
            
            <div className="flex-1 min-w-0">
              <div className="text-white font-medium truncate">{track.title}</div>
              <div className="text-gray-400 text-sm truncate">{track.artist}</div>
              {track.album && (
                <div className="text-gray-500 text-xs truncate">{track.album}</div>
              )}
            </div>
            
            <div className="flex-shrink-0 text-right">
              <div className="text-gray-400 text-sm">{formatTime(track.duration)}</div>
              <div className="text-gray-500 text-xs">{formatDate(track.last_played)}</div>
            </div>
            
            <button 
              className="flex-shrink-0 p-2 bg-primary-600 rounded-full opacity-0 group-hover:opacity-100 transition-opacity"
              onClick={(e) => {
                e.stopPropagation();
                play(track.id);
              }}
            >
              <Play className="h-3 w-3 text-white" />
            </button>
          </div>
        ))}
      </div>
      
      {tracks.length > 5 && (
        <div className="text-center">
          <button className="text-primary-400 hover:text-primary-300 text-sm">
            View All Tracks
          </button>
        </div>
      )}
    </div>
  );
};

export default RecentTracks;
