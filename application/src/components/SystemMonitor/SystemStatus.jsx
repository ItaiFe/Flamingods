import React from 'react';
import { useSystemMonitor } from '../../hooks/useSystemMonitor';
import { Server, Wifi, Users, Activity, Clock, RefreshCw } from 'lucide-react';
import './SystemStatus.css';

const SystemStatus = () => {
  const {
    systemStatus,
    websocketClients,
    health,
    isLoading,
    error,
    lastUpdate,
    refreshAll,
    clearError
  } = useSystemMonitor();

  const formatUptime = (seconds) => {
    if (!seconds) return 'N/A';
    
    const hours = Math.floor(seconds / 3600);
    const minutes = Math.floor((seconds % 3600) / 60);
    
    if (hours > 24) {
      const days = Math.floor(hours / 24);
      const remainingHours = hours % 24;
      return `${days}d ${remainingHours}h ${minutes}m`;
    }
    
    return `${hours}h ${minutes}m`;
  };

  const getHealthColor = () => {
    if (!health) return '#6b7280';
    if (health.status === 'healthy') return '#10b981';
    return '#ef4444';
  };

  const getHealthText = () => {
    if (!health) return 'Unknown';
    return health.status === 'healthy' ? 'Healthy' : 'Unhealthy';
  };

  const handleRefresh = () => {
    refreshAll();
  };

  const handleErrorDismiss = () => {
    clearError();
  };

  return (
    <div className="system-status">
      <div className="status-header">
        <h3>System Status</h3>
        <button 
          className="refresh-button"
          onClick={handleRefresh}
          disabled={isLoading}
        >
          <RefreshCw size={16} />
        </button>
      </div>

      {error && (
        <div className="error-banner" onClick={handleErrorDismiss}>
          <span className="error-message">{error}</span>
          <span className="error-dismiss">Click to dismiss</span>
        </div>
      )}

      <div className="status-grid">
        {/* Server Health */}
        <div className="status-card health">
          <div className="card-header">
            <Server size={20} />
            <h4>Server Health</h4>
          </div>
          <div className="card-content">
            <div className="status-indicator">
              <div 
                className="health-dot"
                style={{ backgroundColor: getHealthColor() }}
              />
              <span className="status-text">{getHealthText()}</span>
            </div>
            {health?.timestamp && (
              <div className="timestamp">
                <Clock size={12} />
                {new Date(health.timestamp).toLocaleTimeString()}
              </div>
            )}
          </div>
        </div>

        {/* WebSocket Connections */}
        <div className="status-card websocket">
          <div className="card-header">
            <Wifi size={20} />
            <h4>WebSocket</h4>
          </div>
          <div className="card-content">
            <div className="connection-stats">
              <div className="stat-item">
                <span className="stat-label">Active:</span>
                <span className="stat-value">
                  {systemStatus?.websocket?.clients || 0}
                </span>
              </div>
              <div className="stat-item">
                <span className="stat-label">Total Connected:</span>
                <span className="stat-value">
                  {systemStatus?.websocket?.total_connected || 0}
                </span>
              </div>
            </div>
          </div>
        </div>

        {/* Device Statistics */}
        <div className="status-card devices">
          <div className="card-header">
            <Activity size={20} />
            <h4>Devices</h4>
          </div>
          <div className="card-content">
            <div className="device-stats">
              <div className="stat-item">
                <span className="stat-label">Total:</span>
                <span className="stat-value">
                  {systemStatus?.devices?.total || 0}
                </span>
              </div>
              <div className="stat-item">
                <span className="stat-label">Online:</span>
                <span className="stat-value">
                  {systemStatus?.devices?.online || 0}
                </span>
              </div>
            </div>
          </div>
        </div>

        {/* Server Info */}
        <div className="status-card server">
          <div className="card-header">
            <Server size={20} />
            <h4>Server Info</h4>
          </div>
          <div className="card-content">
            <div className="server-info">
              <div className="info-item">
                <span className="info-label">Version:</span>
                <span className="info-value">
                  v{systemStatus?.server?.version || '1.0.0'}
                </span>
              </div>
              <div className="info-item">
                <span className="info-label">Uptime:</span>
                <span className="info-value">
                  {formatUptime(systemStatus?.server?.uptime)}
                </span>
              </div>
            </div>
          </div>
        </div>
      </div>

      {lastUpdate && (
        <div className="last-update">
          <Clock size={14} />
          <span>Last updated: {lastUpdate.toLocaleTimeString()}</span>
        </div>
      )}
    </div>
  );
};

export default SystemStatus;
