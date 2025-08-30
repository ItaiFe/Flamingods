import React from 'react';
import { useSystemMonitor } from '../../hooks/useSystemMonitor';
import { Wifi, WifiOff, Server } from 'lucide-react';
import './Header.css';

const Header = () => {
  const { systemStatus, health, isLoading } = useSystemMonitor();

  const getConnectionStatus = () => {
    if (isLoading) return { status: 'loading', icon: Server, color: 'gray' };
    if (health?.status === 'healthy') return { status: 'connected', icon: Wifi, color: 'green' };
    return { status: 'disconnected', icon: WifiOff, color: 'red' };
  };

  const connectionInfo = getConnectionStatus();
  const IconComponent = connectionInfo.icon;

  return (
    <header className="header">
      <div className="header-content">
        <div className="header-left">
          <h1 className="app-title">Stage LED Control</h1>
          <p className="app-subtitle">Professional Stage Lighting Dashboard</p>
        </div>
        
        <div className="header-right">
          <div className="connection-status">
            <IconComponent 
              className={`status-icon ${connectionInfo.status}`} 
              color={connectionInfo.color}
              size={20}
            />
            <span className={`status-text ${connectionInfo.status}`}>
              {connectionInfo.status === 'loading' && 'Connecting...'}
              {connectionInfo.status === 'connected' && 'Connected'}
              {connectionInfo.status === 'disconnected' && 'Disconnected'}
            </span>
          </div>
          
          {systemStatus && (
            <div className="system-info">
              <span className="version">v{systemStatus.server?.version || '1.0.0'}</span>
              <span className="uptime">
                {systemStatus.server?.uptime ? 
                  `${Math.floor(systemStatus.server.uptime / 3600)}h ${Math.floor((systemStatus.server.uptime % 3600) / 60)}m` : 
                  'N/A'
                }
              </span>
            </div>
          )}
        </div>
      </div>
    </header>
  );
};

export default Header;
