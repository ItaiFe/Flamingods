import React, { useState } from 'react';
import { useDeviceManagement } from '../../hooks/useDeviceManagement';
import { Power, PowerOff, Wifi, WifiOff, Clock, Settings } from 'lucide-react';
import './DeviceCard.css';

const DeviceCard = ({ device }) => {
  const { setPower, toggleDevice } = useDeviceManagement();
  const [isControlling, setIsControlling] = useState(false);

  const handlePowerToggle = async () => {
    if (isControlling) return;
    
    setIsControlling(true);
    try {
      await toggleDevice(device.id);
    } catch (error) {
      console.error('Failed to toggle device:', error);
    } finally {
      setIsControlling(false);
    }
  };

  const handlePowerSet = async (powerState) => {
    if (isControlling) return;
    
    setIsControlling(true);
    try {
      await setPower(device.id, powerState);
    } catch (error) {
      console.error('Failed to set device power:', error);
    } finally {
      setIsControlling(false);
    }
  };

  const formatLastSeen = (timestamp) => {
    if (!timestamp) return 'Never';
    
    const date = new Date(timestamp);
    const now = new Date();
    const diff = now - date;
    const minutes = Math.floor(diff / 60000);
    const hours = Math.floor(minutes / 60);
    const days = Math.floor(hours / 24);
    
    if (days > 0) return `${days}d ago`;
    if (hours > 0) return `${hours}h ago`;
    if (minutes > 0) return `${minutes}m ago`;
    return 'Just now';
  };

  const getStatusColor = () => {
    if (!device.online) return '#ef4444'; // Red for offline
    if (device.power === 'on') return '#10b981'; // Green for on
    return '#6b7280'; // Gray for off
  };

  const getStatusText = () => {
    if (!device.online) return 'Offline';
    if (device.power === 'on') return 'On';
    return 'Off';
  };

  return (
    <div className={`device-card ${device.online ? 'online' : 'offline'}`}>
      <div className="device-header">
        <div className="device-info">
          <h4 className="device-name">{device.name || device.id}</h4>
          <p className="device-id">{device.id}</p>
        </div>
        
        <div className="device-status">
          <div 
            className="status-indicator"
            style={{ backgroundColor: getStatusColor() }}
          />
          <span className="status-text">{getStatusText()}</span>
        </div>
      </div>

      <div className="device-details">
        <div className="detail-row">
          <Wifi size={14} />
          <span className="detail-label">Type:</span>
          <span className="detail-value">{device.type || 'Unknown'}</span>
        </div>
        
        <div className="detail-row">
          <Clock size={14} />
          <span className="detail-label">Last seen:</span>
          <span className="detail-value">{formatLastSeen(device.last_seen)}</span>
        </div>
        
        {device.ip && (
          <div className="detail-row">
            <Settings size={14} />
            <span className="detail-label">IP:</span>
            <span className="detail-value">{device.ip}</span>
          </div>
        )}
      </div>

      <div className="device-controls">
        {device.online ? (
          <>
            <button
              className={`power-button ${device.power === 'on' ? 'on' : 'off'}`}
              onClick={handlePowerToggle}
              disabled={isControlling}
            >
              {device.power === 'on' ? <PowerOff size={16} /> : <Power size={16} />}
              {isControlling ? '...' : device.power === 'on' ? 'Turn Off' : 'Turn On'}
            </button>
            
            <div className="quick-controls">
              <button
                className="quick-on"
                onClick={() => handlePowerSet('on')}
                disabled={isControlling || device.power === 'on'}
              >
                <Power size={14} />
              </button>
              <button
                className="quick-off"
                onClick={() => handlePowerSet('off')}
                disabled={isControlling || device.power === 'off'}
              >
                <PowerOff size={14} />
              </button>
            </div>
          </>
        ) : (
          <div className="offline-message">
            <WifiOff size={16} />
            <span>Device offline</span>
          </div>
        )}
      </div>
    </div>
  );
};

export default DeviceCard;
