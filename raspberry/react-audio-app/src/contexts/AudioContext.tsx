import React, { createContext, useContext, useReducer, useEffect, ReactNode } from 'react';
import {
  TrackInfo,
  PlaylistInfo,
  PlaybackStatus,
  AudioStats,
  SearchFilters,
  TrackSelectionRequest
} from '../types/audio';
import audioApi from '../services/audioApi';

// State interface
interface AudioState {
  // Playback state
  playbackStatus: PlaybackStatus | null;
  isPlaying: boolean;
  isPaused: boolean;
  isStopped: boolean;
  
  // Library state
  tracks: TrackInfo[];
  playlists: PlaylistInfo[];
  currentTrack: TrackInfo | null;
  currentPlaylist: PlaylistInfo | null;
  
  // UI state
  isLoading: boolean;
  error: string | null;
  
  // Statistics
  stats: AudioStats | null;
  
  // Search and filters
  searchResults: TrackInfo[];
  searchFilters: SearchFilters;
  
  // Upload state
  isUploading: boolean;
  uploadProgress: number;
}

// Action types
type AudioAction =
  | { type: 'SET_LOADING'; payload: boolean }
  | { type: 'SET_ERROR'; payload: string | null }
  | { type: 'SET_PLAYBACK_STATUS'; payload: PlaybackStatus }
  | { type: 'SET_TRACKS'; payload: TrackInfo[] }
  | { type: 'SET_PLAYLISTS'; payload: PlaylistInfo[] }
  | { type: 'SET_CURRENT_TRACK'; payload: TrackInfo | null }
  | { type: 'SET_CURRENT_PLAYLIST'; payload: PlaylistInfo | null }
  | { type: 'SET_STATS'; payload: AudioStats }
  | { type: 'SET_SEARCH_RESULTS'; payload: TrackInfo[] }
  | { type: 'SET_SEARCH_FILTERS'; payload: SearchFilters }
  | { type: 'SET_UPLOADING'; payload: boolean }
  | { type: 'SET_UPLOAD_PROGRESS'; payload: number }
  | { type: 'ADD_TRACK'; payload: TrackInfo }
  | { type: 'REMOVE_TRACK'; payload: string }
  | { type: 'UPDATE_TRACK'; payload: TrackInfo };

// Initial state
const initialState: AudioState = {
  playbackStatus: null,
  isPlaying: false,
  isPaused: false,
  isStopped: true,
  tracks: [],
  playlists: [],
  currentTrack: null,
  currentPlaylist: null,
  isLoading: false,
  error: null,
  stats: null,
  searchResults: [],
  searchFilters: {},
  isUploading: false,
  uploadProgress: 0,
};

// Reducer function
function audioReducer(state: AudioState, action: AudioAction): AudioState {
  switch (action.type) {
    case 'SET_LOADING':
      return { ...state, isLoading: action.payload };
    
    case 'SET_ERROR':
      return { ...state, error: action.payload };
    
    case 'SET_PLAYBACK_STATUS':
      const status = action.payload;
      return {
        ...state,
        playbackStatus: status,
        isPlaying: status.state === 'playing',
        isPaused: status.state === 'paused',
        isStopped: status.state === 'stopped',
        currentTrack: status.current_track || null,
        currentPlaylist: status.current_playlist || null,
      };
    
    case 'SET_TRACKS':
      return { ...state, tracks: action.payload };
    
    case 'SET_PLAYLISTS':
      return { ...state, playlists: action.payload };
    
    case 'SET_CURRENT_TRACK':
      return { ...state, currentTrack: action.payload };
    
    case 'SET_CURRENT_PLAYLIST':
      return { ...state, currentPlaylist: action.payload };
    
    case 'SET_STATS':
      return { ...state, stats: action.payload };
    
    case 'SET_SEARCH_RESULTS':
      return { ...state, searchResults: action.payload };
    
    case 'SET_SEARCH_FILTERS':
      return { ...state, searchFilters: action.payload };
    
    case 'SET_UPLOADING':
      return { ...state, isUploading: action.payload };
    
    case 'SET_UPLOAD_PROGRESS':
      return { ...state, uploadProgress: action.payload };
    
    case 'ADD_TRACK':
      return { ...state, tracks: [...state.tracks, action.payload] };
    
    case 'REMOVE_TRACK':
      return { ...state, tracks: state.tracks.filter(track => track.id !== action.payload) };
    
    case 'UPDATE_TRACK':
      return {
        ...state,
        tracks: state.tracks.map(track =>
          track.id === action.payload.id ? action.payload : track
        ),
      };
    
    default:
      return state;
  }
}

