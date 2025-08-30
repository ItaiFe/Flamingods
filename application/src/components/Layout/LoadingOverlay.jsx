import React from 'react';
import { Loader, Zap, Sparkles, Play, SkipForward } from 'lucide-react';
import './LoadingOverlay.css';

const LoadingOverlay = ({ targetStage }) => {
  const getStageIcon = (stage) => {
    switch (stage) {
      case 'idle':
        return <Play size={48} />;
      case 'skip':
        return <SkipForward size={48} />;
      case 'show':
        return <Zap size={48} />;
      case 'special':
        return <Sparkles size={48} />;
      default:
        return <Loader size={48} />;
    }
  };

  const getStageDisplayName = (stage) => {
    switch (stage) {
      case 'idle':
        return 'IDLE';
      case 'skip':
        return 'SKIP';
      case 'show':
        return 'SHOW';
      case 'special':
        return 'SPECIAL';
      default:
        return 'STAGE';
    }
  };

  const getStageColor = (stage) => {
    switch (stage) {
      case 'idle':
        return '#6b7280';
      case 'skip':
        return '#f59e0b';
      case 'show':
        return '#3b82f6';
      case 'special':
        return '#8b5cf6';
      default:
        return '#6b7280';
    }
  };

  return (
    <div className="loading-overlay">
      <div className="loading-content">
        <div className="loading-icon-container">
          <div 
            className="loading-icon"
            style={{ color: getStageColor(targetStage) }}
          >
            {getStageIcon(targetStage)}
          </div>
        </div>
        
        <h2 className="loading-title">
          Transitioning to {getStageDisplayName(targetStage)}
        </h2>
        
        <p className="loading-description">
          Updating stage lighting and effects...
        </p>
        
        <div className="loading-spinner">
          <Loader size={32} className="spinner-icon" />
        </div>
        
        <div className="loading-progress">
          <div className="progress-bar">
            <div className="progress-fill"></div>
          </div>
          <span className="progress-text">Waiting for ESP response...</span>
        </div>
      </div>
    </div>
  );
};

export default LoadingOverlay;
