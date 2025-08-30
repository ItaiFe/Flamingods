import React, { useState } from 'react';
import { useDeviceManagement } from '../../hooks/useDeviceManagement';
import DeviceCard from './DeviceCard';
import BulkControl from './BulkControl';
import { RefreshCw, Search } from 'lucide-react';
import './DeviceList.css';

const DeviceList = () => {
  const {
    devices,
    isLoading,
    error,
    lastUpdate,
    getDevices,
    discoverDevices,
    clearError
  } = useDeviceManagement();

  const [searchTerm, setSearchTerm] = useState('');
  const [showOffline, setShowOffline] = useState(true);

  const filteredDevices = devices.filter(device => {
    const matchesSearch = device.name?.toLowerCase().includes(searchTerm.toLowerCase()) ||
                         device.id?.toLowerCase().includes(searchTerm.toLowerCase());
    const matchesOnlineFilter = showOffline || device.online;
    return matchesSearch && matchesOnlineFilter;
  });

  const onlineDevices = devices.filter(d => d.online);
  const offlineDevices = devices.filter(d => !d.online);

  const handleRefresh = () => {
    getDevices();
  };

  const handleDiscover = () => {
    discoverDevices();
  };

  const handleErrorDismiss = () => {
    clearError();
  };

  return (
    <div className="device-list">
      <div className="device-list-header">
        <h3>WiFi Devices</h3>
        <div className="device-counts">
          <span className="online-count">{onlineDevices.length} online</span>
          <span className="offline-count">{offlineDevices.length} offline</span>
        </div>
      </div>

      {error && (
        <div className="error-banner" onClick={handleErrorDismiss}>
          <span className="error-message">{error}</span>
          <span className="error-dismiss">Click to dismiss</span>
        </div>
      )}

      <div className="device-controls">
        <div className="search-box">
          <Search size={16} />
          <input
            type="text"
            placeholder="Search devices..."
            value={searchTerm}
            onChange={(e) => setSearchTerm(e.target.value)}
          />
        </div>
        
        <div className="filter-controls">
          <label className="filter-checkbox">
            <input
              type="checkbox"
              checked={showOffline}
              onChange={(e) => setShowOffline(e.target.checked)}
            />
            Show offline
          </label>
        </div>
      </div>

      <div className="action-buttons">
        <button 
          className="refresh-button"
          onClick={handleRefresh}
          disabled={isLoading}
        >
          <RefreshCw size={16} />
          {isLoading ? 'Refreshing...' : 'Refresh'}
        </button>
        
        <button 
          className="discover-button"
          onClick={handleDiscover}
          disabled={isLoading}
        >
          <Search size={16} />
          {isLoading ? 'Discovering...' : 'Discover'}
        </button>
        
        {isLoading && (
          <div className="discovery-status">
            <div className="discovery-progress">
              <div className="progress-bar">
                <div className="progress-fill"></div>
              </div>
              <span className="progress-text">
                Scanning network with multi-threading... (up to 90s)
              </span>
            </div>
          </div>
        )}
      </div>

      {lastUpdate && (
        <div className="last-update">
          Last updated: {lastUpdate.toLocaleTimeString()}
        </div>
      )}

      <BulkControl devices={filteredDevices} />

      <div className="devices-container">
        {filteredDevices.length === 0 ? (
          <div className="no-devices">
            {isLoading ? (
              <div className="loading-state">
                <RefreshCw className="loading-spinner" size={24} />
                <span>Loading devices...</span>
              </div>
            ) : (
              <div className="empty-state">
                <span>No devices found</span>
                <button onClick={handleDiscover} className="discover-link">
                  Discover devices
                </button>
              </div>
            )}
          </div>
        ) : (
          filteredDevices.map(device => (
            <DeviceCard key={device.id} device={device} />
          ))
        )}
      </div>
    </div>
  );
};

export default DeviceList;
