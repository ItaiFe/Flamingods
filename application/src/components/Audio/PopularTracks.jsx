import React from 'react';
import { useAudio } from './AudioContext';
import { TrendingUp, Play, Music, Trophy } from 'lucide-react';

const PopularTracks = () => {
  const { state, play } = useAudio();
  const { tracks } = state;

  // Get popular tracks (by play count)
  const popularTracks = tracks
    .filter(track => track.play_count && track.play_count > 0)
    .sort((a, b) => (b.play_count || 0) - (a.play_count || 0))
    .slice(0, 5);

  const formatTime = (seconds) => {
    if (!seconds || seconds < 0) return '0:00';
    const minutes = Math.floor(seconds / 60);
    const remainingSeconds = Math.floor(seconds % 60);
    return `${minutes}:${remainingSeconds.toString().padStart(2, '0')}`;
  };

  const getRankIcon = (rank) => {
    switch (rank) {
      case 1:
        return <Trophy className="h-4 w-4 text-yellow-400" />;
      case 2:
        return <Trophy className="h-4 w-4 text-gray-300" />;
      case 3:
        return <Trophy className="h-4 w-4 text-amber-600" />;
      default:
        return null;
    }
  };

  if (popularTracks.length === 0) {
    return (
      <div className="space-y-4">
        <div className="flex items-center space-x-2">
          <TrendingUp className="h-5 w-5 text-primary-400" />
          <h2 className="text-xl font-semibold text-white">Popular Tracks</h2>
        </div>
        <div className="text-center py-8 text-gray-400">
          <Music className="h-12 w-12 mx-auto mb-3 opacity-50" />
          <p>No popular tracks yet</p>
          <p className="text-sm">Start playing music to see popular tracks here</p>
        </div>
      </div>
    );
  }

  return (
    <div className="space-y-4">
      <div className="flex items-center space-x-2">
        <TrendingUp className="h-5 w-5 text-primary-400" />
        <h2 className="text-xl font-semibold text-white">Popular Tracks</h2>
      </div>
      
      <div className="space-y-2">
        {popularTracks.map((track, index) => (
          <div 
            key={track.id} 
            className="flex items-center space-x-3 p-3 bg-gray-700 rounded-lg hover:bg-gray-600 transition-colors cursor-pointer group"
            onClick={() => play(track.id)}
          >
            <div className="flex-shrink-0 w-8 h-8 bg-primary-600 rounded-full flex items-center justify-center text-white font-bold text-sm relative">
              {index + 1}
              {getRankIcon(index + 1) && (
                <div className="absolute -top-1 -right-1">
                  {getRankIcon(index + 1)}
                </div>
              )}
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
              <div className="text-gray-500 text-xs">
                {track.play_count || 0} plays
              </div>
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

export default PopularTracks;
