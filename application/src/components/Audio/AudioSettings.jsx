import React, { useState } from 'react';
import { useAudio } from './AudioContext';
import { Settings, Volume2, Music, Database, RefreshCw, Info, Globe, Shield } from 'lucide-react';

const AudioSettings = () => {
  const { state, refreshLibrary } = useAudio();
  
  const [settings, setSettings] = useState({
    autoPlay: true,
    crossfade: true,
    crossfadeDuration: 3,
    defaultVolume: 70,
    showNotifications: true,
    theme: 'dark',
    language: 'en'
  });

  const [isRefreshing, setIsRefreshing] = useState(false);

  const handleSettingChange = (key, value) => {
    setSettings(prev => ({ ...prev, [key]: value }));
  };

  const handleRefreshLibrary = async () => {
    setIsRefreshing(true);
    try {
      await refreshLibrary();
    } finally {
      setIsRefreshing(false);
    }
  };

  const formatFileSize = (bytes) => {
    if (!bytes || bytes === 0) return '0 B';
    
    const k = 1024;
    const sizes = ['B', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    
    return parseFloat((bytes / Math.pow(k, i)).toFixed(1)) + ' ' + sizes[i];
  };

  const formatDuration = (seconds) => {
    if (!seconds || seconds < 0) return '0:00';
    
    const hours = Math.floor(seconds / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    
    if (hours > 0) {
      return `${hours}h ${minutes}m`;
    } else {
      return `${minutes}m`;
    }
  };

  return (
    <div className="space-y-6">
      {/* Header */}
      <div>
        <h1 className="text-3xl font-bold text-white">Audio Settings</h1>
        <p className="text-gray-400 mt-2">
          Configure your audio experience and app preferences
        </p>
      </div>

      {/* Audio Settings */}
      <div className="card">
        <div className="flex items-center space-x-2 mb-4">
          <Volume2 className="h-5 w-5 text-primary-400" />
          <h2 className="text-xl font-semibold text-white">Audio Settings</h2>
        </div>
        
        <div className="space-y-4">
          <div className="flex items-center justify-between">
            <div>
              <label className="text-white font-medium">Auto-play</label>
              <p className="text-sm text-gray-400">Automatically play next track</p>
            </div>
            <label className="relative inline-flex items-center cursor-pointer">
              <input
                type="checkbox"
                checked={settings.autoPlay}
                onChange={(e) => handleSettingChange('autoPlay', e.target.checked)}
                className="sr-only peer"
              />
              <div className="w-11 h-6 bg-gray-700 peer-focus:outline-none peer-focus:ring-4 peer-focus:ring-primary-800 rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-primary-600"></div>
            </label>
          </div>

          <div className="flex items-center justify-between">
            <div>
              <label className="text-white font-medium">Crossfade</label>
              <p className="text-sm text-gray-400">Smooth transition between tracks</p>
            </div>
            <label className="relative inline-flex items-center cursor-pointer">
              <input
                type="checkbox"
                checked={settings.crossfade}
                onChange={(e) => handleSettingChange('crossfade', e.target.checked)}
                className="sr-only peer"
              />
              <div className="w-11 h-6 bg-gray-700 peer-focus:outline-none peer-focus:ring-4 peer-focus:ring-primary-800 rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-primary-600"></div>
            </label>
          </div>

          {settings.crossfade && (
            <div>
              <label className="text-white font-medium">Crossfade Duration</label>
              <div className="flex items-center space-x-4 mt-2">
                <input
                  type="range"
                  min="1"
                  max="10"
                  value={settings.crossfadeDuration}
                  onChange={(e) => handleSettingChange('crossfadeDuration', parseInt(e.target.value))}
                  className="flex-1 h-2 bg-gray-700 rounded-lg appearance-none cursor-pointer slider"
                />
                <span className="text-white w-12">{settings.crossfadeDuration}s</span>
              </div>
            </div>
          )}

          <div>
            <label className="text-white font-medium">Default Volume</label>
            <div className="flex items-center space-x-4 mt-2">
              <input
                type="range"
                min="0"
                max="100"
                value={settings.defaultVolume}
                onChange={(e) => handleSettingChange('defaultVolume', parseInt(e.target.value))}
                className="flex-1 h-2 bg-gray-700 rounded-lg appearance-none cursor-pointer slider"
              />
              <span className="text-white w-12">{settings.defaultVolume}%</span>
            </div>
          </div>
        </div>
      </div>

      {/* Interface Settings */}
      <div className="card">
        <div className="flex items-center space-x-2 mb-4">
          <Globe className="h-5 w-5 text-primary-400" />
          <h2 className="text-xl font-semibold text-white">Interface Settings</h2>
        </div>
        
        <div className="space-y-4">
          <div>
            <label className="text-white font-medium">Theme</label>
            <select
              value={settings.theme}
              onChange={(e) => handleSettingChange('theme', e.target.value)}
              className="input-field w-full md:w-48 mt-2"
            >
              <option value="dark">Dark</option>
              <option value="light">Light</option>
              <option value="auto">Auto</option>
            </select>
          </div>

          <div>
            <label className="text-white font-medium">Language</label>
            <select
              value={settings.language}
              onChange={(e) => handleSettingChange('language', e.target.value)}
              className="input-field w-full md:w-48 mt-2"
            >
              <option value="en">English</option>
              <option value="es">Español</option>
              <option value="fr">Français</option>
              <option value="de">Deutsch</option>
            </select>
          </div>

          <div className="flex items-center justify-between">
            <div>
              <label className="text-white font-medium">Show Notifications</label>
              <p className="text-sm text-gray-400">Display system notifications</p>
            </div>
            <label className="relative inline-flex items-center cursor-pointer">
              <input
                type="checkbox"
                checked={settings.showNotifications}
                onChange={(e) => handleSettingChange('showNotifications', e.target.checked)}
                className="sr-only peer"
              />
              <div className="w-11 h-6 bg-gray-700 peer-focus:outline-none peer-focus:ring-4 peer-focus:ring-primary-800 rounded-full peer peer-checked:after:translate-x-full peer-checked:after:border-white after:content-[''] after:absolute after:top-[2px] after:left-[2px] after:bg-white after:rounded-full after:h-5 after:w-5 after:transition-all peer-checked:bg-primary-600"></div>
            </label>
          </div>
        </div>
      </div>

      {/* Library Management */}
      <div className="card">
        <div className="flex items-center space-x-2 mb-4">
          <Database className="h-5 w-5 text-primary-400" />
          <h2 className="text-xl font-semibold text-white">Library Management</h2>
        </div>
        
        <div className="space-y-4">
          <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
            <div className="bg-gray-700 rounded-lg p-4">
              <div className="text-2xl font-bold text-white">{state.tracks.length}</div>
              <div className="text-sm text-gray-400">Total Tracks</div>
            </div>
            
            <div className="bg-gray-700 rounded-lg p-4">
              <div className="text-2xl font-bold text-white">{state.playlists.length}</div>
              <div className="text-sm text-gray-400">Playlists</div>
            </div>
          </div>

          {state.stats && (
            <div className="grid grid-cols-1 md:grid-cols-3 gap-4">
              <div className="bg-gray-700 rounded-lg p-4">
                <div className="text-lg font-bold text-white">
                  {formatDuration(state.stats.total_duration)}
                </div>
                <div className="text-sm text-gray-400">Total Duration</div>
              </div>
              
              <div className="bg-gray-700 rounded-lg p-4">
                <div className="text-lg font-bold text-white">
                  {formatFileSize(state.stats.library_size_bytes)}
                </div>
                <div className="text-sm text-gray-400">Library Size</div>
              </div>
              
              <div className="bg-gray-700 rounded-lg p-4">
                <div className="text-lg font-bold text-white">
                  {state.stats.tracks_played}
                </div>
                <div className="text-sm text-gray-400">Tracks Played</div>
              </div>
            </div>
          )}

          <button
            onClick={handleRefreshLibrary}
            disabled={isRefreshing}
            className="btn-secondary flex items-center space-x-2"
          >
            <RefreshCw className={`h-4 w-4 ${isRefreshing ? 'animate-spin' : ''}`} />
            <span>{isRefreshing ? 'Refreshing...' : 'Refresh Library'}</span>
          </button>
        </div>
      </div>

      {/* System Information */}
      <div className="card">
        <div className="flex items-center space-x-2 mb-4">
          <Info className="h-5 w-5 text-primary-400" />
          <h2 className="text-xl font-semibold text-white">System Information</h2>
        </div>
        
        <div className="space-y-3">
          <div className="flex justify-between">
            <span className="text-gray-400">App Version</span>
            <span className="text-white">1.0.0</span>
          </div>
          
          <div className="flex justify-between">
            <span className="text-gray-400">API Endpoint</span>
            <span className="text-white">http://192.168.1.203:8000</span>
          </div>
          
          <div className="flex justify-between">
            <span className="text-gray-400">Connection Status</span>
            <span className="text-green-400">Connected</span>
          </div>
          
          {state.stats && (
            <>
              <div className="flex justify-between">
                <span className="text-gray-400">Server Uptime</span>
                <span className="text-white">
                  {formatDuration(state.stats.uptime_seconds)}
                </span>
              </div>
              
              <div className="flex justify-between">
                <span className="text-gray-400">Last Scan</span>
                <span className="text-white">
                  {state.stats.last_scan ? new Date(state.stats.last_scan).toLocaleString() : 'Never'}
                </span>
              </div>
            </>
          )}
        </div>
      </div>

      {/* Security & Privacy */}
      <div className="card">
        <div className="flex items-center space-x-2 mb-4">
          <Shield className="h-5 w-5 text-primary-400" />
          <h2 className="text-xl font-semibold text-white">Security & Privacy</h2>
        </div>
        
        <div className="space-y-4">
          <div className="bg-blue-900/20 border border-blue-700 rounded-lg p-4">
            <div className="text-blue-300 text-sm">
              <strong>Note:</strong> This app runs on your local network and does not send data to external servers.
            </div>
          </div>
          
          <div className="text-sm text-gray-400">
            <p>• All audio files are stored locally on your Raspberry Pi</p>
            <p>• No personal data is collected or transmitted</p>
            <p>• Network access is limited to your local network</p>
          </div>
        </div>
      </div>

      {/* Actions */}
      <div className="flex space-x-4">
        <button className="btn-primary">
          Save Settings
        </button>
        <button className="btn-secondary">
          Reset to Defaults
        </button>
        <button className="btn-danger">
          Clear All Data
        </button>
      </div>
    </div>
  );
};

export default AudioSettings;