// Context interface
interface AudioContextType {
  state: AudioState;
  dispatch: React.Dispatch<AudioAction>;
  
  // Playback actions
  play: (control?: any) => Promise<void>;
  pause: () => Promise<void>;
  stop: () => Promise<void>;
  next: () => Promise<void>;
  previous: () => Promise<void>;
  setVolume: (volume: number) => Promise<void>;
  toggleMute: () => Promise<void>;
  
  // Library actions
  loadTracks: () => Promise<void>;
  loadPlaylists: () => Promise<void>;
  loadStats: () => Promise<void>;
  
  // Search actions
  searchTracks: (filters: SearchFilters) => Promise<void>;
  getRandomTrack: (filters?: Partial<SearchFilters>) => Promise<TrackInfo | null>;
  getPopularTracks: (limit?: number) => Promise<TrackInfo[]>;
  getRecentTracks: (limit?: number) => Promise<TrackInfo[]>;
  
  // Upload actions
  uploadSong: (file: File, category?: string) => Promise<void>;
  uploadMultipleSongs: (files: File[], category?: string) => Promise<void>;
  deleteSong: (trackId: string) => Promise<void>;
  
  // Playlist actions
  loadPlaylist: (playlistId: string) => Promise<void>;
  playPlaylist: (playlistId: string) => Promise<void>;
  
  // Selection actions
  selectAndPlayTrack: (request: TrackSelectionRequest) => Promise<void>;
  
  // Utility actions
  refreshLibrary: () => Promise<void>;
  clearError: () => void;
}

// Create context
const AudioContext = createContext<AudioContextType | undefined>(undefined);

