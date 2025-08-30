import axios from 'axios';
import {
  TrackInfo,
  PlaylistInfo,
  PlaybackStatus,
  AudioResponse,
  AudioStats,
  SearchFilters,
  TrackSelectionRequest,
  UploadResponse,
  BatchUploadResponse
} from '../types/audio';

const API_BASE_URL = process.env.REACT_APP_API_URL || 'http://localhost:8000';

const api = axios.create({
  baseURL: API_BASE_URL,
  timeout: 10000,
});

// Request interceptor for logging
api.interceptors.request.use(
  (config) => {
    console.log(`API Request: ${config.method?.toUpperCase()} ${config.url}`);
    return config;
  },
  (error) => {
    return Promise.reject(error);
  }
);

// Response interceptor for error handling
api.interceptors.response.use(
  (response) => {
    return response;
  },
  (error) => {
    console.error('API Error:', error.response?.data || error.message);
    return Promise.reject(error);
  }
);

export const audioApi = {
  // Playback Control
  play: async (control?: any): Promise<AudioResponse> => {
    const response = await api.post('/audio/play', control || {});
    return response.data;
  },

  pause: async (): Promise<AudioResponse> => {
    const response = await api.post('/audio/pause');
    return response.data;
  },

  stop: async (): Promise<AudioResponse> => {
    const response = await api.post('/audio/stop');
    return response.data;
  },

  next: async (): Promise<AudioResponse> => {
    const response = await api.post('/audio/next');
    return response.data;
  },

  previous: async (): Promise<AudioResponse> => {
    const response = await api.post('/audio/previous');
    return response.data;
  },

  // Volume Control
  setVolume: async (volume: number): Promise<AudioResponse> => {
    const response = await api.post(`/audio/volume/${volume}`);
    return response.data;
  },

  toggleMute: async (): Promise<AudioResponse> => {
    const response = await api.post('/audio/mute');
    return response.data;
  },

  // Status and Information
  getStatus: async (): Promise<PlaybackStatus> => {
    const response = await api.get('/audio/status');
    return response.data;
  },

  getTracks: async (): Promise<TrackInfo[]> => {
    const response = await api.get('/audio/tracks');
    return response.data;
  },

  getTrack: async (trackId: string): Promise<TrackInfo> => {
    const response = await api.get(`/audio/tracks/${trackId}`);
    return response.data;
  },

  getPlaylists: async (): Promise<PlaylistInfo[]> => {
    const response = await api.get('/audio/playlists');
    return response.data;
  },

  getPlaylist: async (playlistId: string): Promise<PlaylistInfo> => {
    const response = await api.get(`/audio/playlists/${playlistId}`);
    return response.data;
  },

  // Library Management
  scanLibrary: async (forceRefresh = false): Promise<AudioResponse> => {
    const response = await api.post('/audio/scan', { force_refresh: forceRefresh });
    return response.data;
  },

  getStats: async (): Promise<AudioStats> => {
    const response = await api.get('/audio/stats');
    return response.data;
  },

  // Playlist Management
  loadPlaylist: async (playlistId: string): Promise<AudioResponse> => {
    const response = await api.post(`/audio/playlists/${playlistId}/load`);
    return response.data;
  },

  playPlaylist: async (playlistId: string): Promise<AudioResponse> => {
    const response = await api.post(`/audio/playlists/${playlistId}/play`);
    return response.data;
  },

  // Advanced Control
  control: async (control: any): Promise<AudioResponse> => {
    const response = await api.post('/audio/control', control);
    return response.data;
  },

  // Health Check
  getHealth: async (): Promise<any> => {
    const response = await api.get('/audio/health');
    return response.data;
  },

  // File Upload
  uploadSong: async (file: File, category?: string): Promise<AudioResponse> => {
    const formData = new FormData();
    formData.append('file', file);
    if (category) {
      formData.append('category', category);
    }

    const response = await api.post('/audio/upload', formData, {
      headers: {
        'Content-Type': 'multipart/form-data',
      },
    });
    return response.data;
  },

  uploadMultipleSongs: async (files: File[], category?: string): Promise<AudioResponse> => {
    const formData = new FormData();
    files.forEach((file) => {
      formData.append('files', file);
    });
    if (category) {
      formData.append('category', category);
    }

    const response = await api.post('/audio/upload/batch', formData, {
      headers: {
        'Content-Type': 'multipart/form-data',
      },
    });
    return response.data;
  },

  deleteSong: async (trackId: string): Promise<AudioResponse> => {
    const response = await api.delete(`/audio/tracks/${trackId}`);
    return response.data;
  },

  scanUploaded: async (): Promise<AudioResponse> => {
    const response = await api.post('/audio/scan/uploaded');
    return response.data;
  },

  // Search and Selection
  searchTracks: async (filters: SearchFilters): Promise<TrackInfo[]> => {
    const params = new URLSearchParams();
    Object.entries(filters).forEach(([key, value]) => {
      if (value !== undefined && value !== null) {
        params.append(key, value.toString());
      }
    });

    const response = await api.get(`/audio/tracks/search?${params.toString()}`);
    return response.data;
  },

  getRandomTrack: async (filters?: Partial<SearchFilters>): Promise<TrackInfo> => {
    const params = new URLSearchParams();
    if (filters) {
      Object.entries(filters).forEach(([key, value]) => {
        if (value !== undefined && value !== null) {
          params.append(key, value.toString());
        }
      });
    }

    const response = await api.get(`/audio/tracks/random?${params.toString()}`);
    return response.data;
  },

  getPopularTracks: async (limit = 10): Promise<TrackInfo[]> => {
    const response = await api.get(`/audio/tracks/popular?limit=${limit}`);
    return response.data;
  },

  getRecentTracks: async (limit = 10): Promise<TrackInfo[]> => {
    const response = await api.get(`/audio/tracks/recent?limit=${limit}`);
    return response.data;
  },

  getTracksByCategory: async (category: string, limit?: number): Promise<TrackInfo[]> => {
    const params = limit ? `?limit=${limit}` : '';
    const response = await api.get(`/audio/tracks/by-category/${category}${params}`);
    return response.data;
  },

  getTrackSelection: async (selectionType: string, filters?: Partial<SearchFilters>): Promise<TrackInfo[]> => {
    const params = new URLSearchParams();
    if (filters) {
      Object.entries(filters).forEach(([key, value]) => {
        if (value !== undefined && value !== null) {
          params.append(key, value.toString());
        }
      });
    }

    const response = await api.get(`/audio/tracks/selection/${selectionType}?${params.toString()}`);
    return response.data;
  },

  selectAndPlayTrack: async (request: TrackSelectionRequest): Promise<AudioResponse> => {
    const response = await api.post('/audio/tracks/select', request);
    return response.data;
  },
};

export default audioApi;
