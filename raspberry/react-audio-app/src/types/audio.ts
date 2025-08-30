export interface TrackInfo {
  id: string;
  title: string;
  artist?: string;
  album?: string;
  filename: string;
  filepath: string;
  format: string;
  size_bytes: number;
  duration_seconds?: number;
  bitrate?: number;
  sample_rate?: number;
  channels?: number;
  genre?: string;
  year?: number;
  track_number?: number;
  created_at: string;
  last_played?: string;
  play_count: number;
}

export interface PlaylistInfo {
  id: string;
  name: string;
  description?: string;
  tracks: TrackInfo[];
  total_duration?: number;
  shuffle: boolean;
  repeat: boolean;
  auto_advance: boolean;
  created_at: string;
  last_modified: string;
  last_played?: string;
}

export interface PlaybackStatus {
  state: 'stopped' | 'playing' | 'paused' | 'loading' | 'error';
  current_track?: TrackInfo;
  current_playlist?: PlaylistInfo;
  position_seconds: number;
  duration_seconds: number;
  progress_percent: number;
  volume: number;
  muted: boolean;
  queue_position: number;
  queue_length: number;
  last_updated: string;
  error_message?: string;
}

export interface AudioResponse {
  success: boolean;
  message: string;
  data?: any;
  error?: string;
  error_code?: string;
}

export interface AudioStats {
  total_tracks: number;
  total_playlists: number;
  total_duration: number;
  library_size_bytes: number;
  tracks_played: number;
  total_play_time: number;
  average_session_length: number;
  scan_time_seconds: number;
  last_scan?: string;
  errors_count: number;
  uptime_seconds: number;
  memory_usage_mb: number;
  cpu_usage_percent: number;
}

export interface UploadResponse {
  track_id: string;
  title: string;
  artist?: string;
  filename: string;
  filepath: string;
  category: string;
  size_bytes: number;
  duration_seconds?: number;
}

export interface BatchUploadResponse {
  uploaded_count: number;
  failed_count: number;
  uploaded_files: UploadResponse[];
  failed_files: Array<{
    filename: string;
    error: string;
  }>;
  category: string;
}

export interface SearchFilters {
  query?: string;
  artist?: string;
  album?: string;
  genre?: string;
  format?: string;
  duration_min?: number;
  duration_max?: number;
  limit?: number;
}

export interface TrackSelectionRequest {
  selection_type: 'random' | 'popular' | 'recent' | 'diverse';
  count: number;
  genre?: string;
  artist?: string;
  duration_min?: number;
  duration_max?: number;
  auto_play?: boolean;
}
