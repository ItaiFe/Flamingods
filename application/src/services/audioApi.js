import axios from 'axios';

// Create axios instance with base configuration
const api = axios.create({
  baseURL: process.env.REACT_APP_API_URL || 'http://192.168.1.203:8000',
  timeout: 10000,
  headers: {
    'Content-Type': 'application/json',
  },
});

// Audio API endpoints
export const audioApi = {
  // Playback control
  playTrack: (trackId) => api.post(`/audio/play/${trackId}`),
  pausePlayback: () => api.post('/audio/pause'),
  resumePlayback: () => api.post('/audio/resume'),
  stopPlayback: () => api.post('/audio/stop'),
  nextTrack: () => api.post('/audio/next'),
  previousTrack: () => api.post('/audio/previous'),
  
  // Volume control
  setVolume: (volume) => api.post(`/audio/volume/${volume}`),
  toggleMute: () => api.post('/audio/mute'),
  
  // Status and info
  getPlaybackStatus: () => api.get('/audio/status'),
  getTracks: () => api.get('/audio/tracks'),
  getTrack: (trackId) => api.get(`/audio/tracks/${trackId}`),
  getPlaylists: () => api.get('/audio/playlists'),
  getPlaylist: (playlistId) => api.get(`/audio/playlists/${playlistId}`),
  
  // Library management
  scanLibrary: () => api.post('/audio/scan'),
  getStats: () => api.get('/audio/stats'),
  
  // Playlist management
  loadPlaylist: (playlistId) => api.post(`/audio/playlists/${playlistId}/load`),
  playPlaylist: (playlistId) => api.post(`/audio/playlists/${playlistId}/play`),
  
  // Generic control
  control: (action, params = {}) => api.post('/audio/control', { action, ...params }),
  
  // Health check
  health: () => api.get('/audio/health'),
  
  // File upload
  uploadSong: (formData) => api.post('/audio/upload', formData, {
    headers: {
      'Content-Type': 'multipart/form-data',
    },
  }),
  uploadBatch: (formData) => api.post('/audio/upload/batch', formData, {
    headers: {
      'Content-Type': 'multipart/form-data',
    },
  }),
  
  // File deletion
  deleteTrack: (trackId) => api.delete(`/audio/tracks/${trackId}`),
  
  // Scan uploaded files
  scanUploaded: () => api.post('/audio/scan/uploaded'),
  
  // Track search and selection
  searchTracks: (filters = {}) => api.get('/audio/tracks/search', { params: filters }),
  getRandomTrack: (filters = {}) => api.get('/audio/tracks/random', { params: filters }),
  getPopularTracks: (limit = 10) => api.get('/audio/tracks/popular', { params: { limit } }),
  getRecentTracks: (limit = 10) => api.get('/audio/tracks/recent', { params: { limit } }),
  getTracksByCategory: (category, limit = 10) => api.get(`/audio/tracks/by-category/${category}`, { params: { limit } }),
  getTracksBySelection: (selectionType, params = {}) => api.get(`/audio/tracks/selection/${selectionType}`, { params }),
  selectTrack: (criteria) => api.post('/audio/tracks/select', criteria),
};

// Request interceptor for logging
api.interceptors.request.use(
  (config) => {
    console.log(`API Request: ${config.method?.toUpperCase()} ${config.url}`);
    return config;
  },
  (error) => {
    console.error('API Request Error:', error);
    return Promise.reject(error);
  }
);

// Response interceptor for error handling
api.interceptors.response.use(
  (response) => {
    return response;
  },
  (error) => {
    if (error.response) {
      // Server responded with error status
      console.error('API Error Response:', error.response.data);
      console.error('Status:', error.response.status);
    } else if (error.request) {
      // Request made but no response received
      console.error('API No Response:', error.request);
    } else {
      // Something else happened
      console.error('API Error:', error.message);
    }
    return Promise.reject(error);
  }
);

export default api;