// Provider component
export const AudioProvider: React.FC<{ children: ReactNode }> = ({ children }) => {
  const [state, dispatch] = useReducer(audioReducer, initialState);

  // Load initial data
  useEffect(() => {
    loadInitialData();
  }, []);

  // Set up status polling
  useEffect(() => {
    const interval = setInterval(pollPlaybackStatus, 1000);
    return () => clearInterval(interval);
  }, []);

  const loadInitialData = async () => {
    try {
      dispatch({ type: 'SET_LOADING', payload: true });
      await Promise.all([
        loadTracks(),
        loadPlaylists(),
        loadStats(),
        pollPlaybackStatus(),
      ]);
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to load initial data' });
    } finally {
      dispatch({ type: 'SET_LOADING', payload: false });
    }
  };

  const pollPlaybackStatus = async () => {
    try {
      const status = await audioApi.getStatus();
      dispatch({ type: 'SET_PLAYBACK_STATUS', payload: status });
    } catch (error) {
      // Silently fail for status polling
    }
  };

  const play = async (control?: any) => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      await audioApi.play(control);
      await pollPlaybackStatus();
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to play audio' });
    }
  };

  const pause = async () => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      await audioApi.pause();
      await pollPlaybackStatus();
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to pause audio' });
    }
  };

  const stop = async () => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      await audioApi.stop();
      await pollPlaybackStatus();
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to stop audio' });
    }
  };

  const next = async () => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      await audioApi.next();
      await pollPlaybackStatus();
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to play next track' });
    }
  };

  const previous = async () => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      await audioApi.previous();
      await pollPlaybackStatus();
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to play previous track' });
    }
  };

  const setVolume = async (volume: number) => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      await audioApi.setVolume(volume);
      await pollPlaybackStatus();
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to set volume' });
    }
  };

  const toggleMute = async () => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      await audioApi.toggleMute();
      await pollPlaybackStatus();
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to toggle mute' });
    }
  };

  const loadTracks = async () => {
    try {
      const tracks = await audioApi.getTracks();
      dispatch({ type: 'SET_TRACKS', payload: tracks });
    } catch (error) {
      throw error;
    }
  };

  const loadPlaylists = async () => {
    try {
      const playlists = await audioApi.getPlaylists();
      dispatch({ type: 'SET_PLAYLISTS', payload: playlists });
    } catch (error) {
      throw error;
    }
  };

  const loadStats = async () => {
    try {
      const stats = await audioApi.getStats();
      dispatch({ type: 'SET_STATS', payload: stats });
    } catch (error) {
      throw error;
    }
  };

  const searchTracks = async (filters: SearchFilters) => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      const results = await audioApi.searchTracks(filters);
      dispatch({ type: 'SET_SEARCH_RESULTS', payload: results });
      dispatch({ type: 'SET_SEARCH_FILTERS', payload: filters });
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Search failed' });
    }
  };

  const getRandomTrack = async (filters?: Partial<SearchFilters>): Promise<TrackInfo | null> => {
    try {
      const track = await audioApi.getRandomTrack(filters);
      return track;
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to get random track' });
      return null;
    }
  };

  const getPopularTracks = async (limit = 10): Promise<TrackInfo[]> => {
    try {
      return await audioApi.getPopularTracks(limit);
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to get popular tracks' });
      return [];
    }
  };

  const getRecentTracks = async (limit = 10): Promise<TrackInfo[]> => {
    try {
      return await audioApi.getRecentTracks(limit);
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to get recent tracks' });
      return [];
    }
  };

  const uploadSong = async (file: File, category?: string) => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      dispatch({ type: 'SET_UPLOADING', payload: true });
      dispatch({ type: 'SET_UPLOAD_PROGRESS', payload: 0 });
      
      await audioApi.uploadSong(file, category);
      
      // Refresh library after upload
      await loadTracks();
      await loadStats();
      
      dispatch({ type: 'SET_UPLOADING', payload: false });
      dispatch({ type: 'SET_UPLOAD_PROGRESS', payload: 100 });
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Upload failed' });
      dispatch({ type: 'SET_UPLOADING', payload: false });
    }
  };

  const uploadMultipleSongs = async (files: File[], category?: string) => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      dispatch({ type: 'SET_UPLOADING', payload: true });
      dispatch({ type: 'SET_UPLOAD_PROGRESS', payload: 0 });
      
      await audioApi.uploadMultipleSongs(files, category);
      
      // Refresh library after upload
      await loadTracks();
      await loadStats();
      
      dispatch({ type: 'SET_UPLOADING', payload: false });
      dispatch({ type: 'SET_UPLOAD_PROGRESS', payload: 100 });
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Batch upload failed' });
      dispatch({ type: 'SET_UPLOADING', payload: false });
    }
  };

  const deleteSong = async (trackId: string) => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      await audioApi.deleteSong(trackId);
      
      // Remove from local state
      dispatch({ type: 'REMOVE_TRACK', payload: trackId });
      
      // Refresh stats
      await loadStats();
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to delete song' });
    }
  };

  const loadPlaylist = async (playlistId: string) => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      await audioApi.loadPlaylist(playlistId);
      await pollPlaybackStatus();
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to load playlist' });
    }
  };

  const playPlaylist = async (playlistId: string) => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      await audioApi.playPlaylist(playlistId);
      await pollPlaybackStatus();
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to play playlist' });
    }
  };

  const selectAndPlayTrack = async (request: TrackSelectionRequest) => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      await audioApi.selectAndPlayTrack(request);
      await pollPlaybackStatus();
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Track selection failed' });
    }
  };

  const refreshLibrary = async () => {
    try {
      dispatch({ type: 'SET_ERROR', payload: null });
      await audioApi.scanLibrary(true);
      await loadTracks();
      await loadPlaylists();
      await loadStats();
    } catch (error) {
      dispatch({ type: 'SET_ERROR', payload: 'Failed to refresh library' });
    }
  };

  const clearError = () => {
    dispatch({ type: 'SET_ERROR', payload: null });
  };

  const contextValue: AudioContextType = {
    state,
    dispatch,
    play,
    pause,
    stop,
    next,
    previous,
    setVolume,
    toggleMute,
    loadTracks,
    loadPlaylists,
    loadStats,
    searchTracks,
    getRandomTrack,
    getPopularTracks,
    getRecentTracks,
    uploadSong,
    uploadMultipleSongs,
    deleteSong,
    loadPlaylist,
    playPlaylist,
    selectAndPlayTrack,
    refreshLibrary,
    clearError,
  };

  return (
    <AudioContext.Provider value={contextValue}>
      {children}
    </AudioContext.Provider>
  );
};

// Custom hook to use the audio context
export const useAudio = (): AudioContextType => {
  const context = useContext(AudioContext);
  if (context === undefined) {
    throw new Error('useAudio must be used within an AudioProvider');
  }
  return context;
};
