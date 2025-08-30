import React from 'react';
import { Link, useLocation } from 'react-router-dom';
import { Home, Music, Library, ListMusic, Upload, Search, Settings, Zap, Monitor } from 'lucide-react';
import './Sidebar.css';

const Sidebar = ({ children }) => {
  const location = useLocation();

  const isActive = (path) => {
    if (path === '/') {
      return location.pathname === '/';
    }
    return location.pathname.startsWith(path);
  };

  const navItems = [
    { path: '/', icon: Home, label: 'Stage Control', description: 'LED Control & Effects' },
    { path: '/audio', icon: Music, label: 'Audio Dashboard', description: 'Music & Playback' },
    { path: '/audio/library', icon: Library, label: 'Music Library', description: 'Browse Collection' },
    { path: '/audio/playlists', icon: ListMusic, label: 'Playlists', description: 'Manage Playlists' },
    { path: '/audio/upload', icon: Upload, label: 'Upload Music', description: 'Add New Tracks' },
    { path: '/audio/search', icon: Search, label: 'Search & Select', description: 'Find Music' },
    { path: '/audio/settings', icon: Settings, label: 'Audio Settings', description: 'Configure Audio' }
  ];

  return (
    <aside className="sidebar">
      {/* Navigation Menu */}
      <nav className="sidebar-nav">
        <div className="nav-section">
          <h3 className="nav-section-title">Stage Control</h3>
          <ul className="nav-list">
            <li>
              <Link 
                to="/" 
                className={`nav-item ${isActive('/') ? 'active' : ''}`}
              >
                <Zap className="nav-icon" />
                <div className="nav-content">
                  <span className="nav-label">Stage Control</span>
                  <span className="nav-description">LED Control & Effects</span>
                </div>
              </Link>
            </li>
          </ul>
        </div>

        <div className="nav-section">
          <h3 className="nav-section-title">Audio System</h3>
          <ul className="nav-list">
            {navItems.slice(1).map((item) => (
              <li key={item.path}>
                <Link 
                  to={item.path} 
                  className={`nav-item ${isActive(item.path) ? 'active' : ''}`}
                >
                  <item.icon className="nav-icon" />
                  <div className="nav-content">
                    <span className="nav-label">{item.label}</span>
                    <span className="nav-description">{item.description}</span>
                  </div>
                </Link>
              </li>
            ))}
          </ul>
        </div>
      </nav>

      {/* Existing Sidebar Content */}
      <div className="sidebar-content">
        {children}
      </div>
    </aside>
  );
};

export default Sidebar;
