import React, { createContext, useContext, useReducer, useEffect } from 'react';
import { audioApi } from '../../services/audioApi';

// Initial state
const initialState = {
  tracks: [],
  playlists: [],
  currentTrack: null,
  playbackStatus: {
    isPlaying: false,
    isPaused: false,
    currentTime: 0,
    duration: 0,
    volume: 70,
    isMuted: false,
    currentPlaylist: null,
    queue: [],
    repeat: false,
    shuffle: false
  },
  stats: null,
  loading: false,
  error: null
};

// Action types
const ACTIONS = {
  SET_LOADING: 'SET_LOADING',
  SET_ERROR: 'SET_ERROR',
  SET_TRACKS: 'SET_TRACKS',
  SET_PLAYLISTS: 'SET_PLAYLISTS',
  SET_CURRENT_TRACK: 'SET_CURRENT_TRACK',
  SET_PLAYBACK_STATUS: 'SET_PLAYBACK_STATUS',
  SET_STATS: 'SET_STATS',
  UPDATE_PLAYBACK_STATUS: 'UPDATE_PLAYBACK_STATUS',
  ADD_TRACK: 'ADD_TRACK',
  REMOVE_TRACK: 'REMOVE_TRACK',
  UPDATE_PLAYLIST: 'UPDATE_PLAYLIST'
};

// Reducer
const audioReducer = (state, action) => {
  switch (action.type) {
    case ACTIONS.SET_LOADING:
      return { ...state, loading: action.payload };
    case ACTIONS.SET_ERROR:
      return { ...state, error: action.payload };
    case ACTIONS.SET_TRACKS:
      return { ...state, tracks: action.payload };
    case ACTIONS.SET_PLAYLISTS:
      return { ...state, playlists: action.payload };
    case ACTIONS.SET_CURRENT_TRACK:
      return { ...state, currentTrack: action.payload };
    case ACTIONS.SET_PLAYBACK_STATUS:
      return { ...state, playbackStatus: { ...state.playbackStatus, ...action.payload } };
    case ACTIONS.SET_STATS:
      return { ...state, stats: action.payload };
    case ACTIONS.UPDATE_PLAYBACK_STATUS:
      return { 
        ...state, 
        playbackStatus: { ...state.playbackStatus, ...action.payload }
      };
    case ACTIONS.ADD_TRACK:
      return { ...state, tracks: [...state.tracks, action.payload] };
    case ACTIONS.REMOVE_TRACK:
      return { 
        ...state, 
        tracks: state.tracks.filter(track => track.id !== action.payload)
      };
    case ACTIONS.UPDATE_PLAYLIST:
      return {
        ...state,
        playlists: state.playlists.map(playlist =>
          playlist.id === action.payload.id ? action.payload : playlist
        )
      };
    default:
      return state;
  }
};

// Create context
const AudioContext = createContext();

// Provider component
export const AudioProvider = ({ children }) => {
  const [state, dispatch] = useReducer(audioReducer, initialState);

  // Load initial data
  useEffect(() => {
    loadTracks();
    loadPlaylists();
    loadStats();
  }, []);

  // Poll playback status
  useEffect(() => {
    const interval = setInterval(() => {
      if (state.playbackStatus.isPlaying) {
        updatePlaybackStatus();
      }
    }, 1000);

    return () => clearInterval(interval);
  }, [state.playbackStatus.isPlaying]);

  // Action functions
  const setLoading = (loading) => {
    dispatch({ type: ACTIONS.SET_LOADING, payload: loading });
  };

  const setError = (error) => {
    dispatch({ type: ACTIONS.SET_ERROR, payload: error });
  };

  const loadTracks = async () => {
    try {
      setLoading(true);
      const response = await audioApi.getTracks();
      dispatch({ type: ACTIONS.SET_TRACKS, payload: response.data });
    } catch (error) {
      setError(error.message);
    } finally {
      setLoading(false);
    }
  };

  const loadPlaylists = async () => {
    try {
      const response = await audioApi.getPlaylists();
      dispatch({ type: ACTIONS.SET_PLAYLISTS, payload: response.data });
    } catch (error) {
      setError(error.message);
    }
  };

  const loadStats = async () => {
    try {
      const response = await audioApi.getStats();
      dispatch({ type: ACTIONS.SET_STATS, payload: response.data });
    } catch (error) {
      setError(error.message);
    }
  };

  const play = async (trackId = null) => {
    try {
      if (trackId) {
        const response = await audioApi.playTrack(trackId);
        if (response.data.success) {
          const track = state.tracks.find(t => t.id === trackId);
          dispatch({ type: ACTIONS.SET_CURRENT_TRACK, payload: track });
          dispatch({ 
            type: ACTIONS.SET_PLAYBACK_STATUS, 
            payload: { isPlaying: true, isPaused: false }
          });
        }
      } else {
        const response = await audioApi.resumePlayback();
        if (response.data.success) {
          dispatch({ 
            type: ACTIONS.SET_PLAYBACK_STATUS, 
            payload: { isPlaying: true, isPaused: false }
          });
        }
      }
    } catch (error) {
      setError(error.message);
    }
  };

  const pause = async () => {
    try {
      const response = await audioApi.pausePlayback();
      if (response.data.success) {
        dispatch({ 
          type: ACTIONS.SET_PLAYBACK_STATUS, 
          payload: { isPlaying: false, isPaused: true }
        });
      }
    } catch (error) {
      setError(error.message);
    }
  };

  const stop = async () => {
    try {
      const response = await audioApi.stopPlayback();
      if (response.data.success) {
        dispatch({ 
          type: ACTIONS.SET_PLAYBACK_STATUS, 
          payload: { 
            isPlaying: false, 
            isPaused: false, 
            currentTime: 0 
          }
        });
      }
    } catch (error) {
      setError(error.message);
    }
  };

  const next = async () => {
    try {
      const response = await audioApi.nextTrack();
      if (response.data.success) {
        updatePlaybackStatus();
      }
    } catch (error) {
      setError(error.message);
    }
  };

  const previous = async () => {
    try {
      const response = await audioApi.previousTrack();
      if (response.data.success) {
        updatePlaybackStatus();
      }
    } catch (error) {
      setError(error.message);
    }
  };

  const setVolume = async (volume) => {
    try {
      const response = await audioApi.setVolume(volume);
      if (response.data.success) {
        dispatch({ 
          type: ACTIONS.SET_PLAYBACK_STATUS, 
          payload: { volume }
        });
      }
    } catch (error) {
      setError(error.message);
    }
  };

  const toggleMute = async () => {
    try {
      const response = await audioApi.toggleMute();
      if (response.data.success) {
        dispatch({ 
          type: ACTIONS.SET_PLAYBACK_STATUS, 
          payload: { isMuted: !state.playbackStatus.isMuted }
        });
      }
    } catch (error) {
      setError(error.message);
    }
  };

  const loadPlaylist = async (playlistId) => {
    try {
      const response = await audioApi.loadPlaylist(playlistId);
      if (response.data.success) {
        const playlist = state.playlists.find(p => p.id === playlistId);
        dispatch({ 
          type: ACTIONS.SET_PLAYBACK_STATUS, 
          payload: { 
            currentPlaylist: playlist,
            queue: playlist.tracks || []
          }
        });
      }
    } catch (error) {
      setError(error.message);
    }
  };

  const uploadSong = async (file, category = 'general') => {
    try {
      setLoading(true);
      const formData = new FormData();
      formData.append('file', file);
      formData.append('category', category);
      
      const response = await audioApi.uploadSong(formData);
      if (response.data.success) {
        await loadTracks();
        return response.data;
      }
    } catch (error) {
      setError(error.message);
      throw error;
    } finally {
      setLoading(false);
    }
  };

  const searchTracks = async (filters = {}) => {
    try {
      const response = await audioApi.searchTracks(filters);
      return response.data;
    } catch (error) {
      setError(error.message);
      throw error;
    }
  };

  const getRandomTrack = async (filters = {}) => {
    try {
      const response = await audioApi.getRandomTrack(filters);
      return response.data;
    } catch (error) {
      setError(error.message);
      throw error;
    }
  };

  const getTracksByCategory = async (category, limit = 10) => {
    try {
      const response = await audioApi.getTracksByCategory(category, limit);
      return response.data;
    } catch (error) {
      setError(error.message);
      throw error;
    }
  };

  const refreshLibrary = async () => {
    try {
      setLoading(true);
      const response = await audioApi.scanLibrary();
      if (response.data.success) {
        await loadTracks();
        await loadPlaylists();
        await loadStats();
      }
    } catch (error) {
      setError(error.message);
    } finally {
      setLoading(false);
    }
  };

  const deleteTrack = async (trackId) => {
    try {
      const response = await audioApi.deleteTrack(trackId);
      if (response.data.success) {
        // Remove track from state
        dispatch({ type: ACTIONS.REMOVE_TRACK, payload: trackId });
        return response.data;
      }
    } catch (error) {
      setError(error.message);
      throw error;
    }
  };

  const updatePlaybackStatus = async () => {
    try {
      const response = await audioApi.getPlaybackStatus();
      if (response.data.success) {
        dispatch({ 
          type: ACTIONS.SET_PLAYBACK_STATUS, 
          payload: response.data.status
        });
      }
    } catch (error) {
      // Don't set error for status updates
    }
  };

  const value = {
    state,
    play,
    pause,
    stop,
    next,
    previous,
    setVolume,
    toggleMute,
    loadPlaylist,
    uploadSong,
    searchTracks,
    getRandomTrack,
    getTracksByCategory,
    refreshLibrary,
    deleteTrack,
    loadTracks,
    loadPlaylists,
    loadStats
  };

  return (
    <AudioContext.Provider value={value}>
      {children}
    </AudioContext.Provider>
  );
};

// Hook to use the context
export const useAudio = () => {
  const context = useContext(AudioContext);
  if (!context) {
    throw new Error('useAudio must be used within an AudioProvider');
  }
  return context;
};
